#ifndef STATISTICRESULTITEM_H
#define STATISTICRESULTITEM_H

#include <QMap>


class StatisticResultItem {
 public:
  StatisticResultItem () : count(0), air(16*16), total(16*16) {}

  StatisticResultItem& operator+=(const StatisticResultItem& rhs) {
    this->count += rhs.count;
    this->air   += rhs.air;
    this->total += rhs.total;
    return *this;
  }

  friend StatisticResultItem operator+(StatisticResultItem lhs, const StatisticResultItem& rhs) {
    lhs += rhs;
    return lhs;
  }

  int count;
  int air;
  int total;
};

typedef QMap<int,StatisticResultItem> StatisticResultMap;


#endif // STATISTICRESULTITEM_H
