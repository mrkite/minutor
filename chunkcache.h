/** Copyright (c) 2013, Sean Kasun */
#ifndef CHUNKCACHE_H_
#define CHUNKCACHE_H_

#include <QObject>
#include <QCache>
#include "./chunk.h"

class ChunkID {
 public:
  ChunkID(int x, int z);
  bool operator==(const ChunkID &) const;
  friend uint qHash(const ChunkID &);
 protected:
  int x, z;
};

class ChunkCache : public QObject {
  Q_OBJECT

 public:
  ChunkCache();
  ~ChunkCache();
  void clear();
  void setPath(QString path);
  QString getPath();
  Chunk *fetch(int x, int z);

 signals:
  void chunkLoaded(int x, int z);

 public slots:
  void adaptCacheToWindow(int x, int y);

 private slots:
  void gotChunk(int x, int z);

 private:
  QString path;
  QCache<ChunkID, Chunk> cache;
  QMutex mutex;
  int maxcache;
};

#endif  // CHUNKCACHE_H_
