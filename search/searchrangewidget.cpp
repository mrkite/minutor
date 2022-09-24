#include "searchrangewidget.h"
#include "ui_searchrangewidget.h"


SearchRangeWidget::SearchRangeWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::SearchRangeWidget)
{
  ui->setupUi(this);
}

SearchRangeWidget::~SearchRangeWidget()
{
  delete ui;
}

// search range
unsigned int SearchRangeWidget::getRadius()
{
  return ui->sb_radius->value() + 1;
}

unsigned int SearchRangeWidget::getRadiusChunks()
{
  return getRadius() / 16;
}

void SearchRangeWidget::setRangeY(Range<float> range)
{
  ui->sb_y_start->setValue(range.start);
  ui->sb_y_end  ->setValue(range.end);
}

Range<float> SearchRangeWidget::getRangeY()
{
  if (!ui->check_range_y->isChecked()) {
    return Range<float>::max();
  } else {
    return Range<float>::createFromUnorderedParams(ui->sb_y_start->value(), ui->sb_y_end->value());
  }
}


// search button
void SearchRangeWidget::setButtonText(const QString &text)
{
  ui->pb_search->setText(text);
}

// progress bar
void SearchRangeWidget::setProgressValue(const unsigned int value)
{
  ui->progressBar->setValue(value);
}

void SearchRangeWidget::setProgressMax(const unsigned int value)
{
  ui->progressBar->setMaximum(value);
}

bool SearchRangeWidget::incrementProgressValue()
{
  ui->progressBar->setValue(ui->progressBar->value() + 1);
  // return true when maximum is reached
  return (ui->progressBar->value() == ui->progressBar->maximum());
}
