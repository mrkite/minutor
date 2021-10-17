/** Copyright (c) 2013, Sean Kasun */

#include <algorithm>    // std::max

#include "./chunk.h"
#include "./flatteningconverter.h"
#include "./blockidentifier.h"

template<typename ValueT>
inline void* safeMemCpy(void* dest, const std::vector<ValueT>& srcVec, size_t length)
{
  const size_t src_data_size = (sizeof(ValueT) * srcVec.size());
  if (length > src_data_size) {
    #if defined(DEBUG) || defined(_DEBUG) || defined(QT_DEBUG)
      qWarning() << "Copy too much data!";
    #endif
    length = src_data_size; // this happens sometimes and I guess its then actually a bug in the load() implementation. But this way it at least doesn't crash randomly.
  }

  return memcpy(dest, &srcVec[0], length);
}

Chunk::Chunk()
  : version(0)
  , highest(0)
  , loaded(false)
  , rendering(false)
{}

Chunk::~Chunk() {
  if (loaded) {
    for (int i = 0; i < 16; i++)
      if (sections[i]) {
        if (!(sections[i]->blockPaletteIsShared) && (sections[i]->blockPaletteLength > 0)) {
          delete[] sections[i]->blockPalette;
        }
        sections[i]->blockPaletteLength = 0;
        sections[i]->blockPalette = NULL;

        delete sections[i];
        sections[i] = NULL;
      }
    loaded = false;
  }
}

void Chunk::findHighestBlock()
{
  for (int i = 15; i >= 0; i--) {
    if (this->sections[i]) {
      for (int j = 4095; j >= 0; j--) {
        if (this->sections[i]->blocks[j]) {
          highest = i * 16 + (j >> 8);
          return;
        }
      }
    }
  }
}

const Chunk::EntityMap &Chunk::getEntityMap() const {
  return entities;
}

const ChunkSection *Chunk::getSectionByY(int y) const {
  size_t section_idx = static_cast<size_t>(y) >> 4;
  if (section_idx >= sections.size())
    return nullptr;

  return sections[section_idx];
}

uint Chunk::getBlockHID(int x, int y, int z) const {
  const ChunkSection * const section = getSectionByY(y);
  if (!section) {
    return 0;
  }

  const PaletteEntry& pdata = section->getPaletteEntry(x, y, z);

  return pdata.hid;
}

int Chunk::getBiomeID(int x, int y, int z) const {
  int offset;

  if (this->version >= 2203) {
    // Minecraft 1.15 has Y dependand Biome
    int x_idx = x >> 2;
    int y_idx = y >> 2;
    int z_idx = z >> 2;
    offset = x_idx + 4*z_idx + 16*y_idx;
  } else {
    // Minecraft <1.15 has fixed Biome per collumn
    offset = x + 16*z;
  }

  #if defined(DEBUG) || defined(_DEBUG) || defined(QT_DEBUG)
  if ((offset < 0) || ((unsigned long long)(offset) > sizeof(this->biomes)/sizeof(this->biomes[0]))) {
    qWarning() << "Biome index out of range!";
    return 0;
  }
  #endif
  return this->biomes[offset];
}


//-------------------------------------------------------------------------------------------------
// this is where we load NBT data and parse it

void Chunk::load(const NBT &nbt) {
  renderedAt = -1;  // impossible.
  renderedFlags = 0;  // no flags
  for (int i = 0; i < 16; i++)
    this->sections[i] = NULL;

  if (nbt.has("DataVersion"))
    this->version = nbt.at("DataVersion")->toInt();
  const Tag * level = nbt.at("Level");
  chunkX = level->at("xPos")->toInt();
  chunkZ = level->at("zPos")->toInt();

  // load Biome data
  // Partially-generated chunks may have an empty Biomes tag.
  // Trying to extract the Biomes data in that case will cause a crash.
  if (level->has("Biomes") && level->at("Biomes") && level->at("Biomes")->length()) {
    const Tag * biomesTag = level->at("Biomes");
    if (typeid(*biomesTag) == typeid(Tag_Int_Array)) {
      // Biomes is Tag_Int_Array
      // -> format after "The Flattening"
      // raw copy Biome data
      const Tag_Int_Array * biomeData = dynamic_cast<const Tag_Int_Array*>(level->at("Biomes"));
      std::size_t len = std::min(sizeof(this->biomes), (sizeof(int)*biomeData->length()));
      safeMemCpy(this->biomes, biomeData->toIntArray(), len);
    } else if (typeid(*biomesTag) == typeid(Tag_Byte_Array)) {
      // Biomes is Tag_Byte_Array
      // -> old Biome format before "The Flattening"
      const Tag_Byte_Array * biomeData = dynamic_cast<const Tag_Byte_Array*>(level->at("Biomes"));
      // convert quint8 to quint32
      auto rawBiomes = biomeData->toByteArray();
      int len = std::min(256, biomeData->length());
      for (int i=0; i<len; i++) {
        this->biomes[i] = rawBiomes[i];
      }
    }
  } else {  // no Biome data present
    int len = sizeof(this->biomes) / sizeof(this->biomes[0]);
    for (int i=0; i<len; i++)
      this->biomes[i] = -1;
  }

  // load available Sections
  if (level->has("Sections")) {
    auto sections = level->at("Sections");
    int numSections = sections->length();
    // loop over all stored Sections, they are not guarantied to be ordered or consecutive
    for (int s = 0; s < numSections; s++) {
      const Tag * section = sections->at(s);
      int idx = section->at("Y")->toInt();
      // only sections 0..15 contain block data
      if ((idx >=0) && (idx <16)) {
        ChunkSection *cs = new ChunkSection();
        if (this->version >= 2836) {
          // after "Cliff & Caves" update (1.18)
          loadSection2800(cs, section);
        } else if (this->version >= 1519) {
          // after "The Flattening" update (1.13)
          loadSection1519(cs, section);
        } else {
          loadSection1343(cs, section);
        }

        this->sections[idx] = cs;
      }
    }
  }

  // parse Structures that start in this Chunk
  if (version >= 1519) {
    if (level->has("Structures")) {
      auto nbtListStructures = level->at("Structures");
      auto structurelist     = GeneratedStructure::tryParseChunk(nbtListStructures);
      for (auto it = structurelist.begin(); it != structurelist.end(); ++it) {
        emit structureFound(*it);
      }
    }
  }

  // parse Entities
  if (level->has("Entities")) {
    auto entitylist = level->at("Entities");
    int numEntities = entitylist->length();
    for (int i = 0; i < numEntities; ++i) {
      auto e = Entity::TryParse(entitylist->at(i));
      if (e)
        entities.insertMulti(e->type(), e);
    }
  }

  // check for the highest block in this chunk
  // todo: use highmap from stored NBT data
  findHighestBlock();

  loaded = true; // needs to be at the end!
}


void Chunk::loadEntities(const NBT &nbt) {
  // parse Entities in extra folder (1.17+)
  if (version >= 2681) {
    if (nbt.has("Entities")) {
      auto entitylist = nbt.at("Entities");
      int numEntities = entitylist->length();
      for (int i = 0; i < numEntities; ++i) {
        auto e = Entity::TryParse(entitylist->at(i));
        if (e)
          entities.insertMulti(e->type(), e);
      }
    }
  }
}


// supported DataVersions:
//    0 = 1.8 and below
//
//  169 = 1.9
//  175 = 1.9.1
//  176 = 1.9.2
//  183 = 1.9.3
//  184 = 1.9.4
//
//  510 = 1.10
//  511 = 1.10.1
//  512 = 1.10.2
//
//  819 = 1.11
//  921 = 1.11.1
//  922 = 1.11.2
//
// 1139 = 1.12
// 1241 = 1.12.1
// 1343 = 1.12.2
//
// 1519 = 1.13
// 1628 = 1.13.1
// 2203 = 1.15.19w36a
void Chunk::loadSection1343(ChunkSection *cs, const Tag *section) {
  // copy raw data
  quint8 blocks[4096];
  quint8 data[2048];
  safeMemCpy(blocks, section->at("Blocks")->toByteArray(), 4096);
  safeMemCpy(data,   section->at("Data")->toByteArray(),   2048);
  safeMemCpy(cs->blockLight, section->at("BlockLight")->toByteArray(), 2048);

  // convert old BlockID + data into virtual ID
  for (int i = 0; i < 4096; i++) {
    int d = data[i>>1];         // get raw data (two nibbles)
    if (i & 1) d >>= 4;         // get one nibble of data
    // Shift enough so virtual IDs never overlap 0-4095 range
    cs->blocks[i] = blocks[i] | ((d & 0x0f) << 12);
  }

  // parse optional "Add" part for higher block IDs in mod packs
  if (section->has("Add")) {
    auto raw = section->at("Add")->toByteArray();
    for (int i = 0; i < 2048; i++) {
      cs->blocks[i * 2] |= (raw[i] & 0xf) << 8;
      cs->blocks[i * 2 + 1] |= (raw[i] & 0xf0) << 4;
    }
  }

  // link to Converter palette
  cs->blockPaletteLength = FlatteningConverter::Instance().paletteLength;
  cs->blockPalette = FlatteningConverter::Instance().getPalette();
  cs->blockPaletteIsShared = true;
}


// Chunk format after "The Flattening" version 1519
void Chunk::loadSection1519(ChunkSection *cs, const Tag *section) {

  // decode Palette to be able to map BlockStates
  if (section->has("Palette")) {
    loadSection_decodeBlockPalette(cs, section->at("Palette"));
  } else loadSection_createDummyPalette(cs);  // create a dummy palette

  // map BlockStates to BlockData
  if (section->has("BlockStates")) {
    loadSection_loadBlockStates(cs, section->at("BlockStates"));
  } else {
    // set everything to 0 (minecraft:air)
    memset(cs->blocks, 0, sizeof(cs->blocks));
  }

  // copy Light data
//  if (section->has("SkyLight")) {
//    safeMemCpy(cs->skyLight, section->at("SkyLight")->toByteArray(), 2048);
//  }
  if (section->has("BlockLight")) {
    safeMemCpy(cs->blockLight, section->at("BlockLight")->toByteArray(), 2048);
  } else {
    memset(cs->blockLight, 0, sizeof(cs->blockLight));
  }
}


// Chunk format after "Clifs & Cave version 2800
void Chunk::loadSection2800(ChunkSection * cs, const Tag * section) {

  // decode Palette to be able to map BlockStates
  if (section->has("block_states") && section->at("block_states")->has("palette")) {
    loadSection_decodeBlockPalette(cs, section->at("block_states")->at("palette"));
  } else loadSection_createDummyPalette(cs);

  // map BlockStates to BlockData
  if (section->has("block_states") && section->at("block_states")->has("data")) {
    loadSection_loadBlockStates(cs, section->at("block_states")->at("data"));
  } else {
    // set everything to 0 (minecraft:air)
    memset(cs->blocks, 0, sizeof(cs->blocks));
  }

  // copy Light data
//  if (section->has("SkyLight")) {
//    safeMemCpy(cs->skyLight, section->at("SkyLight")->toByteArray(), 2048);
//  }
  if (section->has("BlockLight")) {
    safeMemCpy(cs->blockLight, section->at("BlockLight")->toByteArray(), 2048);
  } else {
    memset(cs->blockLight, 0, sizeof(cs->blockLight));
  }
}


void Chunk::loadSection_decodeBlockPalette(ChunkSection * cs, const Tag * paletteTag) {
  BlockIdentifier &bi = BlockIdentifier::Instance();

  cs->blockPaletteLength = paletteTag->length();
  cs->blockPaletteIsShared = false;
  cs->blockPalette = new PaletteEntry[cs->blockPaletteLength];
  for (int j = 0; j < paletteTag->length(); j++) {
    // get name and hash it to hid
    cs->blockPalette[j].name = paletteTag->at(j)->at("Name")->toString();
    uint hid  = qHash(cs->blockPalette[j].name);
    // copy all other properties
    if (paletteTag->at(j)->has("Properties"))
    cs->blockPalette[j].properties = paletteTag->at(j)->at("Properties")->getData().toMap();

    // check vor variants
    BlockInfo const & block = bi.getBlockInfo(hid);
    if (block.hasVariants()) {
    // test all available properties
    for (auto key : cs->blockPalette[j].properties.keys()) {
      QString vname = cs->blockPalette[j].name + ":" + key + ":" + cs->blockPalette[j].properties[key].toString();
      uint vhid = qHash(vname);
      if (bi.hasBlockInfo(vhid))
        hid = vhid; // use this vaiant instead
      }
    }
    // store hash of found variant
    cs->blockPalette[j].hid  = hid;
  }
}


void Chunk::loadSection_createDummyPalette(ChunkSection *cs) {
  // create a dummy palette
  cs->blockPalette = new PaletteEntry[1];
  cs->blockPalette[0].name = "minecraft:air";
  cs->blockPalette[0].hid  = 0;
}


void Chunk::loadSection_loadBlockStates(ChunkSection *cs, const Tag * blockStateTag) {

  auto blockStates = blockStateTag->toLongArray();
  int bsCnt  = 0;  // counter for 64bit words
  int bitCnt = 0;  // counter for bits

  if (this->version < 2529) {
    // "compact BlockStates" just the first time after "The Flattening"
    for (int i = 0; i < 4096; i++) {
      int bitSize = (blockStateTag->length())*64/4096;
      int bitMask = (1 << bitSize)-1;
      if (bitCnt+bitSize <= 64) {
        // bits fit into current word
        uint64_t blockState = blockStates[bsCnt];
        cs->blocks[i] = (blockState >> bitCnt) & bitMask;
        bitCnt += bitSize;
        if (bitCnt == 64) {
          bitCnt = 0;
          bsCnt++;
        }
      } else {
        // bits are spread accross two words
        uint64_t blockState1 = blockStates[bsCnt++];
        uint64_t blockState2 = blockStates[bsCnt];
        uint32_t block = (blockState1 >> bitCnt) & bitMask;
        bitCnt += bitSize;
        bitCnt -= 64;
        block += (blockState2 << (bitSize - bitCnt)) & bitMask;
        cs->blocks[i] = block;
      }
    }
  } else {
    // "optimized for loading" BlockStates since 1.16.20w17a
    int bitSize = std::max(4, int(ceil(log2(cs->blockPaletteLength))));
    int bitMask = (1 << bitSize)-1;
    for (int i = 0; i < 4096; i++) {
      uint64_t blockState = blockStates[bsCnt];
      cs->blocks[i] = (blockState >> bitCnt) & bitMask;
      bitCnt += bitSize;
      if (bitCnt+bitSize > 64) {
        bsCnt++;
        bitCnt = 0;
      }
    }
  }

}


//-------------------------------------------------------------------------------------------------
// ChunkSection

ChunkSection::ChunkSection()
  : blockPalette(NULL)
  , blockPaletteLength(0)
  , blockPaletteIsShared(false)  // only the "old" converted format is using one shared palette
{}

const PaletteEntry & ChunkSection::getPaletteEntry(int x, int y, int z) const {
  int xoffset = (x & 0x0f);
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  return getPaletteEntry(xoffset + yoffset + zoffset);
}

const PaletteEntry & ChunkSection::getPaletteEntry(int offset, int y) const {
  int yoffset = (y & 0x0f) << 8;
  return getPaletteEntry(offset + yoffset);
}

inline
const PaletteEntry & ChunkSection::getPaletteEntry(int offset) const {
  quint16 blockid = blocks[offset];
  if (blockid < blockPaletteLength)
    return blockPalette[blockid];
  else
    return blockPalette[0];
}

quint8 ChunkSection::getBiome(int x, int y, int z) const {
  int xoffset = x;
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  return getBlockLight(xoffset + yoffset + zoffset);
}

quint8 ChunkSection::getBiome(int offset, int y) const {
  int yoffset = (y & 0x0f) << 8;
  return getBlockLight(offset + yoffset);
}

inline
quint8 ChunkSection::getBiome(int offset) const {
  return blockLight[offset];
}

//quint8 ChunkSection::getSkyLight(int x, int y, int z) {
//  int xoffset = x;
//  int yoffset = (y & 0x0f) << 8;
//  int zoffset = z << 4;
//  int value = skyLight[(xoffset + yoffset + zoffset) / 2];
//  if (x & 1) value >>= 4;
//  return value & 0x0f;
//}

//quint8 ChunkSection::getSkyLight(int offset, int y) {
//  int yoffset = (y & 0x0f) << 8;
//  int value = skyLight[(offset + yoffset) / 2];
//  if (offset & 1) value >>= 4;
//  return value & 0x0f;
//}

quint8 ChunkSection::getBlockLight(int x, int y, int z) const {
  int xoffset = x;
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  return getBlockLight(xoffset + yoffset + zoffset);
}

quint8 ChunkSection::getBlockLight(int offset, int y) const {
  int yoffset = (y & 0x0f) << 8;
  return getBlockLight(offset + yoffset);
}

inline
quint8 ChunkSection::getBlockLight(int offset) const {
  int value = blockLight[offset / 2];
  if (offset & 1) value >>= 4;
  return value & 0x0f;
}
