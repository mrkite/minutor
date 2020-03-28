/** Copyright (c) 2013, Sean Kasun */
#ifndef CHUNKCACHE_H_
#define CHUNKCACHE_H_

#include <QObject>
#include <QCache>
#include "./chunk.h"
#include "./chunkid.h"

enum class CacheState {
  uncached,
  uncached_loading,
  cached // still can be nullptr when empty
};

class ChunkCache : public QObject {
  Q_OBJECT

 public:
  // singleton: access to global usable instance
  static ChunkCache &Instance();
 private:
  // singleton: prevent access to constructor and copyconstructor
  ChunkCache();
  ~ChunkCache();
  ChunkCache(const ChunkCache &);
  ChunkCache &operator=(const ChunkCache &);

 public:
  void clear();
  void setPath(QString path);
  QString getPath() const;
  QSharedPointer<Chunk> fetch(int cx, int cz);         // fetch Chunk and load when not found
  QSharedPointer<Chunk> fetchCached(int cx, int cz);   // fetch Chunk only if cached
  CacheState getCached(const ChunkID& id, QSharedPointer<Chunk>& chunk_out);    // fetch Chunk only if cached, can tell if just not loaded or empty
  QSharedPointer<Chunk> getChunkSynchronously(const ChunkID& id);         // get chunk if cached directly, or load it in a synchronous blocking way
  int getCacheUsage() const;
  int getCacheMax() const;
  int getMemoryMax() const;

 signals:
  void chunkLoaded(int cx, int cz);
  void structureFound(QSharedPointer<GeneratedStructure> structure);

 public slots:
  void setCacheMaxSize(int chunks);

 private slots:
  void gotChunk(int cx, int cz);
  void routeStructure(QSharedPointer<GeneratedStructure> structure);

 private:
  QString path;                                   // path to folder with region files
  QCache<ChunkID, QSharedPointer<Chunk>> cache;   // real Cache
  QMutex mutex;                                   // Mutex for accessing the Cache
  int maxcache;                                   // number of Chunks that fit into memory
  QThreadPool loaderThreadPool;                   // extra thread pool for loading

  CacheState getCached_intern(const ChunkID& id, QSharedPointer<Chunk>& chunk_out);
};

#endif  // CHUNKCACHE_H_
