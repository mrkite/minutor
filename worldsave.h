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
  WorldSave(QString filename, MapView *map,
            bool regionChecker = false, bool chunkChecker = false,
            int w_top = 0, int w_left = 0, int w_bottom = 0, int w_right = 0);
  ~WorldSave();

 signals:
  void progress(QString status, double amount);
  void finished();

 protected:
  void run();

 private:
  void blankChunk(uchar *scanlines, int stride, int x);
  void drawChunk(uchar *scanlines, int stride, int x, QSharedPointer<Chunk> chunk);

  QString filename;
  MapView *map;
  int top;
  int left;
  int bottom;
  int right;
  bool regionChecker;
  bool chunkChecker;

 public: // static
  static void findWorldBounds(QString path, int *top, int *left, int *bottom, int *right);
};

#endif  // WORLDSAVE_H_
