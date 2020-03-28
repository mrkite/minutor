/** Copyright (c) 2013, Sean Kasun */
#ifndef CHUNK_H_
#define CHUNK_H_

#include <QtCore>
#include <QVector>

#include "./nbt.h"
#include "./entity.h"
#include "./paletteentry.h"
#include "./generatedstructure.h"

#include <array>

class ChunkSection {
 public:
  const PaletteEntry & getPaletteEntry(int x, int y, int z) const;
  const PaletteEntry & getPaletteEntry(int offset, int y) const;
  quint8 getSkyLight(int x, int y, int z);
  quint8 getSkyLight(int offset, int y);
  quint8 getBlockLight(int x, int y, int z) const;
  quint8 getBlockLight(int offset, int y) const;

  PaletteEntry *palette;
  int        paletteLength;

  quint16 blocks[16*16*16];
//quint8  skyLight[16*16*16/2];   // not needed in Minutor
  quint8  blockLight[16*16*16/2];
};


class Chunk : public QObject {
  Q_OBJECT

 public:
  Chunk();
  ~Chunk();
  void load(const NBT &nbt);

  typedef QMap<QString, QSharedPointer<OverlayItem>> EntityMap;

  qint32 get_biome(int x, int z);
  qint32 get_biome(int x, int y, int z);
  qint32 get_biome(int offset);

  const EntityMap& getEntityMap() const;

  const ChunkSection* getSectionByY(int y) const;

  uint getBlockHid(int x, int y, int z) const;

  int getChunkX() const { return chunkX; }
  int getChunkZ() const { return chunkZ; }

signals:
  void structureFound(QSharedPointer<GeneratedStructure> structure);

 protected:
  void loadSection1343(ChunkSection *cs, const Tag *section);
  void loadSection1519(ChunkSection *cs, const Tag *section);


  int version;
  qint32 biomes[16 * 16 * 4];
  int highest;
  std::array<ChunkSection*, 16> sections;
  int renderedAt;
  int renderedFlags;
  bool loaded;
  bool rendering;
  uchar image[16 * 16 * 4];  // cached render: RGBA for 16*16 Blocks
  uchar depth[16 * 16];
  EntityMap entities;
  int chunkX;
  int chunkZ;
  friend class MapView;
  friend class ChunkRenderer;
  friend class ChunkCache;
  friend class WorldSave;
};

#endif  // CHUNK_H_
