/** Copyright (c) 2013, Sean Kasun */

#include <QDebug>
#include <assert.h>

#include "./blockidentifier.h"
#include "./json.h"

static BlockInfo unknownBlock;

BlockInfo::BlockInfo() : transparent(false), liquid(false), rendernormal(true),
  providepower(false), spawninside(false) {}

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
  name = newname;
  bedrock = this->name.contains("Bedrock");
  hopper = this->name.contains("Hopper");
  stairs = this->name.contains("Stairs");
  halfslab = this->name.contains("Slab") && !this->name.contains("Double") &&
      !this->name.contains("Full");
  snow = this->name.contains("Snow");
}

const QString & BlockInfo::getName() { return name; }


bool BlockInfo::isBedrock()  { return bedrock; }
bool BlockInfo::isHopper()   { return hopper; }
bool BlockInfo::isStairs()   { return stairs; }
bool BlockInfo::isHalfSlab() { return halfslab; }
bool BlockInfo::isSnow()     { return snow; }



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
BlockInfo &BlockIdentifier::getBlock(int id, int data) {
  // first apply the mask
  if (blocks.contains(id))
    data &= blocks[id].first()->mask;

  quint32 bid = id | (data << 12);
  // first check the cache
  if (cache[bid] != NULL)
    return *cache[bid];

  // now find the variant
  if (blocks.contains(bid)) {
    QList<BlockInfo*> &list = blocks[bid];
    // run backwards for priority sorting
    for (int i = list.length() - 1; i >= 0; i--) {
      if (list[i]->enabled) {
        cache[bid] = list[i];
        return *list[i];
      }
    }
  }
  // no enabled variant found
  if (blocks.contains(id)) {
    QList<BlockInfo*> &list = blocks[id];
    for (int i = list.length() - 1; i >= 0; i--) {
      if (list[i]->enabled) {
        cache[bid] = list[i];
        return *list[i];
      }
    }
  }
  // no blocks at all found.. dammit
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

static int clamp(int v, int min, int max) {
  return (v < max ? (v > min ? v : min) : max);
}

void BlockIdentifier::clearCache() {
  for (int i = 0; i < 65536; i++) {
    cache[i] = NULL;
  }
}

void BlockIdentifier::parseDefinition(JSONObject *b, BlockInfo *parent,
                                      int pack) {
  int id;
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
    block->setName(b->at("name")->asString());
  else if (parent != NULL)
    block->setName(parent->getName());
  else
    block->setName("Unknown");
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

  // pre multiply alphas
//rd *= block->alpha;
//gn *= block->alpha;
//bl *= block->alpha;
  // ??? why should do we do this ???
  // a transparent block is "blended" correctly in mapview.cpp
  // if we change anything to the color here, we just change it...
  // for this special case: we make all transparent blocks darker

  // pre-calculate light spectrum
  for (int i = 0; i < 16; i++) {

    // calculate light attenuation similar to Minecraft
    // except base 90% here, were Minecraft is using 80% per level
    double light_factor = pow(0.90,15-i);

    block->colors[i].setHsv( blockcolor.hue(),
                             blockcolor.saturation(),
                             blockcolor.value()*light_factor,
//                           blockcolor.value()*(i/15.0),
//                           blockcolor.value()*(i/15.0)*block->alpha, // with pre-multiply alpha
                             255*block->alpha );
  }

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

  blocks[id].append(block);
  packs[pack].append(block);
}
