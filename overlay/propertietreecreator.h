#ifndef PROPERTIETREECREATOR_H
#define PROPERTIETREECREATOR_H

#include <QString>
#include <QMap>
#include <QVariant>
#include <QTreeWidgetItem>

class QTreeWidgetItem;

class PropertieTreeCreator
{
public:
  PropertieTreeCreator();

  template <class IterableT>
  void ParseIterable(QTreeWidgetItem* node, const IterableT& seq);
  template <class IterableT>
  void ParseList(QTreeWidgetItem* node, const IterableT& seq);

  void CreateTree(QTreeWidgetItem *node, const QVariant& v);
  QString GetSummary(const QString& key, const QVariant& v);

private:
  QMap<QString, QString> summary;
};


template <class IterableT>
inline void PropertieTreeCreator::ParseIterable(QTreeWidgetItem* node, const IterableT& seq) {
  typename IterableT::const_iterator it, itEnd = seq.end();
  for (it = seq.begin(); it != itEnd; ++it) {
    QTreeWidgetItem* child = new QTreeWidgetItem();
    child->setData(0, Qt::DisplayRole, it.key());
    child->setData(1, Qt::DisplayRole, GetSummary(it.key(), it.value()));
    CreateTree(child, it.value());
    node->addChild(child);
  }
}


template <class IterableT>
inline void PropertieTreeCreator::ParseList(QTreeWidgetItem* node, const IterableT& seq) {
  typename IterableT::const_iterator it, itEnd = seq.end();
  int i = 0;
  // skip 1 sized arrays
  if (seq.size() == 0) {
    // empty
    if (node)
      node->setData(1, Qt::DisplayRole, "<empty>");
  } else if (seq.size() == 1) {
    CreateTree(node, seq.first());
    if (node)
      node->setData(1, Qt::DisplayRole, GetSummary("[0]", seq.first()));
  } else {
    for (it = seq.begin(); it != itEnd; ++it) {
      QTreeWidgetItem* child = new QTreeWidgetItem();
      QString key = QString("[%1]").arg(i++);
      child->setData(0, Qt::DisplayRole, key);
      child->setData(1, Qt::DisplayRole, GetSummary(key, *it));
      CreateTree(child, *it);

      node->addChild(child);
    }
  }
}


#endif // PROPERTIETREECREATOR_H
