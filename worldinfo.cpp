/** Copyright (c) 2022, EtlamGit */

#include "worldinfo.h"

#include <QtWidgets/QMenu>
#include <QDirIterator>

#include "nbt/nbt.h"
#include "zipreader.h"
#include "json/json.h"


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


bool WorldInfo::parseFolder(const QDir &path)
{
  clear();
  if (!path.exists("level.dat")) // no level.dat?  no world
    return false;

  folder = path;

  NBT level(path.filePath("level.dat"));
  auto data = level.at("Data");

  if (data->has("LevelName"))
    levelName = data->at("LevelName")->toString();

  if (data->has("DataVersion"))
    dataVersion = dynamic_cast<const Tag_Int *>(data->at("DataVersion"))->toUInt();

  if (data->has("DayTime"))
    dayTime = dynamic_cast<const Tag_Long *>(data->at("DayTime"))->toULong();

  // Spawn
  spawnX = data->at("SpawnX")->toInt();
  spawnZ = data->at("SpawnZ")->toInt();

  return true;
}


bool WorldInfo::parseDimensions()
{
  if (!folder.exists("level.dat")) // no level.dat?  no world
    return false;

  NBT level(folder.filePath("level.dat"));
  auto data = level.at("Data");

  // check all enabled datapacks for custom dimensions
  if (data->has("DataPacks") && data->at("DataPacks")->has("Enabled")) {
    auto datapacks = data->at("DataPacks")->at("Enabled");
    int num = datapacks->length();
    for (int d = 0; d < num; d++) {
      QString dp = datapacks->at(d)->toString();
      // Vanilla Dimensions
      if (dp == "vanilla") {
        continue;
      }
      // we only support this style "file/<packname>.zip" -> skip otherwise
      if (!dp.startsWith("file")) continue;
      QStringRef pack_name(&dp, 5, dp.size()-5-4);

      // parse custom Dimension data
      // located in ./datapacks/<packname>.zip -> ./data/<packname>/dimension/<dimensions>.json
      ZipReader zip(folder.path() + "/datapacks/" + pack_name + ".zip");
      if (zip.open()) {
        for (auto & file: zip.getFileList()) {
          if (file.startsWith("data/" + pack_name + "/dimension/") &&
              file.endsWith(".json")) {
            std::unique_ptr<JSONData> json;
            try {
              json = JSON::parse(zip.get(file));
            } catch (JSONParseException e) {
              // file was not parsable -> silently try next one
              continue;
            }
            // now 'json' should contain the Dimension description
            QFileInfo f(file);
            QString dim_name = f.baseName();
            DimensionInfo dim;
            dim.path = "./dimensions/" + pack_name + "/" + dim_name;
            dim.name = pack_name + ":" + dim_name;

            if (json->has("type")) {
              dim.id = json->at("type")->asString();
              parseDimensionType(dim, dim.id);
            } else {
              // mandatory field, in case it is missing -> silently try next one
              continue;
            }

            // store it in list
            dimensions.append(dim);
          }
        }

        zip.close();
      }
    }
  }

  return true;
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
    std::unique_ptr<JSONData> json;
    // located in ./datapacks/<packname>.zip -> ./data/<packname>/dimension_type/<dimensions>.json
    ZipReader zip(folder.path() + "/datapacks/" + dim_type_list[0] + ".zip");
    if (zip.open()) {
      QString file = "data/" + dim_type_list[0] + "/dimension_type/" + dim_type_list[1] + ".json";
      try {
        json = JSON::parse(zip.get(file));
      } catch (JSONParseException e) {
        // file was not parsable -> silently try next one
        zip.close();
        return false;
      }
      zip.close();
    }
    // now 'json' should contain the Dimension Type description

    // build height
    if (json->has("min_y"))
      dim.minY = json->at("min_y")->asNumber();
    if (json->has("height"))
      dim.maxY = json->at("height")->asNumber() + dim.minY;

    if (json->has("coordinate_scale"))
      dim.scale = json->at("coordinate_scale")->asNumber();

    dim.defaultY = dim.maxY;
    if ((json->has("has_ceiling")) && (json->at("has_ceiling")->asBool()))
      dim.defaultY = (dim.minY + dim.maxY) / 2;
  }

  return true;
}

void WorldInfo::clear()
{
  folder = QDir();
  levelName   = "unknown world name";
  dataVersion = 0;
  dayTime     = 0;

  spawnX      = 0;
  spawnZ      = 0;

  dimensions.clear();
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
  for (auto dim: dimensions) {
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
    WorldInfo & wi(WorldInfo::Instance());
    const QList<DimensionInfo> & custom = wi.getDimensions();
    for (int idx = 0; idx<custom.length(); idx++) {
      if (custom[idx].name == name) {
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
      WorldInfo & wi(WorldInfo::Instance());
      const QList<DimensionInfo> & custom = wi.getDimensions();
      emit dimensionChanged(custom[idx-DIM_MAGIC]);
    }
  }
}
