/** Copyright (c) 2022, EtlamGit */
#ifndef WORLDINFO_H
#define WORLDINFO_H

#include <QDir>


class WorldInfo
{
 public:
  // singleton: access to global usable instance
  static WorldInfo &Instance();

  void clear();
  bool parseFolder(const QDir &path);

  QString             getLevelName() const   { return levelName; };
  unsigned int        getDataVersion() const { return dataVersion; };
  unsigned long long  getDayTime() const     { return dayTime; };
  int                 getSpawnX() const      { return spawnX; };
  int                 getSpawnZ() const      { return spawnZ; };

 private:
  // singleton: prevent access to constructor and copyconstructor
  WorldInfo();
  ~WorldInfo();
  WorldInfo(const WorldInfo &);
  WorldInfo &operator=(const WorldInfo &);

  QDir                folder;       // base folder of world
  QString             levelName;    // custom world name
  unsigned int        dataVersion;
  unsigned long long  dayTime;
  int                 spawnX;
  int                 spawnZ;
};

#endif // WORLDINFO_H
