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

  enum CHUNKLOAD_TYPE {
    MAIN_MAP_DATA      = 0,
    SEPARATED_ENTITIES = 1
  };

  static bool loadNbt(QString path, int cx, int cz, QSharedPointer<Chunk> chunk);
  static bool loadNbtHelper(QString filename, int cx, int cz, QSharedPointer<Chunk> chunk, int loadtype);

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
