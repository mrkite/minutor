#ifndef SEARCHRANGEWIDGET_H
#define SEARCHRANGEWIDGET_H

#include <QWidget>

#include "search/range.h"


namespace Ui {
class SearchRangeWidget;
}

class SearchRangeWidget : public QWidget
{
  Q_OBJECT

public:
  explicit SearchRangeWidget(QWidget *parent = nullptr);
  ~SearchRangeWidget();

  unsigned int getRadius();
  unsigned int getRadiusChunks();

  void         setRangeY(Range<float> range);
  Range<float> getRangeY();

  void setButtonText(const QString &text);

  void setProgressValue(const unsigned int value);
  void setProgressMax(const unsigned int value);
  bool incrementProgressValue();

private:
  Ui::SearchRangeWidget *ui;
};

#endif // SEARCHRANGEWIDGET_H
