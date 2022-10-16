#ifndef STATISTICLABEL_H
#define STATISTICLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QToolTip>

#include "search/statisticresultitem.h"


class StatisticLabel : public QLabel
{
  Q_OBJECT

 public:
  explicit StatisticLabel(QWidget *parent = nullptr) : QLabel(parent) {};

 public slots:
  void setResultMap(const StatisticResultMap &results, int max_key) { m_results = results; m_max_key = max_key;}

  void mouseMoveEvent(QMouseEvent *event) {
    int y = m_max_key - event->pos().y();
    const StatisticResultItem & result = m_results[y];

    QToolTip::showText(event->globalPos(),
                       "Y:" + QString::number( y ) + " -> " + QString::number( result.count ),
                       this, rect() );
    QWidget::mouseMoveEvent(event);
  }

 private:
  StatisticResultMap  m_results;
  int                 m_max_key;
};

#endif // STATISTICLABEL_H
