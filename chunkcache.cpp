/** Copyright (c) 2013, Sean Kasun */

#include "./chunkcache.h"
#include "./chunkloader.h"

#if defined(__unix__) || defined(__unix) || defined(unix)
#include <unistd.h>
#elif defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif

ChunkID::ChunkID(int cx, int cz) : cx(cx), cz(cz) {
}
bool ChunkID::operator==(const ChunkID &other) const {
  return (other.cx == cx) && (other.cz == cz);
}
uint qHash(const ChunkID &c) {
  return (c.cx << 16) ^ (c.cz & 0xffff);  // safe way to hash a pair of integers
}

ChunkCache::ChunkCache() {
  int chunks = 10000;  // 10% more than 1920x1200 blocks
#if defined(__unix__) || defined(__unix) || defined(unix)
#ifdef _SC_AVPHYS_PAGES
  auto pages = sysconf(_SC_AVPHYS_PAGES);
  auto page_size = sysconf(_SC_PAGE_SIZE);
  chunks = (pages*page_size) / (sizeof(Chunk) + 16*sizeof(ChunkSection));
#endif
#elif defined(_WIN32) || defined(WIN32)
  MEMORYSTATUSEX status;
  status.dwLength = sizeof(status);
  GlobalMemoryStatusEx(&status);
  DWORDLONG available = qMin(status.ullAvailPhys, status.ullAvailVirtual);
  chunks = available / (sizeof(Chunk) + 16 * sizeof(ChunkSection));
#endif
  cache.setMaxCost(chunks);
  maxcache = 2 * chunks;  // most chunks are less than half filled with sections

  // determain optimal thread pool size for "loading"
  // as this contains disk access, use less than number of cores
  int tmax = loaderThreadPool.maxThreadCount();
  loaderThreadPool.setMaxThreadCount(tmax / 2);

  qRegisterMetaType<QSharedPointer<GeneratedStructure>>("QSharedPointer<GeneratedStructure>");
}

ChunkCache::~ChunkCache() {
  loaderThreadPool.waitForDone();
}

ChunkCache& ChunkCache::Instance() {
  static ChunkCache singleton;
  return singleton;
}

void ChunkCache::clear() {
  QThreadPool::globalInstance()->waitForDone();

  QMutexLocker guard(&mutex);
  cache.clear();
}

void ChunkCache::setPath(QString path) {
  if (this->path != path)
    clear();
  this->path = path;
}
QString ChunkCache::getPath() const {
  return path;
}

int ChunkCache::getCost() const {
  return cache.totalCost();
}

int ChunkCache::getMaxCost() const {
  return cache.maxCost();
}

QSharedPointer<Chunk> ChunkCache::fetchCached(int cx, int cz) {
  // try to get Chunk from Cache
  ChunkID id(cx, cz);

  QMutexLocker guard(&mutex);
  QSharedPointer<Chunk> * p_chunk(cache[id]);   // const operation

  if (p_chunk != NULL )
    return QSharedPointer<Chunk>(*p_chunk);
  else
    return QSharedPointer<Chunk>(NULL);  // we're loading this chunk, or it's blank.
}

QSharedPointer<Chunk> ChunkCache::fetch(int cx, int cz) {
  // try to get Chunk from Cache
  ChunkID id(cx, cz);
  {
    QMutexLocker guard(&mutex);
    QSharedPointer<Chunk> * p_chunk(cache[id]);   // const operation
    if (p_chunk != NULL ) {
      QSharedPointer<Chunk> chunk(*p_chunk);
      if (chunk->loaded)
        return chunk;
      return QSharedPointer<Chunk>(NULL);  // we're loading this chunk, or it's blank.
    }
  }

  // launch background process to load this chunk
  QSharedPointer<Chunk> * p_chunk = new QSharedPointer<Chunk>(new Chunk());
  connect(p_chunk->data(), SIGNAL(structureFound(QSharedPointer<GeneratedStructure>)),
          this,            SLOT  (routeStructure(QSharedPointer<GeneratedStructure>)));

  {
    QMutexLocker guard(&mutex);
    cache.insert(id, p_chunk);    // non-const operation !
  }
  ChunkLoader *loader = new ChunkLoader(path, cx, cz);
  connect(loader, SIGNAL(loaded(int, int)),
          this,   SLOT(gotChunk(int, int)));
  loaderThreadPool.start(loader);
  return QSharedPointer<Chunk>(NULL);
}

void ChunkCache::gotChunk(int cx, int cz) {
  emit chunkLoaded(cx, cz);
}

void ChunkCache::routeStructure(QSharedPointer<GeneratedStructure> structure) {
  emit structureFound(structure);
}

void ChunkCache::adaptCacheToWindow(int wx, int wy) {
  int chunks = ((wx + 15) >> 4) * ((wy + 15) >> 4);  // number of chunks visible
  chunks *= 1.10;  // add 10%

  QMutexLocker guard(&mutex);
  cache.setMaxCost(qMin(chunks, maxcache));
}
