#ifndef SEARCHENTITYPLUGINWIDGET_H
#define SEARCHENTITYPLUGINWIDGET_H

#include "entityevaluator.h"
#include "searchplugininterface.h"
#include "searchtextwidget.h"

#include <QWidget>
#include <QLayout>


class SearchEntityPluginWidget : public QWidget, public SearchPluginI
{
  Q_OBJECT

 public:
  explicit SearchEntityPluginWidget();
  ~SearchEntityPluginWidget() override;

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

#endif // SEARCHENTITYPLUGINWIDGET_H
