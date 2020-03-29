#ifndef SEARCHENTITYPLUGINWIDGET_H
#define SEARCHENTITYPLUGINWIDGET_H

#include "entityevaluator.h"
#include "searchplugininterface.h"

#include <QWidget>

class SearchTextWidget;

namespace Ui {
class SearchEntityPluginWidget;
}

class SearchEntityPluginWidget : public QWidget, public SearchPluginI
{
  Q_OBJECT

public:
  explicit SearchEntityPluginWidget();
  ~SearchEntityPluginWidget() override;

  QWidget &getWidget() override;

  SearchPluginI::ResultListT searchChunk(Chunk &chunk) override;

private:
  Ui::SearchEntityPluginWidget *ui;

  SearchTextWidget* stw_sells;
  SearchTextWidget* stw_buys;
  SearchTextWidget* stw_entityType;
  SearchTextWidget* stw_villagerType;
  SearchTextWidget* stw_special;

  bool evaluateEntity(EntityEvaluator &entity);
  bool findBuyOrSell(EntityEvaluator& entity, SearchTextWidget &searchText, int index);
};

#endif // SEARCHENTITYPLUGINWIDGET_H
