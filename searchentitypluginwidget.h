#ifndef SEARCHENTITYPLUGINWIDGET_H
#define SEARCHENTITYPLUGINWIDGET_H

#include "entityevaluator.h"
#include "searchplugininterface.h"

#include <QWidget>
#include <QLayout>

class SearchTextWidget;

class SearchEntityPluginWidget : public QWidget, public SearchPluginI
{
  Q_OBJECT

public:
  explicit SearchEntityPluginWidget();
  ~SearchEntityPluginWidget() override;

  QWidget &getWidget() override;

  SearchPluginI::ResultListT searchChunk(Chunk &chunk) override;

private:
  QLayout* layout;

  SearchTextWidget* stw_sells;
  SearchTextWidget* stw_buys;
  SearchTextWidget* stw_entityType;
  SearchTextWidget* stw_villagerType;
  SearchTextWidget* stw_special;

  bool evaluateEntity(EntityEvaluator &entity);
  bool findBuyOrSell(EntityEvaluator& entity, SearchTextWidget &searchText, int index);
};

#endif // SEARCHENTITYPLUGINWIDGET_H
