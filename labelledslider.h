/** Copyright (c) 2013, Sean Kasun */
#ifndef LABELLEDSLIDER_H_
#define LABELLEDSLIDER_H_

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QWheelEvent>
#include <QKeyEvent>

class MSlider : public QSlider {
  Q_OBJECT

 public:
  explicit MSlider(Qt::Orientation orientation, QWidget *parent = 0);

 public slots:
  void wheelEvent(QWheelEvent *event);

};


class LabelledSlider : public QWidget {
  Q_OBJECT

 public:
  explicit LabelledSlider(Qt::Orientation orientation, QWidget *parent = 0);
  int value() const;

 signals:
  void valueChanged(int val);

 public slots:
  void setValue(double val);                 // set absolute value
  void changeValue(double val);              // change value relative to current
  void setRange(int minVal, int maxVal);  // set slider range

 private slots:
  void intValueChange(int val);

 protected:
  void wheelEvent(QWheelEvent *event);

 private:
  MSlider *slider;
  QLabel *label;
  double preciseValue;
};

#endif  // LABELLEDSLIDER_H_
