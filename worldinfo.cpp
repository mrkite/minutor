/** Copyright (c) 2022, EtlamGit */

#include "worldinfo.h"

#include "nbt/nbt.h"
#include "zipreader.h"
#include "json/json.h"


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
      QStringRef packname(&dp, 5, dp.size()-5-4);

      // parse custom Dimension Data
      // located in ./datapacks/<packname>.zip -> ./data/<packname>/dimension_type/<dimensions>.json
      ZipReader zip(folder.path() + "/datapacks/" + packname + ".zip");
      if (zip.open()) {
        for (auto & file: zip.getFileList()) {
          if (file.startsWith("data/" + packname + "/dimension_type/") &&
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
            DimensionInfo dim;
            dim.path = "./dimensions/" + packname + "/" + f.baseName();
            if (json->has("name"))
              dim.name = json->at("name")->asString();

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
