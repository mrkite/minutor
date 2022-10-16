#ifndef SEARCHENTITYPLUGIN_H
#define SEARCHENTITYPLUGIN_H

#include "search/entityevaluator.h"
#include "search/searchplugininterface.h"
#include "search/searchtextwidget.h"

#include <QWidget>
#include <QLayout>


class SearchEntityPlugin : public QWidget, public SearchPluginI
{
  Q_OBJECT

 public:
  explicit SearchEntityPlugin();
  ~SearchEntityPlugin() override;

  QWidget &getWidget() override;

  SearchPluginI::ResultListT searchChunk(const Chunk &chunk) override;

 private:
  QLayout* layout;

  SearchTextWidget* stw_entity;
  SearchTextWidget* stw_villager;
  SearchTextWidget* stw_sells;
  SearchTextWidget* stw_buys;
  SearchTextWidget* stw_special;

  bool evaluateEntity(EntityEvaluator &entity);
  bool findBuyOrSell(EntityEvaluator& entity, SearchTextWidget &searchText, int index);
};

#endif // SEARCHENTITYPLUGIN_H
