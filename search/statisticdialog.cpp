/** Copyright (c) 2022, EtlamGit */

#include <algorithm>
#include <iostream>
#include <fstream>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>

#include "statisticdialog.h"
#include "ui_statisticdialog.h"

#include "chunkcache.h"
#include "identifier/blockidentifier.h"
#include "search/rectangleinnertoouteriterator.h"


StatisticDialog::StatisticDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::StatisticDialog)
{
  ui->setupUi(this);
  ui->layout_select->addWidget(stw_blockName = new SearchTextWidget("block name"));
  stw_blockName->setActive(true);
  stw_blockName->hideActive(true);
  stw_blockName->setExactMatch(true);

  // add suggestions for "block name"
  auto idList = BlockIdentifier::Instance().getKnownIds();

  std::set<QString> nameList;   // std::set<> is sorted, QSet<> not

  for (const auto hid: idList) {
    auto blockInfo = BlockIdentifier::Instance().getBlockInfo(hid);
    if (blockInfo.getName() == "minecraft:air") { air_hid = hid; continue; }
    if (blockInfo.getName() == "minecraft:cave_air") continue;
    nameList.insert(blockInfo.getName());
  }

  for (auto name: nameList) {
    stw_blockName->addSuggestion(name);
  }

  setFixedSize(sizeHint());
}

StatisticDialog::~StatisticDialog()
{
  delete ui;
}


void StatisticDialog::setRangeY(int minimum, int maximum)
{
  ui->range->setRangeY(Range<int>(minimum, maximum));
}


void StatisticDialog::setSearchCenter(const QVector3D &centerPoint)
{
  searchCenter = centerPoint;
  updateStatusText();
}

void StatisticDialog::setSearchCenter(int x, int y, int z)
{
  setSearchCenter(QVector3D(x,y,z));
}


// auto-magically connected via name match
void StatisticDialog::on_pb_search_clicked() {
  // when search is pending -> cancel and return
  if (!currentFuture.isFinished() && !currentFuture.isCanceled()) {
    cancelSearch();
    return;
  }

  // determine Chunks to be searched
  auto chunks = QSharedPointer< QList<ChunkID> >::create();
  const int radius = ui->range->getRadiusChunks();
  for (RectangleInnerToOuterIterator it(searchCenter, radius); it != it.end(); ++it) {
    const ChunkID id(it->x(), it->y());
    chunks->append(id);
  }

  // prepare UI
  ui->range->setButtonText("Cancel");
  ui->range->setProgressMaximum(chunks->size());
  ui->range->setProgressValue(0);
  clearResults();

  // setup search parameters
  const Range<int>   rangeY  = ui->range->getRangeY();

  // get HID for selected "block name"
  quint16 blockHID = 0;
  for (const auto hid: BlockIdentifier::Instance().getKnownIds()) {
    auto blockInfo = BlockIdentifier::Instance().getBlockInfo(hid);
    if (blockInfo.getName() == stw_blockName->getSearchText()) blockHID = hid;
  }

  currentStatistic = QSharedPointer<AsyncStatistic>::create(*this, rangeY, blockHID);

  std::function<t_result(const ChunkID &)> mapper =
      [currentStatistic = currentStatistic, chunks /* needed to keep list alive during search */]
      (const ChunkID &id) -> t_result
      { return currentStatistic->processChunk_async(id); };

  currentFuture = QtConcurrent::mappedReduced(
        *chunks,
        mapper,
        StatisticDialog::AsyncStatistic::reduceResults
  );
}


// auto-magically connected via name match
void StatisticDialog::on_pb_save_clicked() {
  if (currentFuture.isFinished()) {
    const auto &resultList = currentFuture.results();
    if (resultList.size() == 1) {
      const auto &resultMap  = resultList[0];

      // get filename to save to
      QFileDialog fileDialog(this);
      fileDialog.setDefaultSuffix("csv");
      QString filename = fileDialog.getSaveFileName(this, tr("Save statistic results into file"),
                                                    QString(), "Comma Separated Value (*.csv)");
      // check if filename was given
      if (filename.isEmpty())
        return;

      // add .csv suffix if not present
      QFile file(filename);
      QFileInfo fileinfo(file);
      if (fileinfo.suffix().isEmpty()) {
        filename.append(".csv");
      }

      // write
      std::ofstream myfile(filename.toStdString());
      if (myfile.is_open()) {
        // CSV header
        myfile << "Y;count;air;total" << std::endl;
        // results in reverse order
        QMapIterator<int, ResultItem> key(resultMap);
        key.toBack();
        while (key.hasPrevious()) {
          key.previous();
          const ResultItem &result = resultMap.value(key.key());
          myfile << key.key() << ";"
                 << result.count << ";"
                 << result.air << ";"
                 << result.total << std::endl;
        }
        myfile.close();
      }
    }
  }
}


void StatisticDialog::updateStatusText()
{
  updateResultImage();
  if (currentFuture.isFinished() && currentFuture.results().size() == 1) {
    ui->label_result->setText(QString("%4 Blocks found around position: %1,%2,%3")
                                .arg(searchCenter.x())
                                .arg(searchCenter.y())
                                .arg(searchCenter.z())
                                .arg(resultSum.count));
  } else
    ui->label_result->setText(QString("Statistic around position: %1,%2,%3")
                                .arg(searchCenter.x())
                                .arg(searchCenter.y())
                                .arg(searchCenter.z()));
}


void StatisticDialog::updateProgress() {
  if (ui->range->incrementProgressValue()) {
    finishSearch(); // finished
  }
}


void StatisticDialog::clearResults()
{
  ui->label_graph->clear();
  setFixedSize(sizeHint());
}


// search is finished
void StatisticDialog::finishSearch() {
  currentFuture.waitForFinished();
  currentStatistic.reset();

  // update search status
  updateStatusText();

  ui->range->setButtonText("Search");
}


// search is cancelled
void StatisticDialog::cancelSearch() {
  if (!currentFuture.isFinished()) {
    currentFuture.cancel();
  }
  finishSearch();
}


void StatisticDialog::updateResultImage()
{
  // calculate total number
  resultSum.count = 0;
  resultSum.air   = 0;
  resultSum.total = 0;

  if (currentFuture.isFinished()) {
    const auto &resultList = currentFuture.results();
    if (resultList.size() == 1) {
      const auto &resultMap  = resultList[0];

      // get overall values
      int min_key   = ui->range->getRangeY().end();
      int max_key   = ui->range->getRangeY().begin();
      int max_count = 0;
      for (auto key: resultMap.keys()) {
        const ResultItem &result = resultMap.value(key);
        resultSum += result;
        if (result.count > 0) {
          min_key   = std::min<int>(min_key, key);
          max_key   = std::max<int>(max_key, key);
        }
        max_count = std::max<int>(max_count, result.count);
      }

      // prepare image
      int width  = ui->label_graph->width();
      int height = ui->range->getRangeY().end() - ui->range->getRangeY().begin() + 1;
      QImage image = QPixmap(width, height).toImage();
      for (auto key: resultMap.keys()) {
        int y = (height-1) - key + ui->range->getRangeY().begin();
        const ResultItem &result = resultMap.value(key);
        float scaleR = float(result.count) / float(max_count);
        float scaleW = 1.0 / float(width);
        for (int x = 0; x < width; x++) {
          QColor color;
          if (x*scaleW < scaleR) {
            color = QColor(64,192,64);
          } else {
            color = QColor(128,128,128);
          }
          image.setPixelColor(x, y, color);
        }
      }
      result_image = QPixmap::fromImage(image);
      ui->label_graph->setPixmap(result_image);
      setFixedSize(sizeHint());
    }
  }
}


//-------------------------------------------------------------------------------------------------
// AsyncStatistic

StatisticDialog::t_result StatisticDialog::AsyncStatistic::processChunk_async(const ChunkID &id)
{
  t_result results;

  QSharedPointer<Chunk> chunk = ChunkCache::Instance().getChunkSynchronously(id);
  if (chunk) {
    for (auto y = range_y.begin(); y <= range_y.end(); y++) {
      ResultItem ri; // init to empty Chunk layer
      const ChunkSection * const section = chunk->getSectionByY(y);
      if (section) {
        int offset = (y & 0x0f) * (16*16);
        for (int z = 0; z < 16; z++) {  // n->s
          for (int x = 0; x < 16; x++, offset++) {  // e->w
            quint16 hid = section->getPaletteEntry(offset).hid;
            if (hid != parent.air_hid) ri.air--;
            if (hid == block_hid)      ri.count++;
          }
        }
      }
      results[y] = ri;
    }
  }

  QMetaObject::invokeMethod(&parent, "updateProgress", Qt::QueuedConnection);

  return results;
}

void StatisticDialog::AsyncStatistic::reduceResults(t_result &result, const t_result &intermediate)
{
  for (auto key: intermediate.keys()) {
    if (result.contains(key)) {
      // combine
      result[key] += intermediate[key];
    } else {
      // just move over
      result[key] = intermediate[key];
    }
  }
}
