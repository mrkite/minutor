#include "searchresultwidget.h"
#include "ui_searchresultwidget.h"

#include <QMessageBox>
#include <QTextStream>
#include <QtWidgets/QFileDialog>
#include <cmath>

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
      switch (column) {
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
    return;

  auto item = new MyTreeWidgetItem(nullptr);
  item->setData(0, Qt::UserRole, QVariant::fromValue(result));

  QVector3D distance = pointOfInterest - result.pos;
  float slopeDistance = distance.length();

  int c = 0;
  item->setText(c++, result.name);
  item->setTextAlignment(c, Qt::AlignHCenter);
  item->setText(c++, QString::number(std::roundf(slopeDistance)));
  item->setTextAlignment(c, Qt::AlignHCenter);
  item->setText(c++, QString().number(int(std::roundf(result.pos.x()))) + "/" +
                     QString().number(int(std::roundf(result.pos.y()))) + "/" +
                     QString().number(int(std::roundf(result.pos.z()))) );
  item->setText(c++, result.offers);

  ui->treeWidget->addTopLevelItem(item);

}

void SearchResultWidget::searchDone()
{
  // adapt width of result columns
  for (int i = 0; i < ui->treeWidget->columnCount(); i++)
      ui->treeWidget->resizeColumnToContents(i);
  // update overlay items
  on_check_display_all_stateChanged();
  // update search status
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
  if (list.size() > 0) {
    auto item = list[0];
    auto data = item->data(0, Qt::UserRole).value<SearchResultItem>();
    emit jumpTo(data.pos);

    if (!ui->check_display_all->isChecked()) {
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

void SearchResultWidget::on_check_display_all_stateChanged()
{
  QVector<QSharedPointer<OverlayItem> > items;

  if (ui->check_display_all->isChecked()) {
    const int count = ui->treeWidget->topLevelItemCount();
    for (int i = 0; i < count; i++) {
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

void SearchResultWidget::on_saveSearchResults_clicked() {
  QFileDialog saveFileDialog(this);
  saveFileDialog.setDefaultSuffix("tsv");
  QString saveFilename = saveFileDialog.getSaveFileName(
        this, tr("Save search results as tab separated values"),
        QString(), "Tab separated files (*.tsv)");

  if (saveFilename.isEmpty())
    return;

  QString delim = "\t";

  // Ensure filename suffix.
  QFile f(saveFilename);
  QFileInfo fileInfo(f);
  if (fileInfo.suffix().isEmpty()) {
    saveFilename.append(".tsv");
    f.setFileName(saveFilename);
  }

  // Save current search data to file.
  if (!f.open(QFile::WriteOnly | QFile::Truncate)) {
    QMessageBox errMsgBox(this);
    QString s = QStringLiteral("Error opening file %1").arg(saveFilename);
    errMsgBox.setText(s);
    errMsgBox.exec();
    return;
  }
  QTextStream ts(&f);

  // Write header.
  const QTreeWidgetItem *header = ui->treeWidget->headerItem();
  int nCol = header->columnCount();
  for (int col = 0; col < nCol - 1; col++) {
    ts << header->text(col) << delim;
  }
  ts << header->text(nCol - 1) << "\n";

  QTreeWidgetItemIterator iter(ui->treeWidget);
  // Write the data as it appears in the search results widget.
  // TODO: Consider adding structure to output data based on the
  // nature of the search/search results.
  while (*iter) {
    // Write columns separated by delimiters.
    int nCol = (*iter)->columnCount();
    for (int col = 0; col < nCol - 1; col++) {
      ts << (*iter)->text(col) << delim;
    }
    // Write last column and start a new line
    ts << (*iter)->text(nCol - 1) << "\n";
    ++iter;
  }

  f.flush();
  f.close();
}
