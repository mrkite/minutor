/** Copyright (c) 2013, Sean Kasun */
#ifndef CHUNKCACHE_H_
#define CHUNKCACHE_H_

#include <QObject>
#include <QCache>
#include "./chunk.h"

class ChunkID {
 public:
  ChunkID(int cx, int cz);
  bool operator==(const ChunkID &) const;
  friend uint qHash(const ChunkID &);
 protected:
  int cx, cz;
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
  Chunk *fetch(int cx, int cz);         // fetch Chunk and load when not found
  Chunk *fetchCached(int cx, int cz);   // fetch Chunk only if cached

 signals:
  void chunkLoaded(int cx, int cz);
  void structureFound(QSharedPointer<GeneratedStructure> structure);

 public slots:
  void adaptCacheToWindow(int wx, int wy);

 private slots:
  void gotChunk(int cx, int cz);
  void routeStructure(QSharedPointer<GeneratedStructure> structure);

 private:
  QString path;                   // path to folder with region files
  QCache<ChunkID, Chunk> cache;   // real Cache
  QMutex mutex;                   // Mutex for accessing the Cache
  int maxcache;                   // number of Chunks that fit into Cache
  QThreadPool loaderThreadPool;   // extra thread pool for loading
};

#endif  // CHUNKCACHE_H_
