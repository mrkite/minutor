/** Copyright (c) 2013, Sean Kasun */
#ifndef CHUNK_H_
#define CHUNK_H_

#include <QtCore>
#include <QVector>
#include <array>

#include "nbt/nbt.h"
#include "overlay/entity.h"
#include "overlay/generatedstructure.h"
#include "paletteentry.h"


class ChunkSection {
 public:
  ChunkSection();
  ~ChunkSection();

  const PaletteEntry & getPaletteEntry(int x, int y, int z) const;
  const PaletteEntry & getPaletteEntry(int offset, int y) const;
  const PaletteEntry & getPaletteEntry(int offset) const;
  quint16 getBiome(int x, int y, int z) const;
  quint16 getBiome(int offset, int y) const;
  quint16 getBiome(int offset) const;
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
  quint16 biomes[4*4*4];          // key into BiomeIdentifer for each 4x4x4 volume of Blocks defining the Biome
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
  int  getHighest() const { return highest; }
  int  getLowest() const  { return lowest; }

  const ChunkSection* getSectionByY(int y) const;
  const ChunkSection* getSectionByIdx(qint8 y) const;

  uint   getBlockHID(int x, int y, int z) const;
  qint32 getBiomeID(int x, int y, int z) const;

  typedef QMultiMap<QString, QSharedPointer<OverlayItem>> EntityMap;
  const EntityMap& getEntityMap() const;

  /** Returns whether the chunk is locked by the ChunkLock resourcepack. */
  bool getIsChunkLocked() const { return isChunkLocked; }

  /** Returns the name of the item needed for unlocking the ChunkLock.
  Only valid if getIsChunkLocked() returns true. */
  const QString & getChunkLockItemName() const { return chunkLockItemName; }

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
  long long inhabitedTime;

  QMap<qint8, ChunkSection*> sections;
  qint32 biomes[16 * 16 * 4]; // before "The Flattining" it was 1*16*16*Bytes, then it got 16*4*4*4*Int before it moved into Sections
  uchar  image[16 * 16 * 4];  // cached render: RGBA for 16*16 Blocks
  short  depth[16 * 16];      // cached depth map to create shadow
  EntityMap entities;

  // ChunkLocked feature:
  bool    isChunkLocked;      // flag specifies whether the chunk is locked by the ChunkLock resourcepack
  QString chunkLockItemName;  // the name of the item needed for unlocking the chunk

  // HID used for minecraft:air
  static const unsigned int air_hid;

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

  /** Checks whether the specified entity NBT is relevant to ChunkLock; if so, updates the ChunkLock-related state. */
  void loadCheckEntityChunkLock(const Tag * entityNbt);
};

#endif  // CHUNK_H_
