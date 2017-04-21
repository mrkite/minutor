#include "jumpto.h"
#include "ui_jumpto.h"
#include "minutor.h"

JumpTo::JumpTo(QWidget *parent) : QDialog(parent), ui(new Ui::JumpTo)
{
  ui->setupUi(this);

  ui->pushButton_Get->setEnabled(!ui->checkBox_Sync->isChecked());

  mapview = ((Minutor*)parent)->getMapview();
  connect(mapview, &MapView::coordinatesChanged, this, &JumpTo::updateValues);

  setWindowTitle(tr("Jump to location"));
}

JumpTo::~JumpTo()
{
  delete ui;
}

void JumpTo::on_spinBox_Block_X_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Chunk_X, value >> 4);
  updateSpinBoxValue(ui->spinBox_Region_X, value >> 9);
}

void JumpTo::on_spinBox_Block_Y_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Chunk_Y, value >> 4);
}

void JumpTo::on_spinBox_Block_Z_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Chunk_Z, value >> 4);
  updateSpinBoxValue(ui->spinBox_Region_Z, value >> 9);
}

void JumpTo::on_spinBox_Chunk_X_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Block_X, value << 4);
  updateSpinBoxValue(ui->spinBox_Region_X, value >> 5);
}

void JumpTo::on_spinBox_Chunk_Y_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Block_Y, value << 4);
}

void JumpTo::on_spinBox_Chunk_Z_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Block_Z, value << 4);
  updateSpinBoxValue(ui->spinBox_Region_Z, value >> 5);
}

void JumpTo::on_spinBox_Region_X_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Block_X, value << 9);
  updateSpinBoxValue(ui->spinBox_Chunk_X, value << 5);
}

void JumpTo::on_spinBox_Region_Z_valueChanged(int value)
{
  updateSpinBoxValue(ui->spinBox_Block_Z, value << 9);
  updateSpinBoxValue(ui->spinBox_Chunk_Z, value << 5);
}

void JumpTo::updateSpinBoxValue(QSpinBox *spinBox, int value) {
  if (spinBox->value() != value) {
    bool oldState = spinBox->blockSignals(true);
    spinBox->setValue(value);
    spinBox->blockSignals(oldState);
  }
}

void JumpTo::updateValues(int x, int y, int z) {
  if (ui->checkBox_Sync->isChecked()) {
    ui->spinBox_Block_X->setValue(x);
    ui->spinBox_Block_Y->setValue(y);
    ui->spinBox_Block_Z->setValue(z);
  }
}

void JumpTo::on_pushButton_Jump_clicked()
{
  mapview->setLocation(ui->spinBox_Block_X->value(), ui->spinBox_Block_Y->value(), ui->spinBox_Block_Z->value(), true);
}

void JumpTo::on_pushButton_Get_clicked()
{
  ui->spinBox_Block_X->setValue(-803);
  ui->spinBox_Block_Z->setValue(1975);
}

void JumpTo::on_checkBox_Sync_stateChanged(int state)
{
  Q_UNUSED(state);
  ui->pushButton_Get->setEnabled(!ui->checkBox_Sync->isChecked());
}
