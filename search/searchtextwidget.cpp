#include "search/searchtextwidget.h"
#include "ui_searchtextwidget.h"

SearchTextWidget::SearchTextWidget(const QString &name, QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::SearchTextWidget)
{
  ui->setupUi(this);
  ui->checkBox_active->setText(name);
}

SearchTextWidget::~SearchTextWidget()
{
  delete ui;
}

bool SearchTextWidget::isActive() const
{
  return ui->checkBox_active->isChecked();
}

bool SearchTextWidget::exactMatch() const
{
  return ui->checkBox_exact_match->isChecked();
}

void SearchTextWidget::addSuggestion(const QString &item)
{
  ui->comboBox_text->addItem(item);
}

QString SearchTextWidget::getSearchText() const
{
  return ui->comboBox_text->currentText();
}

bool SearchTextWidget::matches(const QString &textToSearch) const
{
  QRegExp matcher(getSearchText(), Qt::CaseInsensitive, QRegExp::Wildcard);

  if (exactMatch()) {
    return matcher.exactMatch(textToSearch);
  } else {
    return (matcher.indexIn(textToSearch) >= 0);
  }
}
