#ifndef SEARCHTEXTWIDGET_H
#define SEARCHTEXTWIDGET_H

#include <QWidget>

namespace Ui {
class SearchTextWidget;
}

class SearchTextWidget : public QWidget
{
  Q_OBJECT

 public:
  explicit SearchTextWidget(const QString& name, QWidget *parent = nullptr);
  ~SearchTextWidget();

  bool active() const;
  void setActive (bool flag = true);
  void hideActive(bool flag = true);

  bool exactMatch() const;
  void setExactMatch (bool flag = true);
  void hideExactMatch(bool flag = true);

  void addSuggestion(const QString& item);

  QString getSearchText() const;

  bool matches(const QString& textToSearch) const;

 private:
  Ui::SearchTextWidget *ui;
};

#endif // SEARCHTEXTWIDGET_H
