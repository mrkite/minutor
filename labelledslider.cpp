/** Copyright (c) 2013, Sean Kasun */
#include <QVBoxLayout>
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

  connect(slider, SIGNAL(valueChanged(int)),
          this, SLOT(intValueChange(int)));

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
void LabelledSlider::setValue(int v) {
  slider->setValue(v);
}

// public slot
void LabelledSlider::changeValue(int v) {
  slider->setValue(slider->value() + v);
}

// public slot
void LabelledSlider::setRange(int minVal, int maxVal)
{
  slider->setRange(minVal, maxVal);
}

// private slot
void LabelledSlider::intValueChange(int v) {
  label->setNum(v);
  emit valueChanged(v);
}

void LabelledSlider::wheelEvent(QWheelEvent* event) {
  slider->wheelEvent(event);
}

MSlider::MSlider(Qt::Orientation orientation, QWidget* parent) :
  QSlider(orientation, parent) {}

void  MSlider::wheelEvent(QWheelEvent* event) {
  int delta = event->delta() / 120;  // in order to make wheel intuitive
  this->setValue(this->value() + delta);
}
