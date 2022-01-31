/** Copyright (c) 2013, Sean Kasun */

#include <QDirIterator>
#include <QtWidgets/QMenu>

#include "dimensionidentifier.h"
#include "json/json.h"
#include "worldinfo.h"


//-------------------------------------------------------------------------------------------------
// DimensionInfo used for each defined & found Dimension

DimensionInfo::DimensionInfo()
  : name("Unknown Dimension")
  , path(".")
  , pathIsRegEx(false)
  , enabled(true)
  , scale(1)
  , minY(0)
  , maxY(255)
  , defaultY(255)
{}


//-------------------------------------------------------------------------------------------------
// DimensionIdentifier

DimensionIdentifier::DimensionIdentifier()
  : menuActionGroup(NULL)
{}

DimensionIdentifier::~DimensionIdentifier() {
  for (int i = 0; i < packs.length(); i++) {
    for (int j = 0; j < packs[i].length(); j++)
      delete packs[i][j];
  }
}

DimensionIdentifier& DimensionIdentifier::Instance() {
  static DimensionIdentifier singleton;
  return singleton;
}


void DimensionIdentifier::enableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = true;
}

void DimensionIdentifier::disableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = false;
}

int DimensionIdentifier::addDefinitions(JSONArray *defs, int pack) {
  if (pack == -1) {
    pack = packs.length();
    packs.append(QList<DimensionInfo*>());
  }

  int len = defs->length();
  for (int i = 0; i < len; i++) {
    JSONObject *dimTag = dynamic_cast<JSONObject *>(defs->at(i));
    DimensionInfo *dim = new DimensionInfo();
    dim->enabled = true;
    // Minecraft namespace ID
    if (dimTag->has("id")) {
      dim->id = dimTag->at("id")->asString();
      // construct a default name from ID
      QString nid = QString(dim->id).replace("minecraft:","").replace("_"," ");
      QStringList parts = nid.toLower().split(' ', QString::SkipEmptyParts);
      for (int i = 0; i < parts.size(); i++)
        parts[i].replace(0, 1, parts[i][0].toUpper());
      dim->name = parts.join(" ");
    }
    // explicit name given
    if (dimTag->has("name"))
      dim->name = dimTag->at("name")->asString();
    // path is where the region folder is located
    if (dimTag->has("path"))
      dim->path = dimTag->at("path")->asString();
    // scale
    if (dimTag->has("scale"))
      dim->scale = dimTag->at("scale")->asNumber();
    // unused feature for RegEx in path
    if (dimTag->has("regex"))
      dim->pathIsRegEx = dimTag->at("regex")->asBool();
    if (dimTag->has("minY"))
      dim->minY = dimTag->at("minY")->asNumber();
    if (dimTag->has("maxY"))
      dim->maxY = dimTag->at("maxY")->asNumber();
    if (dimTag->has("defaultY"))
      dim->defaultY = dimTag->at("defaultY")->asNumber();

    definitions.append(dim);
    packs[pack].append(dim);
  }
  return pack;
}

//-------------------------------------------------------------------------------------------------
// Dimension view menu

void DimensionIdentifier::clearDimensionsMenu(QMenu *menu) {
  for (int i = 0; i < currentMenuActions.count(); i++) {
    menu           ->removeAction(currentMenuActions[i]);
    menuActionGroup->removeAction(currentMenuActions[i]);
    delete currentMenuActions[i];
  }
  currentMenuActions.clear();
  //dimensions.clear();
  foundDimensionDirs.clear();
  menu->setEnabled(false);
  if (menuActionGroup != NULL) {
    delete menuActionGroup;
    menuActionGroup = NULL;
  }
}

void DimensionIdentifier::getDimensionsInWorld(QDir path, QMenu *menu, QObject *parent) {
  // first get the currently selected dimension so it doesn't change
  int currentIdx = -1;
  for (int i = 0; i < currentMenuActions.length(); i++)
    if (currentMenuActions[i]->isChecked())
      currentIdx = currentMenuActions[i]->data().toInt();
  clearDimensionsMenu(menu);
  menuActionGroup = new QActionGroup(parent);

  for (int i = 0; i < definitions.length(); i++) {
    if (definitions[i]->enabled) {
      // check path for regex
      if (definitions[i]->pathIsRegEx) {
        QDirIterator it(path.absolutePath(), QDir::Dirs);
        QRegExp rx(definitions[i]->path);
        while (it.hasNext()) {
          it.next();
          if (rx.indexIn(it.fileName()) != -1) {
            QString name = definitions[i]->name;
            for (int c = 0; c < rx.captureCount(); c++)
              name = name.arg(rx.cap(c + 1));
            addDimensionMenu(path, it.fileName(), name, parent);
          }
        }
      } else {
        addDimensionMenu(path, definitions[i]->path, definitions[i]->name, parent);
      }
    }
  }

  // add Custom Dimensions
  WorldInfo & wi(WorldInfo::Instance());
  for (auto dim: wi.getDimensions()) {
    addDimensionMenu(path, dim.path, dim.name, parent);
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
      emit dimensionChanged(*definitions[idx]);
    }
    menu->setEnabled(true);
  }
}


#define DIM_MAGIC 0x10000

void DimensionIdentifier::addDimensionMenu(QDir path, QString dir, QString name, QObject *parent) {
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
  bool found = false;
  // find index in definition list
  for (int idx = 0; idx<definitions.length(); idx++) {
    if (definitions[idx]->name == name) {
      action->setData(idx);
      found = true;
    }
  }
  if (!found) {
    // find index in custom dimension list
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
                  this, SLOT(changeViewToDimension()));
  menuActionGroup->addAction(action);
  currentMenuActions.append(action);
  foundDimensionDirs.append(dir);
}

void DimensionIdentifier::changeViewToDimension() {
  QAction *action = qobject_cast<QAction*>(sender());
  if (action) {
    int idx = action->data().toInt();
    if (idx < DIM_MAGIC) {
      emit dimensionChanged(*definitions[idx]);
    } else {
      WorldInfo & wi(WorldInfo::Instance());
      const QList<DimensionInfo> & custom = wi.getDimensions();
      emit dimensionChanged(custom[idx-DIM_MAGIC]);
    }
  }
}
