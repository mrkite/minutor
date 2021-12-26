/** Copyright 2014 Rian Shelley */
#include <QRegularExpression>
#include <QVector3D>

#include "overlay/properties.h"

#include "ui_properties.h"


Properties::Properties(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::Properties)
{
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
  switch ((QMetaType::Type)p.type()) {
    case QMetaType::QVariantMap: {
      // we got a QMap with all properties stored as [key]->value
      auto data = p.toMap();
      treeCreator.ParseIterable(ui->propertyView->invisibleRootItem(), data);
      this->setWindowTitle(data.value("id", "Properties").toString());
      break;
    }
    case QMetaType::QVariantHash: {
      // we got a QHashMap with all properties stored as [key]->value
      auto data = p.toHash();
      treeCreator.ParseIterable(ui->propertyView->invisibleRootItem(), data);
      this->setWindowTitle(data.value("id", "Properties").toString());
      break;
    }
    case QMetaType::QVariantList: {
      // we got a QList with Properties of several objects
      //  each element is a QMap with all properties stored as [key]->value
      auto data = p.toList();
      treeCreator.ParseList(ui->propertyView->invisibleRootItem(), data);
      if (data.length()>0) {
        // we take the Title from the first list element, ignoring all other
        this->setWindowTitle(data[0].toMap().value("id", "Properties").toString());
      }
      break;
    }
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

