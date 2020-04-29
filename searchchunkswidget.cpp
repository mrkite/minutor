#include "searchchunkswidget.h"
#include "ui_searchchunkswidget.h"

#include "./chunkcache.h"
#include "./chunkmath.h"
#include "./range.h"
#include "./rectangleinnertoouteriterator.h"

#include <QVariant>
#include <QTreeWidgetItem>
#include <QtConcurrent/QtConcurrent>

SearchChunksWidget::SearchChunksWidget(QSharedPointer<SearchPluginI> searchPlugin_, QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::SearchChunksWidget)
  , searchPlugin(searchPlugin_)
{
  ui->setupUi(this);

  auto layout = new QHBoxLayout(ui->plugin_context);
  ui->plugin_context->setLayout(layout);
  layout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
  layout->addWidget(&searchPlugin->getWidget());

  qRegisterMetaType<QSharedPointer<SearchPluginI::ResultListT> >("QSharedPointer<SearchPluginI::ResultListT>");
}

SearchChunksWidget::~SearchChunksWidget()
{
  cancelSearch();
}

void SearchChunksWidget::setSearchCenter(int x, int y, int z)
{
  searchCenter = QVector3D(x,y,z);
  ui->resultList->setPointOfInterest(searchCenter);
}

static Range<float> helperRangeCreation(const QCheckBox& checkBox, const QSpinBox& sb1, const QSpinBox& sb2)
{
  if (!checkBox.isChecked())
  {
    return Range<float>::max();
  }
  else
  {
    return Range<float>::createFromUnorderedParams(sb1.value(), sb2.value());
  }
}

void SearchChunksWidget::on_pb_search_clicked()
{
  if (!currentfuture.isCanceled())
  {
    cancelSearch();
    return;
  }

  const Range<float> range_y = helperRangeCreation(*ui->check_range_y, *ui->sb_y_start, *ui->sb_y_end);
  currentSearch = QSharedPointer<AsyncSearch>::create(*this, range_y, searchPlugin);

  ui->pb_search->setText("Cancel");

  ui->resultList->clearResults();

  const int radius = 1 + (ui->sb_radius->value() / CHUNK_SIDE_LENGTH);

  const bool successfull_init = searchPlugin->initSearch();
  if (!successfull_init)
  {
    return;
  }

  const QVector2D poi = getChunkCoordinates(QVector2D(searchCenter.x(), searchCenter.z()));
  const QVector2D radius2d(radius, radius);

  QRect searchRange((poi - radius2d).toPoint(), (poi + radius2d).toPoint());

  auto chunks = QSharedPointer<QList<ChunkID> >::create();

  for (RectangleInnerToOuterIterator it(searchRange); it != it.end(); ++it)
  {
    const ChunkID id(it->x(), it->y());
    chunks->append(id);
  }

  ui->progressBar->setMaximum(chunks->size());
  ui->progressBar->setValue(0);

  currentfuture = QtConcurrent::map(*chunks, [currentSearch = currentSearch, chunks /* needed to keep list alive during search */](const ChunkID& id){
    currentSearch->loadAndSearchChunk_async(id);
  });
}

void SearchChunksWidget::AsyncSearch::loadAndSearchChunk_async(ChunkID id)
{
  auto chunk = ChunkCache::Instance().getChunkSynchronously(id);

  searchLoadedChunk_async(chunk);
}

void SearchChunksWidget::AsyncSearch::searchLoadedChunk_async(const QSharedPointer<Chunk>& chunk)
{
  QSharedPointer<SearchPluginI::ResultListT> results;

  if (chunk)
  {
    results = searchExistingChunk_async(chunk);
  }

  QMetaObject::invokeMethod(&parent, "displayResultsOfSingleChunk", Qt::QueuedConnection,
                            Q_ARG(QSharedPointer<SearchPluginI::ResultListT>, results));
}

QSharedPointer<SearchPluginI::ResultListT> SearchChunksWidget::AsyncSearch::searchExistingChunk_async(const QSharedPointer<Chunk>& chunk)
{
  ChunkID id(chunk->getChunkX(), chunk->getChunkZ());

  auto strong = searchPlugin.lock();
  if (!strong)
  {
    return QSharedPointer<SearchPluginI::ResultListT>();
  }

  auto results_tmp = strong->searchChunk(*chunk);
  auto results = QSharedPointer<SearchPluginI::ResultListT>::create();

  for (const auto& result: results_tmp)
  {
    if (range_y.isInsideRange(result.pos.y()))
    {
      results->push_back(result);
    }
  }

  return results;
}


void SearchChunksWidget::displayResultsOfSingleChunk(QSharedPointer<SearchPluginI::ResultListT> results)
{
  if (results)
  {
    for (const auto& result: *results)
    {
      ui->resultList->addResult(result);
    }
  }

  addOneToProgress();
}

void SearchChunksWidget::addOneToProgress()
{
  ui->progressBar->setValue(ui->progressBar->value() + 1);

  if (ui->progressBar->maximum() == ui->progressBar->value())
  {
    cancelSearch();
  }
}

void SearchChunksWidget::cancelSearch()
{
  currentfuture.cancel();
  currentfuture.waitForFinished();

  ui->pb_search->setText("Search");

  ui->resultList->searchDone();

  currentSearch.reset();
}

void SearchChunksWidget::on_resultList_jumpTo(const QVector3D &pos)
{
  emit jumpTo(pos);
}

void SearchChunksWidget::on_resultList_updateSearchResultPositions(QVector<QSharedPointer<OverlayItem> > item)
{
  emit updateSearchResultPositions(item);
}
