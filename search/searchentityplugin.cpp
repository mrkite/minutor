#include "search/searchentityplugin.h"
#include "search/searchresultwidget.h"

#include "chunk.h"
#include "identifier/entityidentifier.h"
#include <set>


SearchEntityPlugin::SearchEntityPlugin()
  : QWidget()
  , layout(new QVBoxLayout(this))
{
  layout->addWidget(stw_entity   = new SearchTextWidget("entity type"));
  layout->addWidget(stw_villager = new SearchTextWidget("villager profession"));
  layout->addWidget(stw_sells    = new SearchTextWidget("sells"));
  layout->addWidget(stw_buys     = new SearchTextWidget("buys"));
  layout->addWidget(stw_special  = new SearchTextWidget("special"));

  // add suggestions for "entity type"
  const QList<QString> &knownIds = EntityIdentifier::Instance().getKnownIds();
  for (const auto& name: knownIds) {
    stw_entity->addSuggestion(name);
  }

  // add suggestions for "villager profession"
  QStringList professions;
  professions << "none" << "nitwit" << "armorer" << "butcher"
      << "cartographer" << "cleric" << "farmer" << "fisherman" << "fletcher"
      << "leatherworker" << "librarian" << "mason" << "shepherd"
      << "toolsmith" << "weaponsmith";
  for (const auto& name: qAsConst(professions)) {
    stw_villager->addSuggestion(name);
  }

}

SearchEntityPlugin::~SearchEntityPlugin()
{
  delete layout;
}

QWidget &SearchEntityPlugin::getWidget()
{
  return *this;
}

SearchPluginI::ResultListT SearchEntityPlugin::searchChunk(const Chunk &chunk)
{
  SearchPluginI::ResultListT results;

  const auto& entityMap = chunk.getEntityMap();

  for (const auto& entity: entityMap) {
    EntityEvaluator evaluator(
      EntityEvaluatorConfig(results,
                            entity,
                            std::bind(&SearchEntityPlugin::evaluateEntity, this, std::placeholders::_1)
                            )
      );
  }

  return results;
}

bool SearchEntityPlugin::evaluateEntity(EntityEvaluator &entity)
{
  bool result = true;

  if (stw_entity->active()) {
    QString id = entity.getTypeId().remove("minecraft:");
    result = result && stw_entity->matches(id);
  }

  if (stw_villager->active()) {
    QString searchFor = stw_villager->getSearchText();
    QString career = entity.getVillagerProfession();
    result = result && (career.contains(searchFor, Qt::CaseInsensitive));
  }

  if (stw_buys->active()) {
    result = result && findBuyOrSell(entity, *stw_buys, 0);
  }

  if (stw_sells->active()) {
    result = result && findBuyOrSell(entity, *stw_sells, 1);
  }

  if (stw_special->active()) {
    result = result && stw_special->matches(entity.getSpecialParams());
  }

  return result;
}

bool SearchEntityPlugin::findBuyOrSell(EntityEvaluator &entity, SearchTextWidget& searchText, int index)
{
  bool foundOffer = false;
  auto offers = entity.getVillagerOffers();
  for (const auto& offer: offers) {
    auto splitOffer = offer.split(" => ");
    foundOffer = (splitOffer.count() > index) && searchText.matches(splitOffer[index]);
    if (foundOffer) {
      break;
    }
  }

  return foundOffer;
}


