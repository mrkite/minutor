#ifndef SEARCHENTITYWIDGET_H
#define SEARCHENTITYWIDGET_H

#include "overlay/overlayitem.h"
#include "overlay/propertietreecreator.h"
#include "search/entityevaluator.h"
#include "search/range.h"
#include "search/searchplugininterface.h"
#include "chunkid.h"

#include <QDialog>
#include <QFuture>
#include <QVector3D>

#include <set>

namespace Ui {
class SearchChunksWidget;
}

class ChunkCache;
class Chunk;
class SearchResultWidget;

class SearchChunksWidget : public QDialog
{
  Q_OBJECT

 public:
  explicit SearchChunksWidget(QSharedPointer<SearchPluginI> searchPlugin, QWidget *parent = nullptr);
  ~SearchChunksWidget();

 public slots:
  void setSearchCenter(int x, int y, int z);

 signals:
  void jumpTo(QVector3D pos);
  void updateSearchResultPositions(QVector<QSharedPointer<OverlayItem> >);

 private slots:
  void on_pb_search_clicked();

  void on_resultList_jumpTo(const QVector3D &);
  void on_resultList_updateSearchResultPositions(QVector<QSharedPointer<OverlayItem> >);

  void displayResultsOfSingleChunk(QSharedPointer<SearchPluginI::ResultListT> results);

 private:
  QSharedPointer<Ui::SearchChunksWidget> ui;
  QSharedPointer<SearchPluginI> searchPlugin;
  QVector3D searchCenter;

  class AsyncSearch;
  QSharedPointer<AsyncSearch> currentSearch;
  QFuture<void> currentfuture;

  class AsyncSearch
  {
   public:
    AsyncSearch(SearchChunksWidget& parent_,
                const Range<float>& range_y_,
                const QWeakPointer<SearchPluginI>& searchPlugin_)
      : parent(parent_)
      , range_y(range_y_)
      , searchPlugin(searchPlugin_)
    {}

    void loadAndSearchChunk_async(ChunkID id);

    void searchLoadedChunk_async(const QSharedPointer<Chunk> &chunk);

    QSharedPointer<SearchPluginI::ResultListT> searchExistingChunk_async(const QSharedPointer<Chunk> &chunk);

   private:
    SearchChunksWidget& parent;
    const Range<float> range_y;
    QWeakPointer<SearchPluginI> searchPlugin;
  };

  void addOneToProgress();

  void cancelSearch();
};

#endif // SEARCHENTITYWIDGET_H
