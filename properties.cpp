/** Copyright 2014 Rian Shelley */
#include <QRegularExpression>
#include <QVector3D>
#include "./properties.h"
#include "./ui_properties.h"


Properties::Properties(QWidget *parent) : QDialog(parent),
    ui(new Ui::Properties) {
  ui->setupUi(this);
}

Properties::~Properties() {
  delete ui;
}

void Properties::DisplayProperties(QVariant p) {
  // get current property
  QString propertyName;
  QTreeWidgetItem* item = ui->propertyView->currentItem();
  if (item) {
    propertyName = item->data(0, Qt::DisplayRole).toString();
  }

  ui->propertyView->clear();

  // only support QVariantMap or QVariantHash at this level
  switch (p.type()) {
    case QMetaType::QVariantMap:
      treeCreator.ParseIterable(ui->propertyView->invisibleRootItem(), p.toMap());
      break;
    case QMetaType::QVariantHash:
      treeCreator.ParseIterable(ui->propertyView->invisibleRootItem(), p.toHash());
      break;
    case QMetaType::QVariantList:
      treeCreator.ParseList(ui->propertyView->invisibleRootItem(), p.toList());
      break;
    default:
      qWarning("Trying to display scalar value as a property");
      break;
  }

  // expand at least the first level
  ui->propertyView->expandToDepth(0);

  if (propertyName.size() != 0) {
    // try to restore the path
    QList<QTreeWidgetItem*> items =
        ui->propertyView->findItems(propertyName, Qt::MatchRecursive);
    if (items.size())
      items.front()->setSelected(true);
  }
}

