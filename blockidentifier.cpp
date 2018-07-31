/** Copyright (c) 2013, Sean Kasun */

#include <QDebug>
#include <assert.h>
#include <cmath>

#include "./blockidentifier.h"
#include "./json.h"

static BlockInfo unknownBlock;

BlockInfo::BlockInfo() : transparent(false), liquid(false), rendernormal(true),
  providepower(false), spawninside(false), grass(false), foliage(false) {}

bool BlockInfo::isOpaque() {
  return !(this->transparent);
}

bool BlockInfo::isLiquid() {
  return this->liquid;
}

bool BlockInfo::doesBlockHaveSolidTopSurface(int data) {
  if (this->isOpaque() && this->renderAsNormalBlock()) return true;
  if (this->stairs && ((data & 4) == 4)) return true;
  if (this->halfslab && ((data & 8) == 8)) return true;
  if (this->hopper) return true;
  if (this->snow && ((data & 7) == 7)) return true;
  return false;
}

bool BlockInfo::isBlockNormalCube() {
  return this->isOpaque() && this->renderAsNormalBlock() &&
      !this->canProvidePower();
}

bool BlockInfo::renderAsNormalBlock() {
  return this->rendernormal;
}

bool BlockInfo::canProvidePower() {
  return this->providepower;
}

void BlockInfo::setName(const QString & newname) {
  // set name
  name = newname;
  // precompute mob spawning conditions
  bedrock = this->name.contains("Bedrock");
  hopper = this->name.contains("Hopper");
  stairs = this->name.contains("Stairs");
  halfslab = this->name.contains("Slab") && !this->name.contains("Double") &&
      !this->name.contains("Full");
  snow = this->name.contains("Snow");
  // precompute biome based watercolormodifier
  water = this->name.contains("water");
}

const QString & BlockInfo::getName() { return name; }


bool BlockInfo::isBedrock()  { return bedrock; }
bool BlockInfo::isHopper()   { return hopper; }
bool BlockInfo::isStairs()   { return stairs; }
bool BlockInfo::isHalfSlab() { return halfslab; }
bool BlockInfo::isSnow()     { return snow; }

bool BlockInfo::biomeWater()   { return water; }
bool BlockInfo::biomeGrass()   { return grass; }
bool BlockInfo::biomeFoliage() { return foliage; }

void BlockInfo::setBiomeGrass(bool value)   { grass = value; }
void BlockInfo::setBiomeFoliage(bool value) { foliage = value; }

BlockIdentifier::BlockIdentifier() {
  // clear cache pointers
  for (int i = 0; i < 65536; i++)
    cache[i] = NULL;
  for (int i = 0; i < 16; i++)
    unknownBlock.colors[i] = 0xff00ff;
  unknownBlock.alpha = 1.0;
  unknownBlock.setName("Unknown");
}
BlockIdentifier::~BlockIdentifier() {
  clearCache();
  for (int i = 0; i < packs.length(); i++) {
    for (int j = 0; j < packs[i].length(); j++)
      delete packs[i][j];
  }
}

// this routine is ridiculously slow
BlockInfo &BlockIdentifier::getBlock(QString name, int data) {
  // first apply the mask
  if (blocks.contains(name)) {
    return *(blocks[name].first());
  }
  //   data &= blocks[name].first()->mask;
  //
  // quint32 bid = id | (data << 12);
  // // first check the cache
  // if (cache[bid] != NULL)
  //   return *cache[bid];
  //
  // // now find the variant
  // if (blocks.contains(bid)) {
  //   QList<BlockInfo*> &list = blocks[bid];
  //   // run backwards for priority sorting
  //   for (int i = list.length() - 1; i >= 0; i--) {
  //     if (list[i]->enabled) {
  //       cache[bid] = list[i];
  //       return *list[i];
  //     }
  //   }
  // }
  // // no enabled variant found
  // if (blocks.contains(id)) {
  //   QList<BlockInfo*> &list = blocks[id];
  //   for (int i = list.length() - 1; i >= 0; i--) {
  //     if (list[i]->enabled) {
  //       cache[bid] = list[i];
  //       return *list[i];
  //     }
  //   }
  // }
  // no blocks at all found.. dammit
  unknownBlock.setName(name);
  return unknownBlock;
}

void BlockIdentifier::enableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = true;
  // clear cache
  clearCache();
}

void BlockIdentifier::disableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = false;
  // clear cache
  clearCache();
}

int BlockIdentifier::addDefinitions(JSONArray *defs, int pack) {
  if (pack == -1) {
    pack = packs.length();
    packs.append(QList<BlockInfo*>());
  }
  int len = defs->length();
  for (int i = 0; i < len; i++)
    parseDefinition(dynamic_cast<JSONObject *>(defs->at(i)), NULL, pack);
  // clear cache
  clearCache();
  return pack;
}

void BlockIdentifier::clearCache() {
  for (int i = 0; i < 65536; i++) {
    cache[i] = NULL;
  }
}

void BlockIdentifier::parseDefinition(JSONObject *b, BlockInfo *parent,
                                      int pack) {
  int id;
  QString name;
  if (parent == NULL) {
    id = b->at("id")->asNumber();
  } else {
    id = parent->id;
    int data = b->at("data")->asNumber();
    id |= data << 12;
  }
  BlockInfo *block = new BlockInfo();
  block->id = id;

  if (b->has("name"))
    name = b->at("name")->asString();
  else if (parent != NULL)
    name = parent->getName();
  else
    name = "Unknown";
  block->setName(name);
  block->enabled = true;

  if (b->has("transparent")) {
    block->transparent = b->at("transparent")->asBool();
    block->rendernormal = false;  // for most cases except the following
    if (b->has("rendercube"))
      block->rendernormal = b->at("rendercube")->asBool();
    block->spawninside = false;  // for most cases except the following
    if (b->has("spawninside"))
      block->spawninside = b->at("spawninside")->asBool();
  } else if (parent != NULL) {
    block->transparent = parent->transparent;
    block->rendernormal = parent->rendernormal;
    block->spawninside = parent->spawninside;
  } else {
    block->transparent = false;
    block->rendernormal = true;
    block->spawninside = false;
  }

  if (b->has("liquid"))
    block->liquid = b->at("liquid")->asBool();
  else if (parent != NULL)
    block->liquid = parent->liquid;
  else
    block->liquid = false;

  if (b->has("canProvidePower"))
    block->providepower = b->at("canProvidePower")->asBool();
  else if (parent != NULL)
    block->providepower = parent->providepower;
  else
    block->providepower = false;

  if (b->has("alpha"))
    block->alpha = b->at("alpha")->asNumber();
  else if (parent != NULL)
    block->alpha = parent->alpha;
  else
    block->alpha = 1.0;

  QColor blockcolor;
  if (b->has("color")) {
    QString colorname = b->at("color")->asString();
    if (colorname.length() == 6) {
      // check if this is an old color definition with missing '#'
      bool ok;
      colorname.toInt(&ok,16);
      if (ok)
        colorname.push_front('#');
    }
    blockcolor.setNamedColor(colorname);
    assert(blockcolor.isValid());
  } else if (parent != NULL) {
    // copy brightest color from parent
    blockcolor = parent->colors[15];
  } else {
    // use hashed by name instead
    quint32 hue = qHash(block->getName());
    blockcolor.setHsv(hue % 360, 255, 255);
  }

  // pre-calculate light spectrum
  for (int i = 0; i < 16; i++) {
    // calculate light attenuation similar to Minecraft
    // except base 90% here, were Minecraft is using 80% per level
    double light_factor = pow(0.90,15-i);
    block->colors[i].setRgb(light_factor*blockcolor.red(),
                            light_factor*blockcolor.green(),
                            light_factor*blockcolor.blue(),
                            255*block->alpha );
  }

  // biome dependant color
  if (b->has("biomeGrass"))
    block->setBiomeGrass( b->at("biomeGrass")->asBool() );
  if (b->has("biomeFoliage"))
    block->setBiomeFoliage( b->at("biomeFoliage")->asBool() );

  // variant reduction mask
  if (b->has("mask"))
    block->mask = b->at("mask")->asNumber();
  else if (b->has("variants"))
    block->mask = 0x0f;
  else
    block->mask = 0x00;

  if (b->has("variants")) {
    JSONArray *variants = dynamic_cast<JSONArray *>(b->at("variants"));
    int vlen = variants->length();
    for (int j = 0; j < vlen; j++)
      parseDefinition(dynamic_cast<JSONObject *>(variants->at(j)), block, pack);
  }

  blocks[name].append(block);
  packs[pack].append(block);
}
