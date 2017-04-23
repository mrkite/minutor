#ifndef JUMPTO_H
#define JUMPTO_H

#include <QDialog>
#include <QSpinBox>
#include "mapview.h"

namespace Ui {
class JumpTo;
}

class JumpTo : public QDialog
{
  Q_OBJECT

public:
  explicit JumpTo(QWidget *parent = 0);
  ~JumpTo();

private slots:
  void on_spinBox_Block_X_valueChanged(int value);
  void on_spinBox_Block_Y_valueChanged(int value);
  void on_spinBox_Block_Z_valueChanged(int value);
  void on_spinBox_Chunk_X_valueChanged(int value);
  void on_spinBox_Chunk_Y_valueChanged(int value);
  void on_spinBox_Chunk_Z_valueChanged(int value);
  void on_spinBox_Region_X_valueChanged(int value);
  void on_spinBox_Region_Z_valueChanged(int value);
  void on_pushButton_Jump_clicked();
  void on_pushButton_Get_clicked();
  void updateValues(int x, int y, int z);

  void on_checkBox_Sync_stateChanged(int state);

private:
  Ui::JumpTo *ui;
  MapView *mapview;
  MapView::BlockLocation *location;
  void updateSpinBoxValue(QSpinBox *spinBox, int value);
};

#endif // JUMPTO_H
