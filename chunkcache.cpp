/** Copyright (c) 2013, Sean Kasun */

#include "chunkcache.h"
#include "chunkloader.h"


#if defined(__unix__) || defined(__unix) || defined(unix)
#include <unistd.h>
#elif defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif

ChunkCache::ChunkCache() {
  const int sizeChunkMax     = sizeof(Chunk) + 16 * sizeof(ChunkSection);  // all sections contain Blocks
  const int sizeChunkTypical = sizeof(Chunk) + 6 * sizeof(ChunkSection);   // world generation is average Y=64..128

  // default: 10% more than 1920x1200 blocks
  int chunks = 10000;
  maxcache = chunks;

  // try to determine available pysical memory based on operation system we are running on
#if defined(__unix__) || defined(__unix) || defined(unix)
#ifdef _SC_AVPHYS_PAGES
  auto pages = sysconf(_SC_AVPHYS_PAGES);
  auto page_size = sysconf(_SC_PAGE_SIZE);
  quint64 available = (pages*page_size);
  chunks   = available / sizeChunkMax;
  maxcache = available / sizeChunkTypical;  // most chunks are less filled with sections
#endif
#elif defined(_WIN32) || defined(WIN32)
  MEMORYSTATUSEX status;
  status.dwLength = sizeof(status);
  GlobalMemoryStatusEx(&status);
  DWORDLONG available = qMin(status.ullAvailPhys, status.ullAvailVirtual);
  chunks   = available / sizeChunkMax;
  maxcache = available / sizeChunkTypical;  // most chunks are less filled with sections
#endif
  // we start the Cache based on worst case calculation
  cache.setMaxCost(chunks);

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

int ChunkCache::getCacheUsage() const {
  return cache.totalCost();
}

int ChunkCache::getCacheMax() const {
  return cache.maxCost();
}

int ChunkCache::getMemoryMax() const {
  return maxcache;
}

QSharedPointer<Chunk> ChunkCache::fetchCached(int cx, int cz) {
  // try to get Chunk from Cache
  ChunkID id(cx, cz);

  QSharedPointer<Chunk> chunk;
  getCached(id, chunk);
  return chunk;
}

CacheState ChunkCache::getCached(const ChunkID &id, QSharedPointer<Chunk> &chunk_out)
{
  QMutexLocker guard(&mutex);
  return getCached_intern(id, chunk_out);
}

CacheState ChunkCache::getCached_intern(const ChunkID &id, QSharedPointer<Chunk> &chunk_out)
{
  QSharedPointer<Chunk> * p_chunk = cache[id];   // const operation
  if (!p_chunk)
  {
    return CacheState::uncached;
  }

  chunk_out = (*p_chunk);

  if (!chunk_out)
    return CacheState::cached; // cached - but not existing and thus empty

  if (!chunk_out->loaded)
    return CacheState::uncached_loading;

  return CacheState::cached;
}

QSharedPointer<Chunk> ChunkCache::fetch(int cx, int cz) {
  // try to get Chunk from Cache
  ChunkID id(cx, cz);
  QSharedPointer<Chunk> chunk;
  const CacheState state = getCached(id, chunk);
  if (state == CacheState::cached)
    return chunk;
  else if (state == CacheState::uncached_loading)
    return QSharedPointer<Chunk>(); // already loading, return nullptr

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

QSharedPointer<Chunk> ChunkCache::getChunkSynchronously(const ChunkID& id)
{
  QSharedPointer<Chunk> chunk;
  bool hasFreeSpaceInCache = false;
  {
    QMutexLocker guard(&mutex);
    hasFreeSpaceInCache = (cache.totalCost() < cache.maxCost() * 0.9);

    const CacheState state = getCached_intern(id, chunk);
    if (state == CacheState::cached)
      return chunk;
  }

  // sychronously load
  chunk = QSharedPointer<Chunk>::create();

  if (!ChunkLoader::loadNbt(path, id.getX(), id.getZ(), chunk))
  {
    return QSharedPointer<Chunk>();
  }

  if (hasFreeSpaceInCache && chunk->loaded) // only cache in case of lot of memory to not degrade drawing performance
  {
    QMutexLocker guard(&mutex);
    cache.insert(id, new QSharedPointer<Chunk>(chunk));
  }

  return chunk;
}

void ChunkCache::gotChunk(int cx, int cz) {
  emit chunkLoaded(cx, cz);
}

void ChunkCache::routeStructure(QSharedPointer<GeneratedStructure> structure) {
  emit structureFound(structure);
}

void ChunkCache::setCacheMaxSize(int chunks) {
  QMutexLocker guard(&mutex);
  // we never decrease Cache size, and never exceed physical memory
  cache.setMaxCost(std::max<int>(cache.maxCost(), std::min<int>(chunks, maxcache)));
}
