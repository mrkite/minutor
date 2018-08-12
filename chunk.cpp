/** Copyright (c) 2013, Sean Kasun */

#include <algorithm>

#include "./chunk.h"
#include "./flatteningconverter.h"

quint16 getBits(const unsigned char *data, int pos, int n) {
  quint16 result = 0;
  int arrIndex = pos/8;
  int bitIndex = pos%8;
  quint32 loc =
    data[arrIndex]   << 24 |
    data[arrIndex+1] << 16 |
    data[arrIndex+2] << 8  |
    data[arrIndex+3];

  return ((loc >> (32-bitIndex-n)) & ((1 << n) -1));
}



Chunk::Chunk() {
  loaded = false;
}

void Chunk::load(const NBT &nbt) {
  renderedAt = -1;  // impossible.
  renderedFlags = 0;  // no flags
  memset(this->biomes, 127, 256);  // init to unknown biome
  for (int i = 0; i < 16; i++)
    this->sections[i] = NULL;
  highest = 0;

  int version = 0;
  if (nbt.has("DataVersion"))
    version = nbt.at("DataVersion")->toInt();
  const Tag * level = nbt.at("Level");
  chunkX = level->at("xPos")->toInt();
  chunkZ = level->at("zPos")->toInt();

  // load Biome per column
  auto biomes = level->at("Biomes");
  if (version >= 1519) {
    memcpy(this->biomes, biomes->toIntArray(), sizeof(int)*biomes->length());
  } else {
    // convert quint8 to quint32
    auto rawBiomes = biomes->toByteArray();
    for (int i=0; i<256; i++)
      this->biomes[i] = rawBiomes[i];
  }

  // load available Sections
  auto sections = level->at("Sections");
  int numSections = sections->length();
  // loop over all stored Sections, they are not guarantied to be ordered or consecutive
  for (int s = 0; s < numSections; s++) {
    ChunkSection *cs = new ChunkSection();
    const Tag * section = sections->at(s);
    if (version >= 1519)
      loadSection1519(cs, section);
    else
      loadSection1343(cs, section);

    int idx = section->at("Y")->toInt();
    this->sections[idx] = cs;
  }

  loaded = true;

  auto entitylist = level->at("Entities");
  int numEntities = entitylist->length();
  for (int i = 0; i < numEntities; ++i) {
    auto e = Entity::TryParse(entitylist->at(i));
    if (e)
      entities.insertMulti(e->type(), e);
  }

  // check for the highest block in this chunk
  // todo: use highmap from stored NBT data
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
void Chunk::loadSection1343(ChunkSection *cs, const Tag *section) {
  // copy raw data
  quint8 blocks[4096];
  quint8 data[2048];
  memcpy(blocks, section->at("Blocks")->toByteArray(), 4096);
  memcpy(data,   section->at("Data")->toByteArray(),   2048);
  memcpy(cs->blockLight, section->at("BlockLight")->toByteArray(), 2048);

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

// Cunk format afer "The Flattening" version 1509
void Chunk::loadSection1519(ChunkSection *cs, const Tag *section) {
  // decode Palette to be able to map BlockStates
  auto rawPalette = section->at("Palette");
  cs->paletteLength = rawPalette->length();
  cs->palette = new BlockData[cs->paletteLength];
  for (int j = 0; j < rawPalette->length(); j++) {
    cs->palette[j].name = rawPalette->at(j)->at("Name")->toString();
    cs->palette[j].hid  = qHash(cs->palette[j].name);
    if (rawPalette->at(j)->has("Properties"))
      cs->palette[j].properties = rawPalette->at(j)->at("Properties")->getData().toMap();
  }
  // map BlockStates to BlockData
  // todo: bit fidling looks very complicated -> find easier code
  auto raw = section->at("BlockStates")->toLongArray();
  int blockStatesLength = section->at("BlockStates")->length();
  unsigned char *byteData = new unsigned char[8*blockStatesLength];
  memcpy(byteData, raw, 8*blockStatesLength);
  std::reverse(byteData, byteData+(8*blockStatesLength));
  int bitSize = (blockStatesLength)*64/4096;
  for (int i = 0; i < 4096; i++) {
    cs->blocks[4095-i] = getBits(byteData, i*bitSize, bitSize);
  }
  delete byteData;
  // copy Light data (todo: Skylight is not needed)
  memcpy(cs->skyLight, section->at("SkyLight")->toByteArray(), 2048);
  memcpy(cs->blockLight, section->at("BlockLight")->toByteArray(), 2048);
}

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


const BlockData & ChunkSection::getBlockData(int x, int y, int z) {
  int xoffset = x;
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  return palette[blocks[xoffset + yoffset + zoffset]];
}

const BlockData & ChunkSection::getBlockData(int offset, int y) {
  int yoffset = (y & 0x0f) << 8;
  return palette[blocks[offset + yoffset]];
}

quint8 ChunkSection::getSkyLight(int x, int y, int z) {
  int xoffset = x;
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  int value = skyLight[(xoffset + yoffset + zoffset) / 2];
  if (x & 1) value >>= 4;
  return value & 0x0f;
}

quint8 ChunkSection::getSkyLight(int offset, int y) {
  int yoffset = (y & 0x0f) << 8;
  int value = skyLight[(offset + yoffset) / 2];
  if (offset & 1) value >>= 4;
  return value & 0x0f;
}

quint8 ChunkSection::getBlockLight(int x, int y, int z) {
  int xoffset = x;
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  int value = blockLight[(xoffset + yoffset + zoffset) / 2];
  if (x & 1) value >>= 4;
  return value & 0x0f;
}

quint8 ChunkSection::getBlockLight(int offset, int y) {
  int yoffset = (y & 0x0f) << 8;
  int value = blockLight[(offset + yoffset) / 2];
  if (offset & 1) value >>= 4;
  return value & 0x0f;
}
