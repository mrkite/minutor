#ifndef SEARCHBLOCKPLUGINWIDGET_H
#define SEARCHBLOCKPLUGINWIDGET_H

#include "search/searchplugininterface.h"
#include "search/searchtextwidget.h"

#include <QWidget>
#include <QLayout>
#include <set>


class SearchBlockPluginWidget : public QWidget, public SearchPluginI
{
  Q_OBJECT

 public:
  explicit SearchBlockPluginWidget(QWidget* parent = nullptr);
  ~SearchBlockPluginWidget();

  QWidget &getWidget() override;
  bool initSearch() override;
  SearchPluginI::ResultListT searchChunk(const Chunk &chunk) override;

 private:
  QLayout* layout;

  SearchTextWidget* stw_blockId;
  SearchTextWidget* stw_blockName;

  std::set<quint32> m_searchForIds;
};

#endif // SEARCHBLOCKPLUGINWIDGET_H
