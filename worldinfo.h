/** Copyright (c) 2022, EtlamGit */
#ifndef WORLDINFO_H
#define WORLDINFO_H

#include <QString>
#include <QDir>
#include <QList>

#include "identifier/dimensionidentifier.h"


class WorldInfo
{
 public:
  // singleton: access to global usable instance
  static WorldInfo &Instance();

  void clear();
  bool parseFolder(const QDir &path);
  bool parseDimensions();

  QString             getLevelName() const   { return levelName; };
  unsigned int        getDataVersion() const { return dataVersion; };
  unsigned long long  getDayTime() const     { return dayTime; };
  int                 getSpawnX() const      { return spawnX; };
  int                 getSpawnZ() const      { return spawnZ; };

  const QList<DimensionInfo> & getDimensions() const { return dimensions; };

 private:
  // singleton: prevent access to constructor and copyconstructor
  WorldInfo();
  ~WorldInfo();
  WorldInfo(const WorldInfo &);
  WorldInfo &operator=(const WorldInfo &);

  QDir                  folder;       // base folder of world
  QString               levelName;    // custom world name
  unsigned int          dataVersion;
  unsigned long long    dayTime;
  int                   spawnX;
  int                   spawnZ;

  QList<DimensionInfo>  dimensions;
};

#endif // WORLDINFO_H
