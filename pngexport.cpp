#include "pngexport.h"
#include "ui_pngexport.h"

PngExport::PngExport(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::PngExport)
{
  ui->setupUi(this);

  // connect value change signals to check slots
  connect(ui->spinBox_top,    SIGNAL(valueChanged(int)),
          this,               SLOT(checkTop(int)));
  connect(ui->spinBox_left,   SIGNAL(valueChanged(int)),
          this,               SLOT(checkLeft(int)));
  connect(ui->spinBox_bottom, SIGNAL(valueChanged(int)),
          this,               SLOT(checkBottom(int)));
  connect(ui->spinBox_right,  SIGNAL(valueChanged(int)),
          this,               SLOT(checkRight(int)));

}

PngExport::~PngExport()
{
  delete ui;
}

void PngExport::setBounds(int top, int left, int bottom, int right)
{
  // convert from Chunks to Blocks
  w_top    = 16*top;
  w_left   = 16*left;
  w_bottom = 16*bottom+15;
  w_right  = 16*right+15;

  // restrict range of spin boxes to world dimension
  ui->spinBox_top   ->setRange(w_top, w_bottom-15);
  ui->spinBox_left  ->setRange(w_left, w_right-15);
  ui->spinBox_bottom->setRange(w_top+15, w_bottom);
  ui->spinBox_right ->setRange(w_left+15, w_right);

  // initialize spin boxes to complete world dimension
  ui->spinBox_top   ->setValue(w_top);
  ui->spinBox_left  ->setValue(w_left);
  ui->spinBox_bottom->setValue(w_bottom);
  ui->spinBox_right ->setValue(w_right);
}


void PngExport::checkTop(int value)
{
//  value = 16*round(float(value)/16);
//  ui->spinBox_top->setValue(value);
  if (value+15 > ui->spinBox_bottom->value())
    ui->spinBox_bottom->setValue(value+15);
}

void PngExport::checkLeft(int value)
{
//  value = 16*round(float(value)/16);
//  ui->spinBox_left->setValue(value);
  if (value+15 > ui->spinBox_right->value())
    ui->spinBox_right->setValue(value+15);
}

void PngExport::checkBottom(int value)
{
//  value = 16*round(float(value)/16);
//  ui->spinBox_bottom->setValue(value);
  if (value-15 < ui->spinBox_top->value())
    ui->spinBox_top->setValue(value-15);
}

void PngExport::checkRight(int value)
{
//  value = 16*round(float(value)/16);
//  ui->spinBox_right->setValue(value);
  if (value-15 < ui->spinBox_left->value())
    ui->spinBox_left->setValue(value-15);
}


int PngExport::getTop()    { return ui->spinBox_top->value(); }
int PngExport::getLeft()   { return ui->spinBox_left->value(); }
int PngExport::getBottom() { return ui->spinBox_bottom->value(); }
int PngExport::getRight()  { return ui->spinBox_right->value(); }

bool PngExport::getChunkChecker()  { return ui->checkBox_chunk->checkState(); }
bool PngExport::getRegionChecker() { return ui->checkBox_region->checkState(); }
