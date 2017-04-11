#include "jumpto.h"
#include "ui_jumpto.h"
#include "minutor.h"

JumpTo::JumpTo(QWidget *parent) : QDialog(parent), ui(new Ui::JumpTo)
{
  ui->setupUi(this);

  setWindowTitle(tr("Jump to location"));
}

JumpTo::~JumpTo()
{
  delete ui;
}

void JumpTo::on_spinBox_Block_X_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Chunk_X, value / 16);
  updateSpinBoxValue(ui->spinBox_Region_X, value / 512);
}

void JumpTo::on_spinBox_Block_Z_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Chunk_Z, value / 16);
  updateSpinBoxValue(ui->spinBox_Region_Z, value / 512);
}

void JumpTo::on_spinBox_Chunk_X_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Block_X, value * 16);
  updateSpinBoxValue(ui->spinBox_Region_X, value / 32);
}

void JumpTo::on_spinBox_Chunk_Z_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Block_Z, value * 16);
  updateSpinBoxValue(ui->spinBox_Region_Z, value / 32);
}

void JumpTo::on_spinBox_Region_X_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Block_X, value * 512);
  updateSpinBoxValue(ui->spinBox_Chunk_X, value * 32);
}

void JumpTo::on_spinBox_Region_Z_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Block_Z, value * 512);
  updateSpinBoxValue(ui->spinBox_Chunk_Z, value * 32);
}

void JumpTo::updateSpinBoxValue(QSpinBox *spinBox, int value) {
  if (spinBox->value() != value) {
    bool oldState = spinBox->blockSignals(true);
    spinBox->setValue(value);
    spinBox->blockSignals(oldState);
  }
}

void JumpTo::on_pushButton_Jump_clicked()
{
  ((Minutor*)parent())->jumpToXZ(ui->spinBox_Block_X->value(), ui->spinBox_Block_Z->value());
}
