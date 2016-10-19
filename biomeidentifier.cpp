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

static int clamp(int v, int min, int max) {
  return (v < max ? (v > min ? v : min) : max);
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

    if (b->has("color")) {
      QString color = b->at("color")->asString();
      quint32 col = 0;
      for (int h = 0; h < color.length(); h++) {
        ushort c = color.at(h).unicode();
        col <<= 4;
        if (c >= '0' && c <= '9')
          col |= c - '0';
        else if (c >= 'A' && c <= 'F')
          col |= c - 'A' + 10;
        else if (c >= 'a' && c <= 'f')
          col |= c - 'a' + 10;
      }
      int rd = col >> 16;
      int gn = (col >> 8) & 0xff;
      int bl = col & 0xff;

      if (b->has("alpha"))
        biome->alpha = b->at("alpha")->asNumber();
      else
        biome->alpha = 1.0;

      // pre multiply alphas
      rd *= biome->alpha;
      gn *= biome->alpha;
      bl *= biome->alpha;

      // pre-calculate light spectrum
      double y = 0.299 * rd + 0.587 * gn + 0.114 * bl;
      double u = (bl - y) * 0.565;
      double v = (rd - y) * 0.713;
      double delta = y / 15;
      for (int i = 0; i < 16; i++) {
        y = i * delta;
        rd = (unsigned int)clamp(y + 1.403 * v, 0, 255);
        gn = (unsigned int)clamp(y - 0.344 * u - 0.714 * v, 0, 255);
        bl = (unsigned int)clamp(y + 1.770 * u, 0, 255);
        biome->colors[i] = (rd << 16) | (gn << 8) | bl;
      }
    } else {
      biome->alpha = 0.0;
    }

    biomes[id].append(biome);
    packs[pack].append(biome);
  }

  return pack;
}
