#include "search/searchchunksdialog.h"
#include "ui_searchchunksdialog.h"

#include "chunkcache.h"
#include "search/range.h"
#include "search/rectangleinnertoouteriterator.h"

#include <QVariant>
#include <QVector2D>
#include <QTreeWidgetItem>
#include <QtConcurrent/QtConcurrent>


SearchChunksDialog::SearchChunksDialog(QSharedPointer<SearchPluginI> searchPlugin_, QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::SearchChunksDialog)
  , searchPlugin(searchPlugin_)
{
  ui->setupUi(this);
  ui->layout_plugin->addWidget(&searchPlugin->getWidget());

  qRegisterMetaType<QSharedPointer<SearchPluginI::ResultListT> >("QSharedPointer<SearchPluginI::ResultListT>");
}

SearchChunksDialog::~SearchChunksDialog()
{
  cancelSearch();
}

void SearchChunksDialog::setRangeY(int minimum, int maximum)
{
  ui->range->setRangeY(Range<int>(minimum, maximum));
}

void SearchChunksDialog::setSearchCenter(const QVector3D &centerPoint)
{
  searchCenter = centerPoint;
  ui->resultList->setPointOfInterest(searchCenter);
}

void SearchChunksDialog::setSearchCenter(int x, int y, int z)
{
  setSearchCenter(QVector3D(x,y,z));
}


// auto-magically connected via name match
void SearchChunksDialog::on_pb_search_clicked()
{
  // when search is pending -> cancel and return
  if (!currentfuture.isCanceled()) {
    cancelSearch();
    return;
  }

  const bool successfull_init = searchPlugin->initSearch();
  if (!successfull_init)
    return;

  // prepare UI
  ui->range->setButtonText("Cancel");
  ui->resultList->clearResults();

  // determine Chunks to be searched
  auto chunks = QSharedPointer<QList<ChunkID> >::create();
  const int radius = ui->range->getRadiusChunks();
  for (RectangleInnerToOuterIterator it(searchCenter, radius); it != it.end(); ++it) {
    const ChunkID id(it->x(), it->y());
    chunks->append(id);
  }

  ui->range->setProgressMaximum(chunks->size());
  ui->range->setProgressValue(0);

  // start search
  const Range<int> range_y = ui->range->getRangeY();
  currentSearch = QSharedPointer<AsyncSearch>::create(*this, range_y, searchPlugin);

  currentfuture = QtConcurrent::map(*chunks, [currentSearch = currentSearch, chunks /* needed to keep list alive during search */](const ChunkID& id){
    currentSearch->loadChunk_async(id);
  });
}



void SearchChunksDialog::displayResultsOfSingleChunk(QSharedPointer<SearchPluginI::ResultListT> results)
{
  if (results) {
    for (const auto& result: *results) {
      ui->resultList->addResult(result);
    }
  }

  addOneToProgress();
}

void SearchChunksDialog::addOneToProgress()
{
  if (ui->range->incrementProgressValue()) {
    cancelSearch(); // finished
  }
}

void SearchChunksDialog::cancelSearch()
{
  currentfuture.cancel();
  currentfuture.waitForFinished();
  currentSearch.reset();

  ui->resultList->searchDone();
  ui->range->setButtonText("Search");
}

void SearchChunksDialog::on_resultList_jumpTo(const QVector3D &pos)
{
  emit jumpTo(pos);
}

void SearchChunksDialog::on_resultList_updateSearchResultPositions(QVector<QSharedPointer<OverlayItem> > item)
{
  emit updateSearchResultPositions(item);
}


// AsyncSearch

void SearchChunksDialog::AsyncSearch::loadChunk_async(ChunkID id)
{
  QSharedPointer<Chunk> chunk = ChunkCache::Instance().getChunkSynchronously(id);

  QSharedPointer<SearchPluginI::ResultListT> results;

  if (chunk) {
    results = processChunk_async(chunk);
  }

  QMetaObject::invokeMethod(&parent, "displayResultsOfSingleChunk", Qt::QueuedConnection,
                            Q_ARG(QSharedPointer<SearchPluginI::ResultListT>, results));
}


QSharedPointer<SearchPluginI::ResultListT> SearchChunksDialog::AsyncSearch::processChunk_async(const QSharedPointer<Chunk>& chunk)
{
  ChunkID id(chunk->getChunkX(), chunk->getChunkZ());

  auto strong = searchPlugin.lock();
  if (!strong) {
    return QSharedPointer<SearchPluginI::ResultListT>();
  }

  auto results = strong->searchChunk(*chunk, range_y);
  return QSharedPointer<SearchPluginI::ResultListT>::create(results);
}
