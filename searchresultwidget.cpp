#include "searchresultwidget.h"
#include "ui_searchresultwidget.h"

#include "properties.h"

Q_DECLARE_METATYPE(SearchResultItem);

SearchResultWidget::SearchResultWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchResultWidget)
{
  ui->setupUi(this);

  ui->treeWidget->sortByColumn(1, Qt::SortOrder::AscendingOrder);
}

SearchResultWidget::~SearchResultWidget()
{
  delete ui;
}

void SearchResultWidget::clearResults()
{
  ui->treeWidget->clear();
}

namespace {

  class MyTreeWidgetItem : public QTreeWidgetItem {
    public:
    MyTreeWidgetItem(QTreeWidget* parent):QTreeWidgetItem(parent){}
    private:
    bool operator<(const QTreeWidgetItem &other)const {
      int column = treeWidget()->sortColumn();
      switch (column)
      {
      case 1:
        return text(column).toDouble() < other.text(column).toDouble();
      default:
        return QTreeWidgetItem::operator<(other);
      }
    }
  };

}

void SearchResultWidget::addResult(const SearchResultItem &result)
{
  if (ui->treeWidget->topLevelItemCount() > 16000)
  {
    return;
  }

  auto item = new MyTreeWidgetItem(nullptr);
  item->setData(0, Qt::UserRole, QVariant::fromValue(result));

  QVector3D distance = pointOfInterest - result.pos;
  float slopeDistance = distance.length();

  int c = 0;
  item->setText(c++, result.name);
  item->setText(c++, QString::number(std::roundf(slopeDistance)));
  item->setText(c++, QString("%1,%2,%3").arg(result.pos.x()).arg(result.pos.y()).arg(result.pos.z()));
  item->setText(c++, result.buys);
  item->setText(c++, result.sells);

  ui->treeWidget->addTopLevelItem(item);
}

void SearchResultWidget::searchDone()
{
  on_check_display_all_stateChanged(0);
  updateStatusText();
}

void SearchResultWidget::setPointOfInterest(const QVector3D &centerPoint)
{
  pointOfInterest = centerPoint;
  updateStatusText();
}

void SearchResultWidget::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
  auto properties = new Properties();

  auto props = item->data(0, Qt::UserRole).value<SearchResultItem>().properties;
  properties->DisplayProperties(props);
  properties->showNormal();
}

void SearchResultWidget::on_treeWidget_itemSelectionChanged()
{
  auto list = ui->treeWidget->selectedItems();
  if (list.size() > 0)
  {
    auto item = list[0];
    auto data = item->data(0, Qt::UserRole).value<SearchResultItem>();
    emit jumpTo(data.pos);

    if (!ui->check_display_all->isChecked())
    {
      QVector<QSharedPointer<OverlayItem> > items;
      items.push_back(data.entity);

      emit updateSearchResultPositions(items);
    }
  }
}

void SearchResultWidget::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
  on_treeWidget_itemSelectionChanged();
}

void SearchResultWidget::on_check_display_all_stateChanged(int arg1)
{
  QVector<QSharedPointer<OverlayItem> > items;

  if (ui->check_display_all->isChecked())
  {
    const int count = ui->treeWidget->topLevelItemCount();
    for (int i = 0; i < count; i++)
    {
      auto item = ui->treeWidget->topLevelItem(i);
      auto data = item->data(0, Qt::UserRole).value<SearchResultItem>();

      items.push_back(data.entity);
    }
  }

  emit updateSearchResultPositions(items);
}

void SearchResultWidget::updateStatusText()
{
  ui->lbl_location->setText(QString("%4 results around position: %1,%2,%3")
                              .arg(pointOfInterest.x())
                              .arg(pointOfInterest.y())
                              .arg(pointOfInterest.z())
                              .arg(ui->treeWidget->topLevelItemCount()));
}
