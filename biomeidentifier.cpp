/** Copyright (c) 2013, Sean Kasun */

#include "./biomeidentifier.h"
#include "./json.h"

static BiomeInfo unknownBiome;
BiomeIdentifier::BiomeIdentifier() {
  unknownBiome.name = "Unknown";
}
BiomeIdentifier::~BiomeIdentifier() {
  for (int i = 0; i < packs.length(); i++) {
    for (int j = 0; j < packs[i].length(); j++)
      delete packs[i][j];
  }
}

BiomeInfo &BiomeIdentifier::getBiome(int biome) {
  QList<BiomeInfo*> &list = biomes[biome];
  // search backwards for priority sorting to work
  for (int i = list.length() - 1; i >= 0; i--)
    if (list[i]->enabled)
      return *list[i];
  return unknownBiome;
}

void BiomeIdentifier::enableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = true;
}
void BiomeIdentifier::disableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = false;
}

int BiomeIdentifier::addDefinitions(JSONArray *defs, int pack) {
  if (pack == -1) {
    pack = packs.length();
    packs.append(QList<BiomeInfo *>());
  }

  int len = defs->length();
  for (int i = 0; i < len; i++) {
    JSONObject *b = dynamic_cast<JSONObject *>(defs->at(i));
    int id = b->at("id")->asNumber();

    BiomeInfo *biome = new BiomeInfo();
    biome->enabled = true;
    if (b->has("name"))
      biome->name = b->at("name")->asString();
    else
      biome->name = "Unknown";
    biomes[id].append(biome);
    packs[pack].append(biome);
  }
  return pack;
}
