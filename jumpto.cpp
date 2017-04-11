#include "jumpto.h"
#include "ui_jumpto.h"
#include "minutor.h"

JumpTo::JumpTo(QWidget *parent) : QDialog(parent), ui(new Ui::JumpTo)
{
  ui->setupUi(this);

  setWindowTitle(tr("Jump to location"));

  connect(ui->pushButton_Jump, SIGNAL(clicked()), this, SLOT(slotApply()));
}

JumpTo::~JumpTo()
{
  delete ui;
}

void JumpTo::slotApply() {
  ((Minutor*)parent())->jumpToXZ(ui->spinBox_Block_X->value(),ui->spinBox_Block_Z->value());
}

