/** Copyright (c) 2022, EtlamGit */

#include "worldinfo.h"

#include <QtWidgets/QMenu>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>

#include "nbt/nbt.h"
#include "zipreader.h"


WorldInfo::WorldInfo()
  : menuActionGroup(NULL)
{
  clear();
}

WorldInfo::~WorldInfo() {}

WorldInfo& WorldInfo::Instance() {
  static WorldInfo singleton;
  return singleton;
}

void WorldInfo::clear() {
  folder = QDir();
  levelName   = "unknown world name";
  dataVersion = 0;
  dayTime     = 0;

  spawnX      = 0;
  spawnZ      = 0;

  seed        = 0;

  dimensions.clear();
}


/// used to generate menu File->Open World
/// parse only basic stuff in the world folder:
///  does it look like a Minecraft world
///  does the world have a custom name
bool WorldInfo::parseWorldFolder(const QDir &path)
{
  clear();
  if (!path.exists("level.dat")) // no level.dat?  no world
    return false;

  folder = path;

  NBT level(path.filePath("level.dat"));
  auto data = level.at("Data");
  if (!data) return false;

  if (data->has("LevelName"))
    levelName = data->at("LevelName")->toString();

  return true;
}


/// used when opening a world
/// parse more stuff relevant later for rendering
bool WorldInfo::parseWorldInfo()
{
  NBT level(folder.filePath("level.dat"));
  auto data = level.at("Data");
  if (!data) return false;

  if (data->has("LevelName"))
    levelName = data->at("LevelName")->toString();

  if (data->has("DataVersion")) {
    const Tag_Int * dataversiontag = dynamic_cast<const Tag_Int *>(data->at("DataVersion"));
    if (dataversiontag)
      dataVersion = dataversiontag->toUInt();
  }

  if (data->has("DayTime")) {
    const Tag_Long * daytimetag = dynamic_cast<const Tag_Long *>(data->at("DayTime"));
    if (daytimetag) {
      dayTime = daytimetag->toULong();
    } else {
      // wrong type, but observed in issue #329
      const Tag_Int * daytimeinttag = dynamic_cast<const Tag_Int *>(data->at("DayTime"));
      if (daytimeinttag)
        dayTime = daytimeinttag->toUInt();
    }
  }

  // Spawn
  // default
  spawnX = 0;
  spawnZ = 0;
  spawnDimension = "minecraft:overworld";
  // for old worlds spawn is 2 separated tags
  if (data->has("SpawnX")) { spawnX = data->at("SpawnX")->toInt(); }
  if (data->has("SpawnZ")) { spawnZ = data->at("SpawnZ")->toInt(); }
  // starting with 1.21.9 we have sub tag "spawn"
  if (data->has("spawn")) {
    auto spawn = data->at("spawn");
    if (spawn->has("dimension")) {
      spawnDimension = spawn->at("dimension")->toString();
    }
    if ((spawnDimension == "minecraft:overworld") && spawn->has("pos")) {
      auto pos = spawn->at("pos")->toIntArray();
      spawnX = pos[0];
      spawnZ = pos[2];
    }
  }

  // Seed
  if (data->has("RandomSeed")) {
    const Tag_Long * seedtag = dynamic_cast<const Tag_Long *>(data->at("RandomSeed"));
    if (seedtag)
      seed = seedtag->toLong();
  }
  if (data->has("WorldGenSettings") && data->at("WorldGenSettings")->has("seed")) {
    const Tag_Long * seedtag = dynamic_cast<const Tag_Long *>(data->at("WorldGenSettings")->at("seed"));
    if (seedtag)
      seed = seedtag->toLong();
  }

  // collect information about installed datapacks (mods)
  datapacks.clear();
  parseDatapacks(data);

  return true;
}


void WorldInfo::parseDatapacks(const Tag * data)
{
  // check all enabled datapacks
  if (data->has("DataPacks") && data->at("DataPacks")->has("Enabled")) {
    auto datapacks = data->at("DataPacks")->at("Enabled");
    int num = datapacks->length();
    for (int d = 0; d < num; d++) {
      QString dp = datapacks->at(d)->toString();

      // skip Vanilla
      if (dp == "vanilla") {
        continue;
      }

      // we only support this style "file/<packname>"
      // we only support this style "file/<packname>.zip"
      // -> skip otherwise
      if (!dp.startsWith("file")) continue;

      // "file/<packname>.zip" -> zipped
      if (dp.endsWith(".zip")) {
        QStringRef pack_name(&dp, 5, dp.size()-5-4);
        QString    zip_file = folder.path() + "/datapacks/" + pack_name + ".zip";
        ZipReader zip(zip_file);
        if (zip.open()) {
          for (auto & file: zip.getFileList()) {
            // determine name of datapack
            // -> ./data/<namespace>/*
            if (file.startsWith("data/")) {
              QStringList folders(file.split("/"));
              if (folders.length() > 2)  // only when 2 subfolders
                parseDatapackNamespace(folders[1], zip_file, true, true);
            }

            // custom Dimensions
            // ./data/<packname>/dimension/<dimensions>.json
            if (file.startsWith("data/" + pack_name + "/dimension/") &&
                file.endsWith(".json")) {
              QJsonDocument json_doc = QJsonDocument::fromJson(zip.get(file));
              if (!json_doc.isNull())
                parseDimension(json_doc, pack_name.toString(), QFileInfo(file).baseName());
            }

          }
          zip.close();
        }
        continue;
      }

      // "file/<packname>" -> unzipped
      {
        QStringRef pack_name(&dp, 5, dp.size()-5);
        QString    pack_path(folder.path() + "/datapacks/" + pack_name + "/");
        // determine name of datapack
        // -> ./data/<namespace>/*
        QDirIterator data(pack_path + "data", QDir::Dirs | QDir::NoDotAndDotDot);
        while (data.hasNext()) {
          data.next();
          parseDatapackNamespace(data.fileName(), pack_path, true, false);
        }

        // custom Dimensions
        // ./data/<packname>/dimension/<dimensions>.json
        // -> todo

      }
    }
  }
}


bool WorldInfo::parseDatapackNamespace(const QString name_space, const QString path, bool enabled, bool zipped)
{
  // skip default stuff
  if (name_space == "minecraft") return false;

  // test if already there
  if (datapacks.contains(name_space)) {
    QMap<QString, DatapackInfo>::const_iterator it = datapacks.constFind(name_space);
    for (; it != datapacks.constEnd(); ++it) {
      if (it->path == path) return false;
    }
  }

  // else insert it
  DatapackInfo dp;
  dp.path    = path;
  dp.enabled = enabled;
  dp.zipped  = zipped;
  // store it in list
  datapacks.insert(name_space, dp);
  return true;
}


bool WorldInfo::parseDimension(const QJsonDocument & json_doc, QString pack_name, QString dim_name)
{
  QJsonObject json = json_doc.object();
  // now 'json' should contain the Dimension description
  DimensionInfo dim;
  dim.path = "./dimensions/" + pack_name + "/" + dim_name;
  dim.name = pack_name + ":" + dim_name;

  if (json.contains("type")) {
    dim.id = json.value("type").toString();
    parseDimensionType(dim, dim.id);
    // store it in list
    dimensions.append(dim);
    return true;
  } else {
    // mandatory field, in case it is missing -> silently try next one
    return false;
  }
}


bool WorldInfo::parseDimensionType(DimensionInfo & dim, const QString & dim_type_id)
{
  // dim_type_list[0]: packname where to find dimension_type
  // dim_type_list[1]: name of dimension_type description
  QStringList dim_type_list = dim_type_id.split(':');

  if (dim_type_list[0] == "minecraft") {
    // reference to vanilla dimension
    const DimensionInfo & vanilla = DimensionIdentifier::Instance().getDimensionInfo(dim_type_id);

    dim.minY     = vanilla.minY;
    dim.maxY     = vanilla.maxY;
    dim.defaultY = vanilla.defaultY;
    dim.scale    = vanilla.scale;

  } else {
    // parse custom Dimension Type data
    QJsonObject json;
    // located in ./datapacks/<packname>.zip -> ./data/<packname>/dimension_type/<dimensions>.json
    ZipReader zip(folder.path() + "/datapacks/" + dim_type_list[0] + ".zip");
    if (zip.open()) {
      QString file = "data/" + dim_type_list[0] + "/dimension_type/" + dim_type_list[1] + ".json";
      QJsonDocument json_doc = QJsonDocument::fromJson(zip.get(file));
      zip.close();
      if (json_doc.isNull()) {
        // file was not parsable -> silently try next one
        return false;
      }
      json = json_doc.object();
    }
    // now 'json' should contain the Dimension Type description

    // build height
    if (json.contains("min_y"))
      dim.minY = json.value("min_y").toInt();
    if (json.contains("height"))
      dim.maxY = json.value("height").toInt() + dim.minY;

    if (json.contains("coordinate_scale"))
      dim.scale = json.value("coordinate_scale").toInt();

    dim.defaultY = dim.maxY;
    if ((json.contains("has_ceiling")) && (json.value("has_ceiling").toBool()))
      dim.defaultY = (dim.minY + dim.maxY) / 2;
  }

  return true;
}



//-------------------------------------------------------------------------------------------------
// Dimension view menu

void WorldInfo::clearDimensionsMenu(QMenu *menu) {
  for (int i = 0; i < currentMenuActions.count(); i++) {
    menu           ->removeAction(currentMenuActions[i]);
    menuActionGroup->removeAction(currentMenuActions[i]);
    delete currentMenuActions[i];
  }
  currentMenuActions.clear();
  foundDimensionDirs.clear();
  menu->setEnabled(false);
  if (menuActionGroup != NULL) {
    delete menuActionGroup;
    menuActionGroup = NULL;
  }
}

void WorldInfo::getDimensionsInWorld(QDir path, QMenu *menu, QObject *parent) {
  // first get the currently selected dimension so it doesn't change
  int currentIdx = -1;
  for (int i = 0; i < currentMenuActions.length(); i++)
    if (currentMenuActions[i]->isChecked())
      currentIdx = currentMenuActions[i]->data().toInt();
  clearDimensionsMenu(menu);
  menuActionGroup = new QActionGroup(parent);

  // add normal Dimensions
  int index = 0;
  while (true) {
    const DimensionInfo & dim = DimensionIdentifier::Instance().getDimensionInfo(index++);
    if (dim.name == "Dummy Dimension")
      break;

    if (dim.enabled) {
      // check path for regex
      if (dim.pathIsRegEx) {
        QDirIterator it(path.absolutePath(), QDir::Dirs);
        QRegExp rx(dim.path);
        while (it.hasNext()) {
          it.next();
          if (rx.indexIn(it.fileName()) != -1) {
            QString name = dim.name;
            for (int c = 0; c < rx.captureCount(); c++)
              name = name.arg(rx.cap(c + 1));
            addDimensionToMenu(path, it.fileName(), name, parent);
          }
        }
      } else {
        addDimensionToMenu(path, dim.path, dim.name, parent);
      }
    }
  }

  // add Custom Dimensions
  for (auto & dim: dimensions) {
    addDimensionToMenu(path, dim.path, dim.name, parent);
  }

  // re-add new build actions to menu
  menu->addActions(currentMenuActions);
  if (currentMenuActions.count() > 0) {
    bool changed = true;
    // locate our old selected item
    for (int i = 0; i < currentMenuActions.length(); i++) {
      if (currentMenuActions[i]->data().toInt() == currentIdx) {
        currentMenuActions[i]->setChecked(true);
        changed = false;
        break;
      }
    }
    if (changed) {
      currentMenuActions.first()->setChecked(true);
      int idx = currentMenuActions.first()->data().toInt();
      emit dimensionChanged(DimensionIdentifier::Instance().getDimensionInfo(idx));
    }
    menu->setEnabled(true);
  }
}



#define DIM_MAGIC 0x10000

void WorldInfo::addDimensionToMenu(QDir path, QString dir, QString name, QObject *parent) {
  // prevent adding non-existing directory
  if (!path.exists(dir))
    return;

  // prevent adding unused dimension
  if (!path.exists(dir + "/region"))
    return;

  // prevent re-adding already found directory
  if (foundDimensionDirs.contains(dir))
    return;

  QAction *action = new QAction(parent);
  action->setText(name);
  // find index in definition list
  int index = DimensionIdentifier::Instance().getDimensionIndex(name);
  if (index > -1) {
    // found a matching index in normal Dimensions
    action->setData(index);
  } else {
    // find index in custom Dimension list
    for (int idx = 0; idx<dimensions.length(); idx++) {
      if (dimensions[idx].name == name) {
        action->setData(idx+DIM_MAGIC);
      }
    }
  }

  action->setCheckable(true);
  parent->connect(action, SIGNAL(triggered()),
                  this,   SLOT(changeViewToDimension()));
  menuActionGroup->addAction(action);
  currentMenuActions.append(action);
  foundDimensionDirs.append(dir);
}


void WorldInfo::changeViewToDimension() {
  QAction *action = qobject_cast<QAction*>(sender());
  if (action) {
    int idx = action->data().toInt();
    if (idx < DIM_MAGIC) {
      emit dimensionChanged(DimensionIdentifier::Instance().getDimensionInfo(idx));
    } else {
      emit dimensionChanged(dimensions[idx-DIM_MAGIC]);
    }
  }
}


// datapack stuff

bool WorldInfo::isDatapackEnabled(const QString name_space) const
{
  return datapacks.contains(name_space);
}
