#ifndef STATISTICDIALOG_H
#define STATISTICDIALOG_H

#include <QDialog>
#include <QVector3D>
#include <QFuture>
#include <QSet>
#include <set>

#include "search/searchtextwidget.h"
#include "search/range.h"

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

  class ResultItem {
  public:
    ResultItem () : count(0), air(16*16), total(16*16) {}
    ResultItem& operator+=(const ResultItem& rhs) {
      this->count += rhs.count;
      this->air   += rhs.air;
      this->total += rhs.total;
      return *this;
    }
    friend ResultItem operator+(ResultItem lhs, const ResultItem& rhs) {
      lhs += rhs;
      return lhs;

    }
    int count;
    int air;
    int total;
  };
  typedef QMap<int,ResultItem> t_result;

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

    t_result processChunk_async(const ChunkID &id);

    static void reduceResults(t_result &result, const t_result &intermediate);

   private:
    StatisticDialog& parent;
    const Range<int> range_y;
    QList<quint16>   block_hid;
  };

  QSharedPointer<AsyncStatistic> currentStatistic;
  QFuture<t_result>              currentFuture;
  ResultItem                     resultSum;
};

#endif // STATISTICDIALOG_H
