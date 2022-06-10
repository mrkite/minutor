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

  int getTop() const;
  int getLeft() const;
  int getBottom() const;
  int getRight() const;

  bool getChunkChecker() const;
  bool getRegionChecker() const;

private:
  Ui::PngExport *ui;

  int  snapDistance;

private slots:
  void checkTop(int value);
  void checkLeft(int value);
  void checkBottom(int value);
  void checkRight(int value);

  QString getLabelText(int value);

  void setSingleStep();
};

#endif // PNGEXPORT_H
