#include "search/searchblockplugin.h"
#include "search/searchresultitem.h"

#include "chunk.h"
#include "identifier/blockidentifier.h"

#include <algorithm>

SearchBlockPlugin::SearchBlockPlugin(QWidget* parent)
  : QWidget(parent)
  , layout(new QVBoxLayout(this))
{

  layout->addWidget(stw_blockId   = new SearchTextWidget("block id"));
  layout->addWidget(stw_blockName = new SearchTextWidget("block name"));
  stw_blockName->setActive(true);
  //stw_blockName->hideActive(true);
  stw_blockName->setExactMatch(true);

  // add suggestions for "block name"
  auto idList = BlockIdentifier::Instance().getKnownIds();

  std::set<QString> nameList;   // std::set<> is sorted, QSet<> not

  for (const auto& id: qAsConst(idList)) {
    auto blockInfo = BlockIdentifier::Instance().getBlockInfo(id);
    if (blockInfo.getName() == "minecraft:air") continue;
    if (blockInfo.getName() == "minecraft:cave_air") continue;
    nameList.insert(blockInfo.getName());
  }

  for (const auto& name: nameList) {
    stw_blockName->addSuggestion(name);
  }
}

SearchBlockPlugin::~SearchBlockPlugin()
{
  delete layout;
}

QWidget &SearchBlockPlugin::getWidget()
{
  return *this;
}

bool SearchBlockPlugin::initSearch()
{
  m_searchForIds.clear();

  if (stw_blockId->active()) {
    bool ok = true;
    m_searchForIds.insert(stw_blockId->getSearchText().toUInt());
    if (!ok) {
      return false;
    }
  }

  if (stw_blockName->active()) {
    const QList<quint32> &knownIds = BlockIdentifier::Instance().getKnownIds();
    for (quint32 hid: knownIds) {
      auto blockInfo = BlockIdentifier::Instance().getBlockInfo(hid);
      if (stw_blockName->matches(blockInfo.getName())) {
        m_searchForIds.insert(hid);
      }
    }
  }

  return (m_searchForIds.size() > 0);
}

SearchPluginI::ResultListT SearchBlockPlugin::searchChunk(const Chunk &chunk, const Range<int> &range)
{
  SearchPluginI::ResultListT results;

  if (m_searchForIds.size() == 0) {
    return results;
  }

  // determine tight search range
  int range_start = std::max<int>(chunk.getLowest(), range.begin());
  int range_stop  = std::min<int>(chunk.getHighest(), range.end());

  for (int y = range_start; y <= range_stop; y++) {
    int offset = (y & 0x0f) * (16*16);
    const ChunkSection * const section = chunk.getSectionByY(y);
    if (section) {
      for (int z = 0; z < 16; z++) {
        for (int x = 0; x < 16; x++, offset++) {
          quint32 blockHid = section->getPaletteEntry(offset).hid;
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
  }

  return results;
}
