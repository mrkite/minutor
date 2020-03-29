#ifndef RECTANGLEINNERTOOUTERITERATOR_H
#define RECTANGLEINNERTOOUTERITERATOR_H

#include <QRect>
#include <QPoint>

class RectangleInnerToOuterIterator
{
public:
  RectangleInnerToOuterIterator(const QRect& rect_)
    : rect(rect_)
    , center(rect_.center())
    , currentPos(0,0)
    , shellNr(0)
    , subNr(0)
    , stepNr(0)
  {
    reset();
  }

  void reset()
  {
    currentPos = QPoint(center.x(), center.y());
    shellNr = 1;
    subNr = 1;
    stepNr = 0;
  }

  RectangleInnerToOuterIterator end()
  {
    RectangleInnerToOuterIterator it(*this);
    it.stepNr = rect.size().width() * rect.size().height();
    return it;
  }

  RectangleInnerToOuterIterator& operator++()
  {
    stepNr++;

    do {
      inc();
      getCoordinate();
    } while(((*this) != end()) && !rect.contains(QPoint(currentPos.x(), currentPos.y())));

    return (*this);
  }

  const QPoint& operator*() const
  {
    return currentPos;
  }

  const QPoint* operator->() const
  {
    return &currentPos;
  }

  bool operator==(const RectangleInnerToOuterIterator& other) const
  {
    return (stepNr == other.stepNr);
  }

  bool operator!=(const RectangleInnerToOuterIterator& other) const
  {
    return !operator==(other);
  }

  const QRect& currentRect() const
  {
    return rect;
  }

private:
  QRect rect;
  QPoint center;
  QPoint currentPos;
  int shellNr;
  int subNr;
  int stepNr;

  void inc()
  {
    subNr++;
    const int lastValidSub = (((shellNr-1) * 2) + 1);
    if (subNr > lastValidSub)
    {
      shellNr++;
      subNr=1;
    }
  }

  void getCoordinate()
  {
    const int shellSign   = ((shellNr & 0x1) != 0) ? -1 : 1;
    const int shellRadius = (shellNr >> 1);

    const bool subNrBit1      = ((subNr & 0x1) != 0);
    const int  subNrRemaining = (subNr >> 1);

    int ncx = center.x() + (shellRadius * shellSign);
    int ncy = center.y() + (shellRadius * shellSign);

    if (subNrBit1)
    {
      ncy -= subNrRemaining * shellSign;
    }
    else
    {
      ncx -= subNrRemaining * shellSign;
    }

    currentPos = QPoint(ncx, ncy);
  }
};

#endif // RECTANGLEINNERTOOUTERITERATOR_H
