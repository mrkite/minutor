#ifndef RANGE_H
#define RANGE_H

#include <utility>
#include <limits>

template <typename ValueT>
class Range
{
public:
  Range(ValueT start_including, ValueT end_including)
    : m_begin(start_including)
    , m_end(end_including)
  {}

  ValueT begin() const {return m_begin;};
  ValueT end() const   {return m_end;};

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

  bool isInsideRange(ValueT value) const
  {
    return (value >= m_begin) && (value <= m_end);
  }
private:
  ValueT m_begin;
  ValueT m_end;
};

#endif // RANGE_H
