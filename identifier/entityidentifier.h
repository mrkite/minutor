/** Copyright (c) 2014, EtlamGit */
#ifndef ENTITYIDENTIFIER_H_
#define ENTITYIDENTIFIER_H_

#include <QString>
#include <QColor>
#include <QList>
#include <QMap>
#include <QJsonArray>
#include <QJsonObject>


class EntityInfo {
 public:
  EntityInfo(QString name, QString category, QColor brushColor,
             QColor penColor);
  QString name;
  QString category;
  QColor  brushColor;
  QColor  penColor;
};

class EntityIdentifier {
 public:
  // singleton: access to global usable instance
  static EntityIdentifier &Instance();

  int addDefinitions(QJsonArray defs, int packID = -1);
  void enableDefinitions(int id);
  void disableDefinitions(int id);

  // interface to list of main categories
  typedef QList<QPair<QString, QColor>> TcatList;
  int getNumCategories() const;
  TcatList const &getCategoryList() const;
  QColor getCategoryColor(const QString name) const;

  // interface to single EntityInfo objects
  EntityInfo const &getEntityInfo(const QString id) const;
  QColor getBrushColor(const QString id) const;
  QColor getPenColor(const QString id) const;

  QList<QString> getKnownIds() const;

 private:
  // singleton: prevent access to constructor and copyconstructor
  EntityIdentifier();
  ~EntityIdentifier();
  EntityIdentifier(const EntityIdentifier &);
  EntityIdentifier &operator=(const EntityIdentifier &);

  void parseCategoryDefinition(QJsonObject data, int packID);
  void parseEntityDefinition(QJsonObject entity, QString const &category,
                             QColor catcolor, int packID);

  TcatList categories;  // main categories for entities
  bool addCategory(QPair<QString, QColor> cat);

  typedef QMap<QString, EntityInfo> TentityMap;  // key:id_name value:EntityInfo
  TentityMap dummyMap;

  class TpackInfo {
   public:
    int        packID;
    bool       enabled;
    TentityMap map;
    explicit TpackInfo(int packID) : packID(packID), enabled(true) {}
  };
  QList< TpackInfo > packs;
  TentityMap& getMapForPackID(int packID);
};

#endif  // ENTITYIDENTIFIER_H_
