#include "search/searchblockpluginwidget.h"
#include "search/searchresultwidget.h"

#include "chunk.h"
#include "identifier/blockidentifier.h"

#include <algorithm>

SearchBlockPluginWidget::SearchBlockPluginWidget(QWidget* parent)
  : QWidget(parent)
  , layout(new QVBoxLayout(this))
{

  layout->addWidget(stw_blockId   = new SearchTextWidget("block id"));
  layout->addWidget(stw_blockName = new SearchTextWidget("block name"));

  // add suggestions for "block name"
  auto idList = BlockIdentifier::Instance().getKnownIds();

  std::set<QString> nameList;   // std::set<> is sorted, QSet<> not

  for (const auto& id: idList) {
    auto blockInfo = BlockIdentifier::Instance().getBlockInfo(id);
    if (blockInfo.getName() == "minecraft:air") continue;
    if (blockInfo.getName() == "minecraft:cave_air") continue;
    nameList.insert(blockInfo.getName());
  }

  for (auto name: nameList) {
    stw_blockName->addSuggestion(name);
  }
}

SearchBlockPluginWidget::~SearchBlockPluginWidget()
{
  delete layout;
}

QWidget &SearchBlockPluginWidget::getWidget()
{
  return *this;
}

bool SearchBlockPluginWidget::initSearch()
{
  m_searchForIds.clear();

  if (stw_blockId->isActive()) {
    bool ok = true;
    m_searchForIds.insert(stw_blockId->getSearchText().toUInt());
    if (!ok) {
      return false;
    }
  }

  if (stw_blockName->isActive()) {
    auto idList = BlockIdentifier::Instance().getKnownIds();
    for (auto id: idList) {
      auto blockInfo = BlockIdentifier::Instance().getBlockInfo(id);
      if (stw_blockName->matches(blockInfo.getName())) {
        m_searchForIds.insert(id);
      }
    }
  }

  return (m_searchForIds.size() > 0);
}

SearchPluginI::ResultListT SearchBlockPluginWidget::searchChunk(const Chunk &chunk)
{
  SearchPluginI::ResultListT results;

  if (m_searchForIds.size() == 0) {
    return results;
  }

  for (int z = 0; z < 16; z++) {
    for (int y = chunk.getLowest(); y < chunk.getHighest() ; y++) {
      for (int x = 0; x < 16; x++) {
        const uint blockHid = chunk.getBlockHID(x,y,z);
        const auto it = m_searchForIds.find(blockHid);
        if (it != m_searchForIds.end()) {
          auto info = BlockIdentifier::Instance().getBlockInfo(blockHid);

          SearchResultItem item;
          item.name = info.getName();
          item.pos = QVector3D(chunk.getChunkX() * 16 + x, y, chunk.getChunkZ() * 16 + z) + QVector3D(0.5,0.0,0.5); // mark center of block, not origin
          item.entity = QSharedPointer<Entity>::create(OverlayItem::Point(item.pos));
          results.push_back(item);
        }
      }
    }
  }

  return results;
}
