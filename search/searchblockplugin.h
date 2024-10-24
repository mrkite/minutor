#ifndef SEARCHBLOCKPLUGIN_H
#define SEARCHBLOCKPLUGIN_H

#include "search/searchplugininterface.h"
#include "search/searchtextwidget.h"

#include <QWidget>
#include <QLayout>
#include <set>


class SearchBlockPlugin : public QWidget, public SearchPluginI
{
  Q_OBJECT

 public:
  explicit SearchBlockPlugin(QWidget* parent = nullptr);
  ~SearchBlockPlugin();

  QWidget &getWidget() override;

  bool    initSearch() override;
  SearchPluginI::ResultListT searchChunk(const Chunk &chunk, const Range<int> &range) override;

 private:
  QLayout* layout;

  SearchTextWidget* stw_blockId;
  SearchTextWidget* stw_blockName;

  std::set<quint32> m_searchForIds;
};

#endif // SEARCHBLOCKPLUGIN_H
