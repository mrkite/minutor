/** Copyright (c) 2013, Sean Kasun */

#include <QDebug>
#include <QtWidgets/QMessageBox>
#include <assert.h>
#include <cmath>

#include "./blockidentifier.h"
#include "./json.h"

static BlockInfo unknownBlock;

BlockInfo::BlockInfo()
  : variants(false)
  , transparent(false)
  , liquid(false)
  , rendernormal(true)
  , providepower(false)
  , spawninside(false)
  , grass(false)
  , foliage(false) {}

bool BlockInfo::hasVariants() const {
  return this->variants;
}

bool BlockInfo::isOpaque() const {
  return !(this->transparent);
}

bool BlockInfo::isLiquid() const {
  return this->liquid;
}

bool BlockInfo::doesBlockHaveSolidTopSurface() const {
  if (this->isOpaque() && this->renderAsNormalBlock()) return true;
  if (this->hopper) return true;
  return false;
}

bool BlockInfo::isBlockNormalCube() const {
  return this->isOpaque() && this->renderAsNormalBlock() &&
      !this->canProvidePower();
}

bool BlockInfo::renderAsNormalBlock() const {
  return this->rendernormal;
}

bool BlockInfo::canProvidePower() const {
  return this->providepower;
}

void BlockInfo::setName(const QString & newname) {
  // set name
  name = newname;
  // precompute mob spawning conditions
  bedrock = this->name.contains("Bedrock", Qt::CaseInsensitive);
  hopper  = this->name.contains("Hopper", Qt::CaseInsensitive);
  snow    = this->name.contains("Snow", Qt::CaseInsensitive);
  // precompute biome based watercolormodifier
  water   = this->name.contains("Water", Qt::CaseInsensitive);
}

const QString & BlockInfo::getName() { return name; }


bool BlockInfo::isBedrock()  { return bedrock; }
bool BlockInfo::isHopper()   { return hopper; }
bool BlockInfo::isSnow()     { return snow; }

bool BlockInfo::biomeWater()   { return water; }
bool BlockInfo::biomeGrass()   { return grass; }
bool BlockInfo::biomeFoliage() { return foliage; }

void BlockInfo::setBiomeGrass(bool value)   { grass = value; }
void BlockInfo::setBiomeFoliage(bool value) { foliage = value; }


// --------- --------- --------- ---------
// BlockIdentifier
// --------- --------- --------- ---------

BlockIdentifier::BlockIdentifier() {
  for (int i = 0; i < 16; i++)
    unknownBlock.colors[i] = 0xff00ff;
  unknownBlock.alpha = 1.0;
  // TODO: Hoist string literal into named constant
  unknownBlock.setName("Unknown Block");
}

BlockIdentifier::~BlockIdentifier() {
  for (int i = 0; i < packs.length(); i++) {
    for (int j = 0; j < packs[i].length(); j++)
      delete packs[i][j];
  }
}

BlockIdentifier& BlockIdentifier::Instance() {
  static BlockIdentifier singleton;
  return singleton;
}

BlockInfo &BlockIdentifier::getBlockInfo(uint hid) {
  if (blocks.contains(hid)) {
    return *blocks[hid];
  }
  // no blocks at all found.. dammit
  return unknownBlock;
}

bool BlockIdentifier::hasBlockInfo(uint hid) {
  return blocks.contains(hid);
}

QList<quint32> BlockIdentifier::getKnownIds() const
{
    return blocks.keys();
}

void BlockIdentifier::enableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = true;
}

void BlockIdentifier::disableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = false;
}

int BlockIdentifier::addDefinitions(JSONArray *defs, int pack) {
  if (pack == -1) {
    pack = packs.length();
    packs.append(QList<BlockInfo*>());
  }
  int len = defs->length();
  for (int i = 0; i < len; i++)
    parseDefinition(dynamic_cast<JSONObject *>(defs->at(i)), NULL, pack);
  return pack;
}

void BlockIdentifier::parseDefinition(JSONObject *b, BlockInfo *parent,
                                      int pack) {
  BlockInfo *block = new BlockInfo();

//  int id;
//  if (parent == NULL) {
//    id = b->at("id")->asNumber();
//  } else {
//    id = parent->id;
//    int data = b->at("data")->asNumber();
//    id |= data << 12;
//  }
//  block->id = id;

  // name of Block
  QString name;
  if (b->has("name"))
    name = b->at("name")->asString();
  else if (parent != NULL)
    name = parent->getName();
  else
    name = "Unknown";
  block->setName(name);
  block->enabled = true;

  // optional Block State
  if (b->has("blockstate"))
    block->blockstate = b->at("blockstate")->asString();

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

  // generic attributes
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

  // variants due to block_state difference
  if (b->has("variants")) {
    block->variants = true;
    JSONArray *variants = dynamic_cast<JSONArray *>(b->at("variants"));
    int vlen = variants->length();
    for (int j = 0; j < vlen; j++)
      parseDefinition(dynamic_cast<JSONObject *>(variants->at(j)), block, pack);
  }

  uint hid = qHash(name);
  if (!block->blockstate.isEmpty())
    hid = qHash(name + ":" + block->blockstate);
  if (blocks.contains(hid)) {
    // this will only trigger during development of vanilla_blocks.json
    // and prevents generating a wrong definition file
    QMessageBox::warning((QWidget*)(NULL),
                         "Error loading Block definition: " + name,
                         "Failed to add Block from definition file, as it might be a duplicate\nor generates the same hash as an already existing Block." ,
                         QMessageBox::Cancel, QMessageBox::Cancel);
  }
  blocks.insert(hid, block);
  packs[pack].append(block);

  // we need this ugly code to allow mob spawn detection
  // todo: rework mob spawn highlight
  if (block->getName() == "minecraft:air") {
    blocks.insert(0, block);
  }
}
