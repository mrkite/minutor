/** Copyright (c) 2013, Sean Kasun */
#ifndef CHUNKLOADER_H_
#define CHUNKLOADER_H_

#include <QObject>
#include <QRunnable>
#include "chunkcache.h"

class ChunkLoader : public QObject, public QRunnable {
  Q_OBJECT

 public:
  ChunkLoader(QString path, int cx, int cz);
  ~ChunkLoader();

  static bool loadNbt(QString path, int cx, int cz, QSharedPointer<Chunk> chunk);

 signals:
  void loaded(int cx, int cz);

 protected:
  void run();

 private:
  QString path;
  int     cx, cz;
  ChunkCache &cache;
};

#endif  // CHUNKLOADER_H_
