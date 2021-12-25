#include "search/entityevaluator.h"

#include "overlay/overlayitem.h"
//#include "genericidentifier.h"
#include "search/searchresultwidget.h"
//#include "careeridentifier.h"

#include <QTreeWidgetItem>

static const QString prefixToBeRemoved = "minecraft:";

static QString removeMinecraftPrefix(const QString& id)
{
  if (id.startsWith(prefixToBeRemoved)) {
    return id.mid(prefixToBeRemoved.size());
  }

  return id;
}

using SpecialParamsFunctionT = std::function<QString(const EntityEvaluator &entity)>;

static std::map<QString, SpecialParamsFunctionT> special_param_extractor =
{
  {"minecraft:horse", SpecialParamsFunctionT([](const EntityEvaluator &entity) -> QString {
      return "horse,speed/jump/health: "
        + entity.getNamedAttribute("generic.movementSpeed") + entity.getNamedAttribute("generic.movement_speed") + "/"
        + entity.getNamedAttribute("horse.jumpStrength")    + entity.getNamedAttribute("horse.jump_strength")    + "/"
        + entity.getNamedAttribute("generic.maxHealth")     + entity.getNamedAttribute("generic.max_health");
    })
  },
  {"minecraft:villager", SpecialParamsFunctionT([](const EntityEvaluator &entity) -> QString {
      return "[" + removeMinecraftPrefix(entity.getVillagerProfession()) + "]: " + entity.getVillagerOffers().join(" | ");
    })
  }
};


EntityEvaluator::EntityEvaluator(const EntityEvaluatorConfig& config)
  : config(config)
  , rootNode(QSharedPointer<QTreeWidgetItem>::create())
{
  creator.CreateTree(rootNode.data(), config.entity->properties());

  //searchProperties();

  bool found = config.evalFunction(*this);
  if (found) {
    addResult();
  }
}

QList<QString> EntityEvaluator::getVillagerOffers() const
{
  QList<QString> result;

  auto* node = getNodeFromPath("Offers/Recipes", *rootNode);
  if (!node) {
    return result;
  }

  {   // single receipe only?
    QString receipeDescription = describeReceipe(*node);
    if (receipeDescription.size() > 0) {
      result.append(receipeDescription);
      return result;
    }
  }

  // multiple ones:
  for (int i = 0; i < node->childCount(); i++) {
    auto& currentReceipNode = *node->child(i);
    QString receipeDescription = describeReceipe(currentReceipNode);
    if (receipeDescription.size() > 0) {
      result.append(receipeDescription);
    }
  }

  return result;
}

QString EntityEvaluator::getSpecialParams() const
{
  auto it = special_param_extractor.find(getTypeId());
  if (it != special_param_extractor.end()) {
    return it->second(*this);
  }

  return "";
}

void EntityEvaluator::searchProperties()
{
  searchTreeNode("", *rootNode);
}

void EntityEvaluator::searchTreeNode(const QString prefix, const QTreeWidgetItem &node)
{
  auto keyText = prefix + node.text(0);
  auto valueText = node.text(1);

  bool found = config.evalFunction(*this);
  if (found) {
    addResult();
  } else {
    for (int i = 0; i < node.childCount(); i++) {
      searchTreeNode(keyText + ".", *node.child(i));
    }
  }
}

QString EntityEvaluator::describeReceipe(const QTreeWidgetItem &currentReceipNode) const
{
  QString result = "";

  auto* buyNode = getNodeFromPath("buy", currentReceipNode);
  if (buyNode) {
    result += describeReceipeItem(*buyNode);
  }

  auto* buyBNode = getNodeFromPath("buyB", currentReceipNode);
  if (buyBNode) {
    QString buyB = describeReceipeItem(*buyBNode);
    if (buyB != "air")
      result += "," + buyB;
  }

  auto* sellNode = getNodeFromPath("sell", currentReceipNode);
  if (sellNode) {
    result += " => " + describeReceipeItem(*sellNode);
  }

  return result;
}

QString EntityEvaluator::describeReceipeItem(const QTreeWidgetItem &itemNode) const
{
  QString value = "";

  auto* itemIdNode = getNodeFromPath("id", itemNode);
  if (itemIdNode) {
    auto* itemCountNode = getNodeFromPath("Count", itemNode);
    int count = itemCountNode->text(1).toInt();
    if (count > 1) {
      value += QString::number(count) + "*";
    }

    QString id = removeMinecraftPrefix(itemIdNode->text(1));

    value += id;
  }

  return value;
}

void EntityEvaluator::addResult()
{
  SearchResultItem result;
  result.properties = config.entity->properties();
  result.name = creator.GetSummary("[0]", config.entity->properties());
  result.pos.setX(config.entity->midpoint().x);
  result.pos.setY(config.entity->midpoint().y);
  result.pos.setZ(config.entity->midpoint().z);
  result.offers = getSpecialParams();
  result.entity = config.entity;
  config.resultSink.push_back(result);
}

const QTreeWidgetItem *EntityEvaluator::getNodeFromPath(const QString path, const QTreeWidgetItem &searchRoot)
{
  QStringList elements = path.split("/");

  return getNodeFromPath(elements.begin(), elements.end(), searchRoot);
}

const QTreeWidgetItem *EntityEvaluator::getNodeFromPath(QStringList::iterator iter, QStringList::iterator end, const QTreeWidgetItem &searchRoot)
{
  QString nextName = *iter;
  iter++;

  for (int i = 0; i < searchRoot.childCount(); i++)
  {
    auto *child = searchRoot.child(i);
    if (child && (child->text(0) == nextName)) {
      if (iter != end) {
        return getNodeFromPath(iter, end, *child);
      } else {
        return child;
      }
    }
  }

  return nullptr;
}

const QString EntityEvaluator::getNodeValueFromPath(const QString path, const QTreeWidgetItem &searchRoot, QString defaultValue)
{
  auto* node = getNodeFromPath(path, searchRoot);
  if (node) {
    return node->text(1);
  }

  return defaultValue;
}

QString EntityEvaluator::getTypeId() const
{
  auto* itemIdNode = getNodeFromPath("id", *rootNode);
  if (itemIdNode) {
    return itemIdNode->text(1);
  }

  return "-";
}

bool EntityEvaluator::isVillager() const
{
  auto id = getTypeId();
  return (id == "minecraft:villager");
}

QString EntityEvaluator::getVillagerProfession() const
{
  auto node = getNodeFromPath("VillagerData", *rootNode);
  if (node != nullptr) {
    return getNodeValueFromPath("profession", *node, "");
  }

  return "-";
}

QString EntityEvaluator::getNamedAttribute(const QString &name) const
{
  auto node = getNodeFromPath("Attributes", *rootNode);

  if (!node) return "";

  for (int i = 0; i < node->childCount(); i++) {
    auto *child = node->child(i);
    if (child) {
      auto nameNode = getNodeFromPath("Name", *child);
      if (nameNode) {
        if (nameNode->text(1) == name) {
          auto baseNode = getNodeFromPath("Base", *child);
          if (baseNode) {
            return baseNode->text(1);
          }
        }
      }
    }
  }

  return "";
}
