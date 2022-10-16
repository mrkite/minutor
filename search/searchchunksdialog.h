#ifndef SEARCHENTITYDIALOG_H
#define SEARCHENTITYDIALOG_H

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

#include "ui_searchchunksdialog.h"


class ChunkCache;
class Chunk;
class SearchResultWidget;

class SearchChunksDialog : public QDialog
{
  Q_OBJECT

 public:
  explicit SearchChunksDialog(QSharedPointer<SearchPluginI> searchPlugin, QWidget *parent = nullptr);
  ~SearchChunksDialog();

 public slots:
  void setRangeY(int minimum, int maximum);
  void setSearchCenter(const QVector3D &centerPoint);
  void setSearchCenter(int x, int y, int z);

 signals:
  void jumpTo(QVector3D pos);
  void updateSearchResultPositions(QVector<QSharedPointer<OverlayItem> >);

 private slots:
  void on_pb_search_clicked();

  void on_resultList_jumpTo(const QVector3D &);
  void on_resultList_updateSearchResultPositions(QVector<QSharedPointer<OverlayItem> >);

  void displayResultsOfSingleChunk(QSharedPointer<SearchPluginI::ResultListT> results);

  void addOneToProgress();

  void cancelSearch();

 private:
  Ui::SearchChunksDialog *ui;
  QSharedPointer<SearchPluginI> searchPlugin;
  QVector3D searchCenter;

  class AsyncSearch
  {
   public:
    AsyncSearch(SearchChunksDialog& parent_,
                const Range<int>& range_y_,
                const QWeakPointer<SearchPluginI>& searchPlugin_)
      : parent(parent_)
      , range_y(range_y_)
      , searchPlugin(searchPlugin_)
    {}

    void loadChunk_async(ChunkID id);

    QSharedPointer<SearchPluginI::ResultListT> processChunk_async(const QSharedPointer<Chunk> &chunk);

   private:
    SearchChunksDialog& parent;
    const Range<int> range_y;
    QWeakPointer<SearchPluginI> searchPlugin;
  };

  QSharedPointer<AsyncSearch> currentSearch;
  QFuture<void> currentfuture;
};

#endif // SEARCHENTITYDIALOG_H
