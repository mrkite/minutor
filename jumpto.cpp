#include "jumpto.h"
#include "ui_jumpto.h"
#include "minutor.h"

JumpTo::JumpTo(QWidget *parent) : QDialog(parent), ui(new Ui::JumpTo)
{
  ui->setupUi(this);
  // remove "question mark" in title bar
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  readSettings();

  mapview = ((Minutor*)parent)->getMapview();
  connect(mapview, &MapView::coordinatesChanged, this, &JumpTo::updateValues);
}

JumpTo::~JumpTo()
{
  writeSettings();
  delete ui;
}

void JumpTo::updateYrange(int yMin, int yMax)
{
  ui->spinBox_Block_Y->setMinimum(yMin);
  ui->spinBox_Block_Y->setMaximum(yMax);
  ui->spinBox_Chunk_Y->setMinimum(yMin >> 4);
  ui->spinBox_Chunk_Y->setMaximum(yMax >> 4);
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

void JumpTo::readSettings()
{
  QSettings settings;
  restoreGeometry(settings.value("JumpTo/geometry").toByteArray());
  ui->checkBox_Sync->setChecked(settings.value("JumpTo/sync", false).toBool());
  ui->checkBox_Use_Y->setChecked(settings.value("JumpTo/useY", true).toBool());
}

void JumpTo::writeSettings() {
  QSettings settings;
  settings.setValue("JumpTo/geometry", saveGeometry());
  settings.setValue("JumpTo/sync", ui->checkBox_Sync->isChecked());
  settings.setValue("JumpTo/useY", ui->checkBox_Use_Y->isChecked());
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
  mapview->setLocation(ui->spinBox_Block_X->value(), ui->spinBox_Block_Y->value(), ui->spinBox_Block_Z->value(), true, ui->checkBox_Use_Y->isChecked());
}

void JumpTo::on_pushButton_Get_clicked()
{
  location = mapview->getLocation();
  ui->spinBox_Block_X->setValue(location->x);
  ui->spinBox_Block_Y->setValue(location->y);
  ui->spinBox_Block_Z->setValue(location->z);
}
