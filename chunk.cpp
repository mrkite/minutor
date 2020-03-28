/** Copyright (c) 2013, Sean Kasun */

#include <algorithm>

#include "./chunk.h"
#include "./flatteningconverter.h"
#include "./blockidentifier.h"

template<typename _ValueT>
inline void* safeMemCpy(void* __dest, const std::vector<_ValueT>& __src, size_t __len)
{
  const size_t src_data_size = (sizeof(_ValueT) * __src.size());
    if (__len > src_data_size)
    {
      __len = src_data_size; // this happens sometimes and I guess its then actually a bug in the load() implementation. But this way it at least doesn't crash randomly.
    }

    return memcpy(__dest, &__src[0], __len);
}


quint16 getBits(const unsigned char *data, int pos, int n) {
//  quint16 result = 0;
  int arrIndex = pos/8;
  int bitIndex = pos%8;
  quint32 loc =
    data[arrIndex]   << 24 |
    data[arrIndex+1] << 16 |
    data[arrIndex+2] << 8  |
    data[arrIndex+3];

  return ((loc >> (32-bitIndex-n)) & ((1 << n) -1));
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
        if (sections[i]->paletteLength > 0) {
          delete[] sections[i]->palette;
        }
        sections[i]->paletteLength = 0;
        sections[i]->palette = NULL;

        delete sections[i];
        sections[i] = NULL;
      }
  }
}


int Chunk::get_biome(int x, int z) {
  return get_biome(x, 64, z);
}

int Chunk::get_biome(int x, int y, int z) {
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

  return get_biome(offset);
}

int Chunk::get_biome(int offset) {
  return this->biomes[offset];
}



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
  if (level->has("Biomes")) {
    const Tag_Int_Array * biomes = dynamic_cast<const Tag_Int_Array*>(level->at("Biomes"));
    if (biomes) {  // Biomes is a Tag_Int_Array
      if ((this->version >= 2203)) {
        // raw copy Biome data
        safeMemCpy(this->biomes, biomes->toIntArray(), sizeof(int)*1024);
      } else if ((this->version >= 1519)) {
        // raw copy Biome data
        safeMemCpy(this->biomes, biomes->toIntArray(), sizeof(int)*256);
      }
    } else {  // Biomes is not a Tag_Int_Array
      const Tag_Byte_Array * biomes = dynamic_cast<const Tag_Byte_Array*>(level->at("Biomes"));
      // convert quint8 to quint32
      auto rawBiomes = biomes->toByteArray();
      for (int i=0; i<256; i++) {
        this->biomes[i] = rawBiomes[i];
      }
    }
  } else {  // no Biome data present
    for (int i=0; i<16*16*4; i++)
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
        if (this->version >= 1519) {
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

uint Chunk::getBlockHid(int x, int y, int z) const {
  const ChunkSection * const section = getSectionByY(y);
  if (!section) {
    return 0;
  }

  const PaletteEntry& pdata = section->getPaletteEntry(x, y, z);

  return pdata.hid;
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
    cs->blocks[i] = blocks[i] | ((d & 0x0f) << 8);
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
  cs->paletteLength = 0;
  cs->palette = FlatteningConverter::Instance().getPalette();
}

// Chunk format after "The Flattening" version 1509
void Chunk::loadSection1519(ChunkSection *cs, const Tag *section) {
  BlockIdentifier &bi = BlockIdentifier::Instance();
  // decode Palette to be able to map BlockStates
  if (section->has("Palette")) {
    auto rawPalette = section->at("Palette");
    cs->paletteLength = rawPalette->length();
    cs->palette = new PaletteEntry[cs->paletteLength];
    for (int j = 0; j < rawPalette->length(); j++) {
      // get name and hash it to hid
      cs->palette[j].name = rawPalette->at(j)->at("Name")->toString();
      uint hid  = qHash(cs->palette[j].name);
      // copy all other properties
      if (rawPalette->at(j)->has("Properties"))
      cs->palette[j].properties = rawPalette->at(j)->at("Properties")->getData().toMap();

      // check vor variants
      BlockInfo const & block = bi.getBlockInfo(hid);
      if (block.hasVariants()) {
      // test all available properties
      for (auto key : cs->palette[j].properties.keys()) {
        QString vname = cs->palette[j].name + ":" + key + ":" + cs->palette[j].properties[key].toString();
        uint vhid = qHash(vname);
        if (bi.hasBlockInfo(vhid))
          hid = vhid; // use this vaiant instead
        }
      }
      // store hash of found variant
      cs->palette[j].hid  = hid;
    }
  } else {
    // create a dummy palette
    cs->palette = new PaletteEntry[1];
    cs->palette[0].name = "minecraft:air";
    cs->palette[0].hid  = 0;
  }

  // map BlockStates to BlockData
  // todo: bit fidling looks very complicated -> find easier code
  if (section->has("BlockStates")) {
    auto raw = section->at("BlockStates")->toLongArray();
    int blockStatesLength = section->at("BlockStates")->length();
    unsigned char *byteData = new unsigned char[8*blockStatesLength];
    safeMemCpy(byteData, raw, 8*blockStatesLength);
    std::reverse(byteData, byteData+(8*blockStatesLength));
    int bitSize = (blockStatesLength)*64/4096;
    for (int i = 0; i < 4096; i++) {
      cs->blocks[4095-i] = getBits(byteData, i*bitSize, bitSize);
    }
    delete[] byteData;
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
  }
}

const PaletteEntry & ChunkSection::getPaletteEntry(int x, int y, int z) const {
  int xoffset = (x & 0x0f);
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  return palette[blocks[xoffset + yoffset + zoffset]];
}

const PaletteEntry & ChunkSection::getPaletteEntry(int offset, int y) const {
  int yoffset = (y & 0x0f) << 8;
  return palette[blocks[offset + yoffset]];
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
  int value = blockLight[(xoffset + yoffset + zoffset) / 2];
  if (x & 1) value >>= 4;
  return value & 0x0f;
}

quint8 ChunkSection::getBlockLight(int offset, int y) const {
  int yoffset = (y & 0x0f) << 8;
  int value = blockLight[(offset + yoffset) / 2];
  if (offset & 1) value >>= 4;
  return value & 0x0f;
}
