#ifndef PROPERTIETREECREATOR_H
#define PROPERTIETREECREATOR_H

#include <QString>
#include <QMap>
#include <QVariant>

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

#endif // PROPERTIETREECREATOR_H
