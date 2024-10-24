#ifndef SEARCHRESULTITEM_H
#define SEARCHRESULTITEM_H

#include <QVariant>
#include <QVector3D>
#include <QSharedPointer>

class OverlayItem;

class SearchResultItem
{
 public:
  QString   name;
  QVector3D pos;
  QString   offers;
  QVariant  properties;
  QSharedPointer<OverlayItem> entity;
};

#endif // SEARCHRESULTITEM_H
