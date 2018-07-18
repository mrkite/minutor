/** Copyright (c) 2013, Sean Kasun */

#include "./chunk.h"
#include <algorithm>

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

  auto level = nbt.at("Level");
  chunkX = level->at("xPos")->toInt();
  chunkZ = level->at("zPos")->toInt();

  auto biomes = level->at("Biomes");
  memcpy(this->biomes, biomes->toIntArray(), 4*biomes->length());
  auto sections = level->at("Sections");
  int numSections = sections->length();
  for (int i = 0; i < numSections; i++) {
    auto section = sections->at(i);
    auto cs = new ChunkSection();
    auto rawPalette = section->at("Palette");
    cs->paletteLength = rawPalette->length();
    cs->palette = new BlockData[cs->paletteLength];
    for (int j = 0; j < rawPalette->length(); j++) {
      cs->palette[j].name = rawPalette->at(j)->at("Name")->toString();
      if (rawPalette->at(j)->has("Properties"))
        cs->palette[j].properties = rawPalette->at(j)->at("Properties")->getData().toMap();
    }
    auto raw = section->at("BlockStates")->toLongArray();
    int blockStatesLength = section->at("BlockStates")->length();
    unsigned char *byteData = new unsigned char[8*blockStatesLength];
    memcpy(byteData, raw, 8*blockStatesLength);
    std::reverse(byteData, byteData+(8*blockStatesLength));
    int bitSize = (blockStatesLength)*64/4096;
    for (int i = 0; i < 4096; i++) {
      cs->blocks[4095-i] = getBits(byteData, i*bitSize, bitSize);
    }
    free(byteData);
    memcpy(cs->skyLight, section->at("SkyLight")->toByteArray(), 2048);
    memcpy(cs->blockLight, section->at("BlockLight")->toByteArray(), 2048);
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

  for (int i = 15; i >= 0; i--) {  // check for the highest block in this chunk
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

Chunk::~Chunk() {
  if (loaded) {
    for (int i = 0; i < 16; i++)
      if (sections[i]) {
        delete[] sections[i]->palette;
        delete sections[i];
        sections[i] = NULL;
      }
  }
}


QString ChunkSection::getBlock(int x, int y, int z) {
  int xoffset = x;
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  return palette[blocks[xoffset + yoffset + zoffset]].name;
}

QString ChunkSection::getBlock(int offset, int y) {
  int yoffset = (y & 0x0f) << 8;
  return palette[blocks[offset + yoffset]].name;
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
