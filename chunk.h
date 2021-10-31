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
  ChunkSection();

  const PaletteEntry & getPaletteEntry(int x, int y, int z) const;
  const PaletteEntry & getPaletteEntry(int offset, int y) const;
  const PaletteEntry & getPaletteEntry(int offset) const;
  quint8 getBiome(int x, int y, int z) const;
  quint8 getBiome(int offset, int y) const;
  quint8 getBiome(int offset) const;
  //quint8 getSkyLight(int x, int y, int z);
  //quint8 getSkyLight(int offset, int y);
  //quint8 getSkyLight(int offset);
  quint8 getBlockLight(int x, int y, int z) const;
  quint8 getBlockLight(int offset, int y) const;
  quint8 getBlockLight(int offset) const;

  PaletteEntry *blockPalette;
  int        blockPaletteLength;
  bool       blockPaletteIsShared;

  quint16 blocks[16*16*16];       // index into blockPalette for each Block
  quint8  biomes[4*4*4];          // key into BiomeIdentifer for each 4x4x4 volume of Blocks defining the Biome
//quint8  skyLight[16*16*16/2];   // not needed in Minutor
  quint8  blockLight[16*16*16/2]; // light value for each Block
};


class Chunk : public QObject {
  Q_OBJECT

 public:
  Chunk();
  ~Chunk();
  void load(const NBT &nbt);
  void loadEntities(const NBT &nbt);

  // public getters to read-only access internal data
  int getChunkX() const { return chunkX; }
  int getChunkZ() const { return chunkZ; }
  const uchar * getImage() const { return image; }

  const ChunkSection* getSectionByY(int y) const;
  const ChunkSection* getSectionByIdx(qint8 y) const;

  uint   getBlockHID(int x, int y, int z) const;
  qint32 getBiomeID(int x, int y, int z) const;

  typedef QMap<QString, QSharedPointer<OverlayItem>> EntityMap;
  const EntityMap& getEntityMap() const;

signals:
  void structureFound(QSharedPointer<GeneratedStructure> structure);

 protected:
  bool loadSection1343(ChunkSection * cs, const Tag * section);
  bool loadSection1519(ChunkSection * cs, const Tag * section);
  bool loadSection2844(ChunkSection * cs, const Tag * section);

  int  chunkX;
  int  chunkZ;
  int  version;
  int  highest;
  int  lowest;
  int  renderedAt;
  int  renderedFlags;
  bool loaded;
  bool rendering;

  QMap<qint8, ChunkSection*> sections;
  qint32 biomes[16 * 16 * 4];
  uchar  image[16 * 16 * 4];  // cached render: RGBA for 16*16 Blocks
  short  depth[16 * 16];      // cached depth map to create shadow
  EntityMap entities;
  friend class MapView;
  friend class ChunkRenderer;
  friend class ChunkCache;

private:
  void findHighestBlock();
  void loadLevelTag(const Tag * levelTag);  // nested structure with Level tag (up to 1.17)
  void loadCliffsCaves(const NBT &nbt);     // flat structure without Level tag (1.18+)
  void loadSection_decodeBlockPalette(ChunkSection * cs, const Tag * paletteTag);
  void loadSection_createDummyPalette(ChunkSection * cs);
  void loadSection_loadBlockStates(ChunkSection *cs, const Tag * blockStateTag);
  bool loadSection_decodeBiomePalette(ChunkSection * cs, const Tag * biomesTag);
};

#endif  // CHUNK_H_
