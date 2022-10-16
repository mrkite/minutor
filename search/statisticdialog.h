#ifndef STATISTICDIALOG_H
#define STATISTICDIALOG_H

#include <QDialog>
#include <QVector3D>
#include <QFuture>
#include <QSet>
#include <set>

#include "search/searchtextwidget.h"
#include "search/range.h"
#include "search/statisticresultitem.h"

#include "chunkid.h"
#include "chunk.h"


namespace Ui {
class StatisticDialog;
}

class StatisticDialog : public QDialog
{
  Q_OBJECT

 public:
  explicit StatisticDialog(QWidget *parent = nullptr);
  ~StatisticDialog();

 public slots:
  void setRangeY(int minimum, int maximum);
  void setSearchCenter(const QVector3D& centerPoint);
  void setSearchCenter(int x, int y, int z);

 private slots:
  void on_pb_search_clicked();
  void on_pb_save_clicked();

  void updateStatusText();
  void updateProgress();

  void clearResults();
  void finishSearch();
  void cancelSearch();
  void updateResultImage();

 private:
  Ui::StatisticDialog *ui;
  SearchTextWidget    *stw_blockName;
  QVector3D           searchCenter;
  QPixmap             result_image;
  quint16             air_hid;

  class AsyncStatistic
  {
   public:
    AsyncStatistic(StatisticDialog  &parent_,
                   const Range<int> &range_y_,
                   const QList<quint16> &hid_)
      : parent(parent_)
      , range_y(range_y_)
      , block_hid(hid_)
    {}

    StatisticResultMap processChunk_async(const ChunkID &id);

    static void reduceResults(StatisticResultMap &result, const StatisticResultMap &intermediate);

   private:
    StatisticDialog& parent;
    const Range<int> range_y;
    QList<quint16>   block_hid;
  };

  QSharedPointer<AsyncStatistic> currentStatistic;
  QFuture<StatisticResultMap>    currentFuture;
  StatisticResultItem            resultSum;
};

#endif // STATISTICDIALOG_H
