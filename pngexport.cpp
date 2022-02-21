#include "pngexport.h"
#include "ui_pngexport.h"

#include <QSettings>
#include <cmath>

PngExport::PngExport(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::PngExport)
  , snapDistance(16)
{
  ui->setupUi(this);
  // remove "question mark" in title bar
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  // connect value change signals to check slots of "other" spinbox
  connect(ui->spinBox_top,    QOverload<int>::of(&QSpinBox::valueChanged),
          this,               &PngExport::checkTop);
  connect(ui->spinBox_left,   QOverload<int>::of(&QSpinBox::valueChanged),
          this,               &PngExport::checkLeft);
  connect(ui->spinBox_bottom, QOverload<int>::of(&QSpinBox::valueChanged),
          this,               &PngExport::checkBottom);
  connect(ui->spinBox_right,  QOverload<int>::of(&QSpinBox::valueChanged),
          this,               &PngExport::checkRight);

  // connect radio buttons to change single-step distance
  connect(ui->radioButton_chunk,  &QRadioButton::toggled,
          this,                   &PngExport::setSingleStep);
  connect(ui->radioButton_region, &QRadioButton::toggled,
          this,                   &PngExport::setSingleStep);

  // load the settings
  QSettings settings;
  if (settings.value("PNGExportSnapDistance", 16).toInt() == 512) {
    snapDistance = 512;
    ui->radioButton_region->setChecked(true);
  }
}

PngExport::~PngExport()
{
  delete ui;
}

void PngExport::setBoundsFromChunks(int top, int left, int bottom, int right)
{
  // convert from Chunks to Blocks
  setBoundsFromBlocks(16*top, 16*left, 16*bottom+15, 16*right+15);
}

void PngExport::setBoundsFromBlocks(int top, int left, int bottom, int right)
{
  // extend to next region boundary
  int e_top    = 512* std::floor(float(top)/512.0);
  int e_left   = 512* std::floor(float(left)/512.0);
  int e_bottom = 512*(std::floor(float(bottom)/512.0) +1) -1;
  int e_right  = 512*(std::floor(float(right)/512.0) +1) -1;

  // restrict range of spin boxes to world dimension
  ui->spinBox_top   ->setRange(e_top, e_bottom-(snapDistance-1));
  ui->spinBox_left  ->setRange(e_left, e_right-(snapDistance-1));
  ui->spinBox_bottom->setRange(e_top+(snapDistance-1), e_bottom);
  ui->spinBox_right ->setRange(e_left+(snapDistance-1), e_right);

  // initialize spin boxes to complete world dimension
  ui->spinBox_top   ->setValue(top);
  ui->spinBox_left  ->setValue(left);
  ui->spinBox_bottom->setValue(bottom);
  ui->spinBox_right ->setValue(right);
}


void PngExport::checkTop(int value)
{
  value = snapDistance * std::round(float(value)/snapDistance);
  ui->spinBox_top->setValue(value);
  if (value+(snapDistance-1) > ui->spinBox_bottom->value())
    ui->spinBox_bottom->setValue(value+(snapDistance-1));
}

void PngExport::checkLeft(int value)
{
  value = snapDistance * std::round(float(value)/snapDistance);
  ui->spinBox_left->setValue(value);
  if (value+(snapDistance-1) > ui->spinBox_right->value())
    ui->spinBox_right->setValue(value+(snapDistance-1));
}

void PngExport::checkBottom(int value)
{
  value = snapDistance * std::round(float(value)/snapDistance) -1;
  ui->spinBox_bottom->setValue(value);
  if (value-(snapDistance-1) < ui->spinBox_top->value())
    ui->spinBox_top->setValue(value-(snapDistance-1));
}

void PngExport::checkRight(int value)
{
  value = snapDistance * std::round(float(value)/snapDistance) -1;
  ui->spinBox_right->setValue(value);
  if (value-(snapDistance-1) < ui->spinBox_left->value())
    ui->spinBox_left->setValue(value-(snapDistance-1));
}

void PngExport::setSingleStep()
{
  if (ui->radioButton_chunk ->isChecked()) snapDistance = 16;
  if (ui->radioButton_region->isChecked()) snapDistance = 512;
  // change single step behavior
  ui->spinBox_top   ->setSingleStep(snapDistance);
  ui->spinBox_left  ->setSingleStep(snapDistance);
  ui->spinBox_bottom->setSingleStep(snapDistance);
  ui->spinBox_right ->setSingleStep(snapDistance);

  // validate current values
  checkTop   (ui->spinBox_top   ->value());
  checkLeft  (ui->spinBox_left  ->value());
  checkBottom(ui->spinBox_bottom->value());
  checkRight (ui->spinBox_right ->value());

  // store the settings
  QSettings settings;
  settings.setValue("PNGExportSnapDistance", snapDistance);
}


int PngExport::getTop() const    { return ui->spinBox_top   ->value(); }
int PngExport::getLeft() const   { return ui->spinBox_left  ->value(); }
int PngExport::getBottom() const { return ui->spinBox_bottom->value(); }
int PngExport::getRight() const  { return ui->spinBox_right ->value(); }

bool PngExport::getChunkChecker() const  { return ui->checkBox_chunk ->checkState(); }
bool PngExport::getRegionChecker() const { return ui->checkBox_region->checkState(); }
