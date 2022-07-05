/** Copyright (c) 2013, Sean Kasun */

#include <QDebug>
#include <QtWidgets/QMessageBox>
#include <assert.h>
#include <cmath>

#include "blockidentifier.h"

static BlockInfo unknownBlock;

BlockInfo::BlockInfo()
  : variants(false)
  , transparent(false)
  , liquid(false)
  , rendernormal(true)
  , providepower(false)
  , spawninside(false)
  , spawnontop(false)
  , bedrock(false)
  , hopper(false)
  , snow(false)
  , water(false)
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
  if (this->spawnontop) return true;  // shortcut for normal opaque Blocks and top Slabs/Stairs
  if (this->hopper) return true;
  if (this->isOpaque() && this->renderAsNormalBlock()) return true;
  return false;
}

bool BlockInfo::isBlockNormalCube() const {
  return this->isOpaque() &&
         this->renderAsNormalBlock() &&
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
  water   = this->name.contains("Water", Qt::CaseInsensitive)
         || this->name.contains("Bubble_Column", Qt::CaseInsensitive);
}

const QString & BlockInfo::getName() const { return name; }


bool BlockInfo::isBedrock() const  { return bedrock; }
bool BlockInfo::isHopper() const   { return hopper; }
bool BlockInfo::isSnow() const     { return snow; }

bool BlockInfo::biomeWater() const   { return water; }
bool BlockInfo::biomeGrass() const   { return grass; }
bool BlockInfo::biomeFoliage() const { return foliage; }

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

const BlockInfo &BlockIdentifier::getBlockInfo(uint hid) const {
  auto iter = blocks.find(hid);
  if (iter != blocks.end())
    return *iter.value();
  // no blocks at all found.. dammit
  return unknownBlock;
}

bool BlockIdentifier::hasBlockInfo(uint hid) const {
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

int BlockIdentifier::addDefinitions(QJsonArray defs, int pack) {
  if (pack == -1) {
    pack = packs.length();
    packs.append(QList<BlockInfo*>());
  }
  int len = defs.size();
  for (int i = 0; i < len; i++)
    parseDefinition(defs.at(i).toObject(), NULL, pack);
  return pack;
}

void BlockIdentifier::parseDefinition(QJsonObject b, BlockInfo *parent,
                                      int pack) {
  BlockInfo *block = new BlockInfo();

//  int id;
//  if (parent == NULL) {
//    id = b.value("id").toInt();
//  } else {
//    id = parent->id;
//    int data = b.value("data").toInt();
//    id |= data << 12;
//  }
//  block->id = id;

  // name of Block
  QString name;
  if (b.contains("name"))
    name = b.value("name").toString();
  else if (parent != NULL)
    name = parent->getName();
  else
    name = "Unknown";
  block->setName(name);
  block->enabled = true;

  // optional Block State
  if (b.contains("blockstate"))
    block->blockstate = b.value("blockstate").toString();

  if (b.contains("transparent")) {
    // default setting for a transparent Block
    block->transparent = b.value("transparent").toBool();
    block->rendernormal = false;
    block->spawninside = false;
    block->spawnontop = false;
  } else if (parent != NULL) {
    // copy parent attributes as default
    block->transparent = parent->transparent;
    block->rendernormal = parent->rendernormal;
    block->spawninside = parent->spawninside;
    block->spawnontop = parent->spawnontop;
  } else {
    // default setting for opaque Block
    block->transparent = false;
    block->rendernormal = true;
    block->spawninside = false;
    block->spawnontop = true;
  }
  // override these attributes when explicitly given
  if (b.contains("rendercube"))
    block->rendernormal = b.value("rendercube").toBool();
  if (b.contains("spawninside"))
    block->spawninside = b.value("spawninside").toBool();
  if (b.contains("spawnontop"))
    block->spawnontop = b.value("spawnontop").toBool();


  // generic attributes
  if (b.contains("liquid"))
    block->liquid = b.value("liquid").toBool();
  else if (parent != NULL)
    block->liquid = parent->liquid;
  else
    block->liquid = false;

  if (b.contains("canProvidePower"))
    block->providepower = b.value("canProvidePower").toBool();
  else if (parent != NULL)
    block->providepower = parent->providepower;
  else
    block->providepower = false;

  if (b.contains("alpha"))
    block->alpha = b.value("alpha").toDouble();
  else if (parent != NULL)
    block->alpha = parent->alpha;
  else
    block->alpha = 1.0;

  QColor blockcolor;
  if (b.contains("color")) {
    QString colorname = b.value("color").toString();
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
  if (b.contains("biomeGrass"))
    block->setBiomeGrass( b.value("biomeGrass").toBool() );
  if (b.contains("biomeFoliage"))
    block->setBiomeFoliage( b.value("biomeFoliage").toBool() );

  // variants due to block_state difference
  if (b.contains("variants")) {
    block->variants = true;
    QJsonArray variants = b.value("variants").toArray();
    int vlen = variants.size();
    for (int j = 0; j < vlen; j++)
      parseDefinition(variants.at(j).toObject(), block, pack);
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
