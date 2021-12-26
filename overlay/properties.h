/** Copyright 2014 Rian Shelley */
#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include "overlay/propertietreecreator.h"

#include <QDialog>
#include <QMap>
#include <QVector3D>

namespace Ui {
class Properties;
}

class QTreeWidgetItem;

class Properties : public QDialog {
  Q_OBJECT

 public:
  explicit Properties(QWidget *parent = 0);
  ~Properties();

  void DisplayProperties(QVariant p);

 private:
  Ui::Properties *ui;
  PropertieTreeCreator treeCreator;
};

#endif  // PROPERTIES_H_
