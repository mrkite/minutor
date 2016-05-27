/** Copyright (c) 2013, Sean Kasun */
#ifndef WORLDSAVE_H_
#define WORLDSAVE_H_

#include <QObject>
#include <QRunnable>

class MapView;
class Chunk;

class WorldSave : public QObject, public QRunnable {
  Q_OBJECT
 public:
  WorldSave(QString filename, MapView *map, bool regionChecker = false,
            bool chunkChecker = false);
  ~WorldSave();
 signals:
  void progress(QString status, double amount);
  void finished();
 protected:
  void run();
 private:
  void findBounds(QString path, int *top, int *left, int *bottom, int *right);
  void blankChunk(uchar *scanlines, int stride, int x);
  void drawChunk(uchar *scanlines, int stride, int x, Chunk *chunk);

  QString filename;
  MapView *map;
  bool regionChecker;
  bool chunkChecker;
};

#endif  // WORLDSAVE_H_
