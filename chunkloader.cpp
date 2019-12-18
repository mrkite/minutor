/** Copyright (c) 2013, Sean Kasun */

#include "./chunkloader.h"
#include "./chunkcache.h"
#include "./chunk.h"


ChunkLoader::ChunkLoader(QString path, int cx, int cz)
  : path(path)
  , cx(cx), cz(cz)
  , cache(ChunkCache::Instance())
{}

ChunkLoader::~ChunkLoader()
{}

void ChunkLoader::run() {
  // get existing Chunk entry from Cache
  QSharedPointer<Chunk> chunk(cache.fetchCached(cx, cz));
  // load & parse NBT data
  loadNbt(path, cx, cz, chunk);
  emit loaded(cx, cz);
}

bool ChunkLoader::loadNbt(QString path, int cx, int cz, QSharedPointer<Chunk> chunk)
{
  // check if chunk is a valid storage
  if (!chunk) {
    return false;
  }

  // get coordinates of region file
  int rx = cx >> 5;
  int rz = cz >> 5;

  QFile f(path + "/region/r." + QString::number(rx) + "." + QString::number(rz) + ".mca");
  if (!f.open(QIODevice::ReadOnly)) {
    // no chunks in this region (region file not present at all)
    return false;
  }

  const int headerSize = 4096;

  if (f.size() < headerSize) {
    // file header not yet fully written by minecraft
    return false;
  }

  // map header into memory
  uchar *header = f.map(0, headerSize);
  int offset = 4 * ((cx & 31) + (cz & 31) * 32);

  int coffset = (header[offset] << 16) | (header[offset + 1] << 8) | header[offset + 2];
  int numSectors = header[offset+3];
  f.unmap(header);

  if (coffset == 0) {
    // no Chunk information stored in region file
    f.close();
    return false;
  }

  const int chunkStart = coffset * 4096;
  const int chunkSize = numSectors * 4096;

  if (f.size() < (chunkStart + chunkSize)) {
    // Chunk not yet fully written by Minecraft
    return false;
  }

  uchar *raw = f.map(chunkStart, chunkSize);
  if (raw == NULL) {
    f.close();
    return false;
  }
  // parse Chunk data
  // Chunk will be flagged "loaded" in a thread save way
  NBT nbt(raw);
  chunk->load(nbt);
  f.unmap(raw);
  f.close();

  // if we reach this point, everything went well
  return true;
}
