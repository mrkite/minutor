/** Copyright (c) 2013, Sean Kasun */
#ifndef CHUNK_H_
#define CHUNK_H_

#include <QtCore>
#include <QVector>

#include "./nbt.h"
#include "./entity.h"
class BlockIdentifier;

class ChunkSection {
 public:
  quint16 getBlock(int x, int y, int z);
  quint8  getData(int x, int y, int z);
  quint8  getLight(int x, int y, int z);

  quint16 blocks[4096];
  quint8  data[2048];
  quint8  light[2048];
};

class Chunk {
 public:
  Chunk();
  void load(const NBT &nbt);
  ~Chunk();
 protected:
  typedef QMap<QString, QSharedPointer<OverlayItem>> EntityMap;

  quint8 biomes[256];
  int highest;
  ChunkSection *sections[16];
  int renderedAt;
  int renderedFlags;
  bool loaded;
  uchar image[16 * 16 * 4];  // cached render
  uchar depth[16 * 16];
  EntityMap entities;
  int chunkX;
  int chunkZ;
  friend class MapView;
  friend class ChunkCache;
  friend class WorldSave;
};

#endif  // CHUNK_H_
