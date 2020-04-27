#ifndef RANGE_H
#define RANGE_H

#include <utility>
#include <limits>

template <typename ValueT>
class Range
{
  public:
  Range(ValueT start_including, ValueT end_including)
    : start(start_including)
    , end(end_including)
  {}

  static Range createFromUnorderedParams(ValueT v1, ValueT v2)
  {
    if (v1 > v2)
    {
      std::swap(v1, v2);
    }

    return Range(v1, v2);
  }

  static Range max()
  {
    return Range(std::numeric_limits<ValueT>::lowest(), std::numeric_limits<ValueT>::max());
  }

  const ValueT start;
  const ValueT end;

  bool isInsideRange(ValueT value) const
  {
    return (value >= start) && (value <= end);
  }
};

#endif // RANGE_H
