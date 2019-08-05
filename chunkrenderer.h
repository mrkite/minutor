/** Copyright (c) 2019, Mc_Etlam */
#ifndef CHUNKRENDERER_H
#define CHUNKRENDERER_H

#include <QObject>
#include <QRunnable>
#include "chunkcache.h"

class ChunkRenderer : public QObject, public QRunnable {
  Q_OBJECT

 public:
  ChunkRenderer(int cx, int cz, int y, int flags);
  ~ChunkRenderer() {}

 protected:
  void run();

 public:  // public to allow usage from WorldSave
  void ChunkRenderer::renderChunk(Chunk *chunk);

 signals:
  void rendered(int cx, int cz);

 private:
  int cx, cz;
  int depth;
  int flags;
  ChunkCache &cache;
};

class CaveShade {
 public:
  // singleton: access to global usable instance
  static float getShade(int index);
 private:
  // singleton: prevent access to constructor and copyconstructor
  CaveShade();
  ~CaveShade() {}
  CaveShade(const CaveShade &);
  CaveShade &operator=(const CaveShade &);

 public:
  static const int CAVE_DEPTH = 16;  // maximum depth caves are searched in cave mode
  float caveshade[CAVE_DEPTH];
};

#endif // CHUNKRENDERER_H
