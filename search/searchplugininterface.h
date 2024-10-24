#ifndef SEARCHPLUGININTERFACE_H
#define SEARCHPLUGININTERFACE_H

#include "search/range.h"
#include "search/searchresultitem.h"
#include "chunk.h"

#include <vector>


class QWidget;

class SearchPluginI
{
 public:
  using ResultListT = std::vector<SearchResultItem>;


  virtual QWidget& getWidget() = 0;

  virtual bool initSearch() { return true; }

  virtual ResultListT searchChunk(const Chunk &chunk, const Range<int> &range) = 0;
  ResultListT searchChunk(const Chunk &chunk)
  {
    Range<int> search_range(chunk.getLowest(), chunk.getHighest());
    return searchChunk(chunk, search_range);
  }

  virtual ~SearchPluginI() {}
};

#endif // SEARCHPLUGININTERFACE_H
