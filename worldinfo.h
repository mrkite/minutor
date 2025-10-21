/** Copyright (c) 2022, EtlamGit */
#ifndef WORLDINFO_H
#define WORLDINFO_H

#include <QString>
#include <QDir>
#include <QList>
#include <QMenu>

#include "identifier/dimensionidentifier.h"
#include "nbt/nbt.h"


class DatapackInfo {
 public:
  DatapackInfo()
    : path("")
    , enabled(false)
    , zipped(false) {}

  QString path;
  bool enabled;
  bool zipped;
};


class WorldInfo : public QObject
{
  Q_OBJECT

 public:
  // singleton: access to global usable instance
  static WorldInfo &Instance();

  void clear();
  bool parseWorldFolder(const QDir &path);
  bool parseWorldInfo();

  QString             getLevelName() const      { return levelName; };
  unsigned int        getDataVersion() const    { return dataVersion; };
  unsigned long long  getDayTime() const        { return dayTime; };
  int                 getSpawnX() const         { return spawnX; };
  int                 getSpawnZ() const         { return spawnZ; };
  const QString &     getSpawnDimension() const { return spawnDimension; };
  signed long long    getSeed() const           { return seed; };

  bool                isDatapackEnabled(const QString name) const;

  const QList<DimensionInfo> & getDimensions() const { return dimensions; };

  // Dimension view menu
  void clearDimensionsMenu(QMenu *menu);
  void getDimensionsInWorld(QDir path, QMenu *menu, QObject *parent);

 signals:
  void dimensionChanged(const DimensionInfo &dim);    // dimension changed in menu

 private slots:
  void changeViewToDimension();                       // dimension changed in menu

 private:
  // singleton: prevent access to constructor and copyconstructor
  WorldInfo();
  ~WorldInfo();
  WorldInfo(const WorldInfo &);
  WorldInfo &operator=(const WorldInfo &);

  void parseDatapacks(const Tag * data);
  bool parseDatapackNamespace(const QString name_space, const QString path, bool enabled, bool zipped);
  bool parseDimension(const QJsonDocument & json_doc, QString pack_name, QString dim_name);
  bool parseDimensionType(DimensionInfo & dim, const QString & dim_type_id);

  QDir                  folder;       // base folder of world
  QString               levelName;    // custom world name
  unsigned int          dataVersion;
  unsigned long long    dayTime;
  int                   spawnX;
  int                   spawnZ;
  QString               spawnDimension;
  long long             seed;

  QList<DimensionInfo>  dimensions;
  QMultiMap<QString, DatapackInfo> datapacks;

  // Dimension view menu
  void addDimensionToMenu(QDir path, QString dir, QString name, QObject *parent);

  QList<QAction *> currentMenuActions;
  QActionGroup *   menuActionGroup;
  QList<QString>   foundDimensionDirs;  // all directories where we already found a Dimension
};

#endif // WORLDINFO_H
