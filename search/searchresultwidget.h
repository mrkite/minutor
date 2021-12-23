#ifndef SEARCHRESULTWIDGET_H
#define SEARCHRESULTWIDGET_H

#include "overlay/properties.h"

#include <QWidget>
#include <QVariant>
#include <QVector3D>
#include <QSharedPointer>

class QTreeWidgetItem;
class OverlayItem;

namespace Ui {
class SearchResultWidget;
}

class SearchResultItem
{
 public:
  QString   name;
  QVector3D pos;
  QString   offers;
  QVariant  properties;
  QSharedPointer<OverlayItem> entity;
};

class SearchResultWidget : public QWidget
{
  Q_OBJECT

 public:
  explicit SearchResultWidget(QWidget *parent = nullptr);
  ~SearchResultWidget();

  void clearResults();
  void addResult(const SearchResultItem &result);
  void searchDone();

  void setPointOfInterest(const QVector3D& centerPoint);

 signals:
  void jumpTo(QVector3D pos);
  void updateSearchResultPositions(QVector<QSharedPointer<OverlayItem> >);

 protected slots:
  void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
 private slots:
  void on_treeWidget_itemSelectionChanged();

  void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

  void on_check_display_all_stateChanged();

  void on_saveSearchResults_clicked();

 private:
  Ui::SearchResultWidget *ui;

  QVector3D pointOfInterest;

  void updateStatusText();
};

#endif // SEARCHRESULTWIDGET_H
