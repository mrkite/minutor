/** Copyright (c) 2013, Sean Kasun */

#include <assert.h>

#include "./biomeidentifier.h"
#include "./json.h"

static BiomeInfo unknownBiome;

BiomeInfo::BiomeInfo()
  : name("Unknown Biome")
  , enabled(false)
  , watercolor(255,255,255)
  , alpha(1.0)
  , temperature(0.5)
  , rainfall(0.5)
{}


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

    // check for "alpha" (0: transparent / 1: saturated)
    // probably never used
    if (b->has("alpha"))
      biome->alpha = b->at("alpha")->asNumber();

    // get temperature definition
    if (b->has("temperature"))
      biome->temperature = b->at("temperature")->asNumber();

    // get rainfall definition
    if (b->has("rainfall"))
      biome->rainfall = b->at("rainfall")->asNumber();

    // get watercolor definition
    if (b->has("watercolor")) {
      biome->watercolor.setNamedColor(b->at("watercolor")->asString());
      assert(biome->watercolor.isValid());
    }

    // get color definition
    QColor biomecolor;
    if (b->has("color")) {
      QString colorname = b->at("color")->asString();
      if (colorname.length() == 6) {
        // check if this is an old color definition with missing '#'
        bool ok;
        colorname.toInt(&ok,16);
        if (ok)
          colorname.push_front('#');
      }
      biomecolor.setNamedColor(colorname);
      assert(biomecolor.isValid());
    } else {
      // use hashed by name instead
      quint32 hue = qHash(biome->name);
      biomecolor.setHsv(hue % 360, 255, 255);
    }

    // pre-calculate light spectrum
    for (int i = 0; i < 16; i++) {
      // calculate light attenuation similar to Minecraft
      // except base 90% here, were Minecraft is using 80% per level
      double light_factor = pow(0.90,15-i);
      biome->colors[i].setRgb(light_factor*biomecolor.red(),
                              light_factor*biomecolor.green(),
                              light_factor*biomecolor.blue(),
                              255*biome->alpha );
    }

    biomes[id].append(biome);
    packs[pack].append(biome);
  }

  return pack;
}
