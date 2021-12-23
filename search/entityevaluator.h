#ifndef ENTITYEVALUATOR_H
#define ENTITYEVALUATOR_H

#include "overlay/propertietreecreator.h"
#include "search/searchplugininterface.h"

#include <QSharedPointer>

#include <functional>

class ChunkCache;
class Chunk;
class SearchResultItem;
class GenericIdentifier;
class OverlayItem;

class QTreeWidgetItem;

class EntityEvaluator;

struct EntityEvaluatorConfig
{
  typedef std::function<bool(EntityEvaluator&)> SearchFunctionT;

  EntityEvaluatorConfig(SearchPluginI::ResultListT& resultSink_,
                        QSharedPointer<OverlayItem> entity_,
                        SearchFunctionT evalFunction_)
    : resultSink(resultSink_)
    , entity(entity_)
    , evalFunction(evalFunction_)
  {}

  SearchPluginI::ResultListT& resultSink;
  QSharedPointer<OverlayItem> entity;
  std::function<bool(EntityEvaluator&)> evalFunction;
};

class EntityEvaluator
{
public:
  EntityEvaluator(const EntityEvaluatorConfig& config);

  QList<QString> getVillagerOffers() const;
  QString getSpecialParams() const;

  static const QTreeWidgetItem *getNodeFromPath(const QString path, const QTreeWidgetItem &searchRoot);
  static const QTreeWidgetItem *getNodeFromPath(QStringList::iterator start, QStringList::iterator end, const QTreeWidgetItem &searchRoot);

  static const QString getNodeValueFromPath(const QString path, const QTreeWidgetItem &searchRoot, QString defaultValue);

  QString getTypeId() const;
  bool isVillager() const;
  QString getVillagerProfession() const;

  QString getNamedAttribute(const QString& name) const;

private:
  EntityEvaluatorConfig config;
  QSharedPointer<QTreeWidgetItem> rootNode;
  PropertieTreeCreator creator;

  void searchProperties();
  void searchTreeNode(const QString prefix, const QTreeWidgetItem& node);

  QString describeReceipe(const QTreeWidgetItem& node) const;
  QString describeReceipeItem(const QTreeWidgetItem& node) const;

  void addResult();
};


#endif // ENTITYEVALUATOR_H
