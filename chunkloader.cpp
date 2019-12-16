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
  loadNbt();
  emit loaded(cx, cz);
}

void ChunkLoader::loadNbt()
{
  // get coordinates of Region file
  int rx = cx >> 5;
  int rz = cz >> 5;

  QFile f(path + "/region/r." + QString::number(rx) + "." +
          QString::number(rz) + ".mca");
  if (!f.open(QIODevice::ReadOnly)) {  // no chunks in this region
    return;
  }

  const int headerSize = 4096;

  if (f.size() < headerSize) {
    return; // file header not yet fully written by minecraft
  }

  // map header into memory
  uchar *header = f.map(0, headerSize);
  int offset = 4 * ((cx & 31) + (cz & 31) * 32);

  int coffset = (header[offset] << 16) | (header[offset + 1] << 8) |
      header[offset + 2];
  int numSectors = header[offset+3];
  f.unmap(header);

  if (coffset == 0) {  // no chunk
    f.close();
    return;
  }

  const int chunkStart = coffset * 4096;
  const int chunkSize = numSectors * 4096;

  if (f.size() < (chunkStart + chunkSize)) {
    return; // chunk not yet fully written by minecraft
  }

  uchar *raw = f.map(chunkStart, chunkSize);
  if (raw == NULL) {
    f.close();
    return;
  }
  // get existing Chunk entry from Cache
  QSharedPointer<Chunk> chunk(cache.fetchCached(cx, cz));
  // parse Chunk data
  // Chunk will be flagged "loaded" in a thread save way
  if (chunk) {
    NBT nbt(raw);
    chunk->load(nbt);
  }
  f.unmap(raw);
  f.close();
}
