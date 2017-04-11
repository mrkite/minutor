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
  void slotApply();

private:
  Ui::JumpTo *ui;
};

#endif // JUMPTO_H
