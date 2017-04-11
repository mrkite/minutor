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
  bool oldState = ui->spinBox_Chunk_X->blockSignals(true);
  ui->spinBox_Chunk_X->setValue(value / 16);
  ui->spinBox_Chunk_X->blockSignals(oldState);

  oldState = ui->spinBox_Region_X->blockSignals(true);
  ui->spinBox_Region_X->setValue(value / 512);
  ui->spinBox_Region_X->blockSignals(oldState);
}

void JumpTo::on_spinBox_Block_Z_valueChanged(int value)
{
  bool oldState = ui->spinBox_Chunk_Z->blockSignals(true);
  ui->spinBox_Chunk_Z->setValue(value / 16);
  ui->spinBox_Chunk_Z->blockSignals(oldState);

  oldState = ui->spinBox_Region_Z->blockSignals(true);
  ui->spinBox_Region_Z->setValue(value / 512);
  ui->spinBox_Region_Z->blockSignals(oldState);
}

void JumpTo::on_spinBox_Chunk_X_valueChanged(int value)
{
  bool oldState = ui->spinBox_Block_X->blockSignals(true);
  ui->spinBox_Block_X->setValue(value * 16);
  ui->spinBox_Block_X->blockSignals(oldState);

  oldState = ui->spinBox_Region_X->blockSignals(true);
  ui->spinBox_Region_X->setValue(value / 32);
  ui->spinBox_Region_X->blockSignals(oldState);
}

void JumpTo::on_spinBox_Chunk_Z_valueChanged(int value)
{
  bool oldState = ui->spinBox_Block_Z->blockSignals(true);
  ui->spinBox_Block_Z->setValue(value * 16);
  ui->spinBox_Block_Z->blockSignals(oldState);

  oldState = ui->spinBox_Region_Z->blockSignals(true);
  ui->spinBox_Region_Z->setValue(value / 32);
  ui->spinBox_Region_Z->blockSignals(oldState);
}

void JumpTo::on_spinBox_Region_X_valueChanged(int value)
{
  bool oldState = ui->spinBox_Block_X->blockSignals(true);
  ui->spinBox_Block_X->setValue(value * 512);
  ui->spinBox_Block_X->blockSignals(oldState);

  oldState = ui->spinBox_Chunk_X->blockSignals(true);
  ui->spinBox_Chunk_X->setValue(value * 32);
  ui->spinBox_Chunk_X->blockSignals(oldState);
}

void JumpTo::on_spinBox_Region_Z_valueChanged(int value)
{
  bool oldState = ui->spinBox_Block_Z->blockSignals(true);
  ui->spinBox_Block_Z->setValue(value * 512);
  ui->spinBox_Block_Z->blockSignals(oldState);

  oldState = ui->spinBox_Chunk_Z->blockSignals(true);
  ui->spinBox_Chunk_Z->setValue(value * 32);
  ui->spinBox_Chunk_Z->blockSignals(oldState);
}

void JumpTo::on_pushButton_Jump_clicked()
{
  ((Minutor*)parent())->jumpToXZ(ui->spinBox_Block_X->value(), ui->spinBox_Block_Z->value());
}
