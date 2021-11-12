#include "searchentitypluginwidget.h"

//#include "./careeridentifier.h"
#include "chunk.h"
#include "searchresultwidget.h"
#include "searchtextwidget.h"

SearchEntityPluginWidget::SearchEntityPluginWidget()
  : QWidget()
  , layout(new QVBoxLayout(this))
{
  layout->addWidget(stw_sells = new SearchTextWidget("sells"));
  layout->addWidget(stw_buys = new SearchTextWidget("buys"));
  layout->addWidget(stw_entityType = new SearchTextWidget("entity type"));
  layout->addWidget(stw_villagerType = new SearchTextWidget("villager type"));
  layout->addWidget(stw_special = new SearchTextWidget("special"));
}

SearchEntityPluginWidget::~SearchEntityPluginWidget()
{
  delete layout;
}

QWidget &SearchEntityPluginWidget::getWidget()
{
  return *this;
}

SearchPluginI::ResultListT SearchEntityPluginWidget::searchChunk(Chunk &chunk)
{
  SearchPluginI::ResultListT results;

  const auto& entityMap = chunk.getEntityMap();

  for(const auto& entity: entityMap)
  {
    EntityEvaluator evaluator(
      EntityEvaluatorConfig(results,
                            entity,
                            std::bind(&SearchEntityPluginWidget::evaluateEntity, this, std::placeholders::_1)
                            )
      );
  }

  return results;
}

bool SearchEntityPluginWidget::evaluateEntity(EntityEvaluator &entity)
{
  bool result = true;

  if (stw_villagerType->isActive())
  {
    QString searchFor = stw_villagerType->getSearchText();
    QString career = entity.getCareerName();
    result = result && (career.contains(searchFor, Qt::CaseInsensitive));
  }

  if (stw_buys->isActive())
  {
    result = result && findBuyOrSell(entity, *stw_buys, 0);
  }

  if (stw_sells->isActive())
  {
    result = result && findBuyOrSell(entity, *stw_sells, 1);
  }

  if (stw_entityType->isActive())
  {
    QString id = entity.getTypeId();
    result = result && stw_entityType->matches(id);
  }

  if (stw_special->isActive())
  {
    result = result && stw_special->matches(entity.getSpecialParams());
  }

  return result;
}

bool SearchEntityPluginWidget::findBuyOrSell(EntityEvaluator &entity, SearchTextWidget& searchText, int index)
{
  bool foundBuy = false;
  auto offers = entity.getOffers();
  for (const auto& offer: offers)
  {
    auto splitOffer = offer.split(" => ");
    foundBuy = (splitOffer.count() > index) && searchText.matches(splitOffer[index]);
    if (foundBuy)
    {
      break;
    }
  }

  return foundBuy;
}


