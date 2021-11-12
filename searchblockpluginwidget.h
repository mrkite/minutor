#ifndef SEARCHBLOCKPLUGINWIDGET_H
#define SEARCHBLOCKPLUGINWIDGET_H

#include "searchplugininterface.h"
#include "blockidentifier.h"

#include <QWidget>
#include <QLayout>
#include <set>

class SearchTextWidget;

class SearchBlockPluginWidget : public QWidget, public SearchPluginI
{
  Q_OBJECT

public:
  explicit SearchBlockPluginWidget(QWidget* parent = nullptr);
  ~SearchBlockPluginWidget();

  QWidget &getWidget() override;
  bool initSearch() override;
  SearchPluginI::ResultListT searchChunk(Chunk &chunk) override;

private:
  QLayout* layout;

  SearchTextWidget* stw_blockId;
  SearchTextWidget* stw_blockName;

  std::set<quint32> m_searchForIds;
};

#endif // SEARCHBLOCKPLUGINWIDGET_H
