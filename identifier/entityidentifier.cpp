/** Copyright (c) 2014, EtlamGit */
#include <QDebug>
#include <assert.h>

#include "entityidentifier.h"

EntityInfo::EntityInfo(QString name, QString category, QColor brushColor,
                       QColor penColor) : name(name), category(category),
  brushColor(brushColor), penColor(penColor) {}

// dummy for entities not found in *_entity.json
static EntityInfo entityDummy("Name unknown", "Others",  // Name Category
                              Qt::black, Qt::black);     // Black circle with Black border


// --------- --------- --------- ---------
// EntityIdentifier
// --------- --------- --------- ---------

EntityIdentifier::EntityIdentifier() {}
EntityIdentifier::~EntityIdentifier() {}

EntityIdentifier& EntityIdentifier::Instance() {
  static EntityIdentifier singleton;
  return singleton;
}

void EntityIdentifier::enableDefinitions(int packID) {
  if (packID < 0) return;
  for (auto it = packs.begin(); it != packs.end(); ++it) {
    if (it->packID == packID) {
      it->enabled = true;
      return;
    }
  }
}

void EntityIdentifier::disableDefinitions(int packID) {
  if (packID < 0) return;
  for (auto it = packs.begin(); it != packs.end(); ++it) {
    if (it->packID == packID)
      it->enabled = false;
  }
}

int EntityIdentifier::addDefinitions(QJsonArray defs, int packID) {
  if (packID == -1) {
    // find largest used packID
    for (auto it = packs.constBegin(); it != packs.constEnd(); ++it) {
      if (it->packID > packID)
        packID = it->packID;
    }
    packID++;  // use one higher than largest found
    packs.append(TpackInfo(packID));
  }
  int len = defs.size();
  for (int i = 0; i < len; i++)
    parseCategoryDefinition(defs.at(i).toObject(), packID);
  return packID;
}

EntityIdentifier::TentityMap& EntityIdentifier::getMapForPackID(int packID) {
  for (auto it = packs.begin(); it != packs.end(); ++it) {
    if (it->packID == packID)
      return it->map;
  }
  return dummyMap;
}

void EntityIdentifier::parseCategoryDefinition(QJsonObject data, int packID) {
  QString category;
  if (data.contains("category"))
    category = data.value("category").toString();
  else
    category = "Unknown";

  QColor catcolor;
  if (data.contains("catcolor")) {
    QString colorname = data.value("catcolor").toString();
    catcolor.setNamedColor(colorname);
    assert(catcolor.isValid());
  } else {  // use hashed by name instead
    quint32 hue = qHash(category);
    catcolor.setHsv(hue % 360, 255, 255);
  }
  addCategory(qMakePair(category, catcolor));

  if (data.contains("entity")) {
    QJsonArray entities = data.value("entity").toArray();
    int len = entities.size();

    for (int e = 0; e < len; e++)
      parseEntityDefinition(entities.at(e).toObject(),
                            category, catcolor, packID);
  }
}

void EntityIdentifier::parseEntityDefinition(QJsonObject entity,
                                             QString const &category,
                                             QColor catcolor, int packID) {
  QString id("unknown");
  if (entity.contains("id"))
    id = entity.value("id").toString().toLower();

  if (entity.contains("catcolor")) {
    QString colorname = entity.value("catcolor").toString();
    catcolor.setNamedColor(colorname);
    assert(catcolor.isValid());
  }

  QColor color;
  if (entity.contains("color")) {
    QString colorname = entity.value("color").toString();
    color.setNamedColor(colorname);
    assert(color.isValid());
  } else {  // use hashed by name instead
    quint32 hue = qHash(id);
    color.setHsv(hue % 360, 255, 255);
  }

  QString name;
  if (entity.contains("name")) {
    // use provided name
    name = entity.value("name").toString();
  } else {
    // or try to build name automatically
    // split at underscores
    QStringList tokens = id.toLower().replace('_',' ').split(" ");
    // make first character uppercase
    for (QList<QString>::iterator tokItr = tokens.begin(); tokItr != tokens.end(); ++tokItr) {
      (*tokItr) = (*tokItr).at(0).toUpper() + (*tokItr).mid(1);
    }
    name = tokens.join(" ");
  }

  // enter entity into manager
  TentityMap& map = getMapForPackID(packID);
  map.insert(id, EntityInfo(name, category, catcolor, color));

  // add duplicates: when new 1.11+ or 1.13+ id definitions are available
  // legacy id is stored in own definition element (duplicates automatically)
  if (entity.contains("idlist")) {
    QJsonArray idlist = entity.value("idlist").toArray();
    int len = idlist.size();
    for (int j = 0; j < len; j++) {
      QString idl = idlist.at(j).toString().toLower();
      map.insert(idl, EntityInfo(name, category, catcolor, color));
    }
  }
}

bool EntityIdentifier::addCategory(QPair<QString, QColor> cat) {
  for (auto it = categories.constBegin(); it != categories.constEnd(); ++it) {
    if (it->first == cat.first)
      return false;
  }
  categories.append(cat);
  return true;
}

int EntityIdentifier::getNumCategories() const {
  return categories.size();
}

EntityIdentifier::TcatList const &EntityIdentifier::getCategoryList() const {
  return categories;
}

QColor EntityIdentifier::getCategoryColor(const QString name) const {
  for (auto it = categories.constBegin(); it != categories.constEnd(); ++it) {
    if (it->first == name)
      return it->second;
  }
  return Qt::black;  // dummy
}

EntityInfo const &EntityIdentifier::getEntityInfo(const QString id) const {
  for (auto it = packs.constBegin(); it != packs.constEnd(); ++it) {
    if (it->enabled) {
      // convert ID to lower case and strip "minecraft:" if exists
      TentityMap::const_iterator info = it->map.find(id.toLower().remove("minecraft:"));
      if (info != it->map.end()) {  // found it
        return info.value();
      }
    }
  }
  return entityDummy;
}

QColor EntityIdentifier::getBrushColor(const QString id) const {
  return getEntityInfo(id).brushColor;
}

QColor EntityIdentifier::getPenColor(const QString id) const {
  return getEntityInfo(id).penColor;
}

QList<QString> EntityIdentifier::getKnownIds() const {
  QList<QString> temp;
  for (auto it = packs.constBegin(); it != packs.constEnd(); ++it) {
    if (it->enabled) {
      for (QString& e: it->map.keys())
        temp.append(e);
    }
  }
  return temp;
}
