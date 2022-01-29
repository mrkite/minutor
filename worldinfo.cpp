/** Copyright (c) 2022, EtlamGit */

#include "worldinfo.h"

#include "nbt/nbt.h"


WorldInfo::WorldInfo()
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

  // Dimension Data

  // Spawn
  spawnX = data->at("SpawnX")->toInt();
  spawnZ = data->at("SpawnZ")->toInt();

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
}
