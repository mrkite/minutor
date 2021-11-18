/** Copyright (c) 2021, EtlamGit */
#include "labelledseparator.h"

LabeledSeparator::LabeledSeparator(QWidget* parent)
  : QWidgetAction(parent)
{
  line.setFrameShape(QFrame::HLine);
  line.setFrameShadow(QFrame::Sunken);
  line.setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

  layout.addWidget(&label);
  layout.addWidget(&line);
  widget.setLayout(&layout);
  widget.setEnabled(false);
  this->setDefaultWidget(&widget);
}

LabeledSeparator::LabeledSeparator(const QString& text, QWidget* parent)
  : LabeledSeparator(parent)
{
  this->setText(text);
}

void LabeledSeparator::setText(const QString& text)
{
  label.setText(text);
}
