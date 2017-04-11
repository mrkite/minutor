#ifndef JUMPTO_H
#define JUMPTO_H

#include <QDialog>

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
  void on_spinBox_Block_Z_valueChanged(int value);
  void on_spinBox_Chunk_X_valueChanged(int value);
  void on_spinBox_Chunk_Z_valueChanged(int value);
  void on_spinBox_Region_X_valueChanged(int value);
  void on_spinBox_Region_Z_valueChanged(int value);
  void on_pushButton_Jump_clicked();

private:
  Ui::JumpTo *ui;
};

#endif // JUMPTO_H
