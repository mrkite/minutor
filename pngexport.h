/** Copyright (c) 2017, EtlamGit */
#ifndef PNGEXPORT_H
#define PNGEXPORT_H

#include <QDialog>

namespace Ui {
class PngExport;
}

class PngExport : public QDialog
{
  Q_OBJECT

public:
  explicit PngExport(QWidget *parent = 0);
  ~PngExport();

  void setBoundsFromChunks(int top, int left, int bottom, int right);
  void setBoundsFromBlocks(int top, int left, int bottom, int right);

  int getTop();
  int getLeft();
  int getBottom();
  int getRight();

  bool getChunkChecker();
  bool getRegionChecker();

private:
  Ui::PngExport *ui;

  int w_top;
  int w_left;
  int w_bottom;
  int w_right;

private slots:
  void checkTop(int value);
  void checkLeft(int value);
  void checkBottom(int value);
  void checkRight(int value);
};

#endif // PNGEXPORT_H
