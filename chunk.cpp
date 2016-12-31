/** Copyright (c) 2013, Sean Kasun */

#include "./chunk.h"

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
  memcpy(this->biomes, biomes->toByteArray(), biomes->length());
  auto sections = level->at("Sections");
  int numSections = sections->length();
  for (int i = 0; i < numSections; i++) {
    auto section = sections->at(i);
    auto cs = new ChunkSection();
    auto raw = section->at("Blocks")->toByteArray();
    for (int i = 0; i < 4096; i++)
      cs->blocks[i] = raw[i];
    if (section->has("Add")) {
      raw = section->at("Add")->toByteArray();
      for (int i = 0; i < 2048; i++) {
        cs->blocks[i * 2] |= (raw[i] & 0xf) << 8;
        cs->blocks[i * 2 + 1] |= (raw[i] & 0xf0) << 4;
      }
    }
    memcpy(cs->data, section->at("Data")->toByteArray(), 2048);
    memcpy(cs->light, section->at("BlockLight")->toByteArray(), 2048);
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
        delete sections[i];
        sections[i] = NULL;
      }
  }
}


quint16 ChunkSection::getBlock(int x, int y, int z) {
  int xoffset = x;
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  return blocks[xoffset + yoffset + zoffset];
}

quint16 ChunkSection::getBlock(int offset, int y) {
  int yoffset = (y & 0x0f) << 8;
  return blocks[offset + yoffset];
}

quint8 ChunkSection::getData(int x, int y, int z) {
  int xoffset = x;
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  int value = data[(xoffset + yoffset + zoffset) / 2];
  if (x & 1) value >>= 4;
  return value & 0x0f;
}

quint8 ChunkSection::getData(int offset, int y) {
  int yoffset = (y & 0x0f) << 8;
  int value = data[(offset + yoffset) / 2];
  if (offset & 1) value >>= 4;
  return value & 0x0f;
}

quint8 ChunkSection::getLight(int x, int y, int z) {
  int xoffset = x;
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  int value = light[(xoffset + yoffset + zoffset) / 2];
  if (x & 1) value >>= 4;
  return value & 0x0f;
}

quint8 ChunkSection::getLight(int offset, int y) {
  int yoffset = (y & 0x0f) << 8;
  int value = light[(offset + yoffset) / 2];
  if (offset & 1) value >>= 4;
  return value & 0x0f;
}
