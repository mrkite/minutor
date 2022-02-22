/** Copyright (c) 2013, Sean Kasun */
#include <QVBoxLayout>
#include <cmath>
#include "labelledslider.h"

LabelledSlider::LabelledSlider(Qt::Orientation orientation, QWidget *parent) :
  QWidget(parent) {
  slider = new MSlider(orientation);
  slider->setRange(0, 255);
  slider->setTickInterval(64);
  slider->setTickPosition(QSlider::TicksBelow);

  label = new QLabel();
  label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  label->setFixedWidth(label->fontMetrics().width("888"));
  label->setNum(255);

  connect(slider, &MSlider::valueChanged,
          this,   &LabelledSlider::intValueChange);

  QBoxLayout *myLayout;
  if (orientation == Qt::Vertical) {
    myLayout = new QVBoxLayout;
    myLayout->addWidget(label);
    myLayout->addWidget(slider);
  } else {
    myLayout = new QHBoxLayout;
    myLayout->addWidget(slider);
    myLayout->addWidget(label);
  }
  setLayout(myLayout);
  setFocusProxy(slider);
}

int LabelledSlider::value() const {
  return slider->value();
}

// public slot
void LabelledSlider::setValue(double v) {
  preciseValue = v;
  if (preciseValue < slider->minimum()) preciseValue = slider->minimum();
  if (preciseValue > slider->maximum()) preciseValue = slider->maximum();
  slider->setValue(static_cast<int>(floor(preciseValue + 0.5)));
}

// public slot
void LabelledSlider::changeValue(double v) {
  this->setValue(preciseValue + v);
}

// public slot
void LabelledSlider::setRange(int minVal, int maxVal)
{
  slider->setRange(minVal, maxVal);
}

// private slot
void LabelledSlider::intValueChange(int v) {
  // update preciceValue in case slider is moved by other means
  preciseValue -= static_cast<long>(preciseValue);  // preserve fractional part
  preciseValue += v;                                // take over integer part
  label->setNum(v);
  emit valueChanged(v);
}

void LabelledSlider::wheelEvent(QWheelEvent* event) {
  this->changeValue(event->angleDelta().y() / 120.0); // in order to make wheel intuitive
}

MSlider::MSlider(Qt::Orientation orientation, QWidget* parent) :
  QSlider(orientation, parent) {}

// catch the wheelEvent to prevent it from scrolling the slider
// ignore the event to allow it to bubble up to the LabelledSlider
void  MSlider::wheelEvent(QWheelEvent* event) {
  event->ignore();
}
