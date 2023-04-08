/** Copyright (c) 2021, EtlamGit */
#include "labelledseparator.h"

LabeledSeparator::LabeledSeparator(QWidget* parent)
  : QWidgetAction(parent)
{
  line.setFrameShape(QFrame::HLine);
  line.setFrameShadow(QFrame::Sunken);
  line.setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

  layout.addWidget(&checkbox);
  layout.addWidget(&label);
  layout.addWidget(&line);
  checkbox.setTristate(true);
  setCheckable(true);
  connect(&checkbox, &QCheckBox::toggled, this, &QAction::setChecked);
  connect(&checkbox, &QCheckBox::clicked, this, &QWidgetAction::triggered);
  connect(this, &QWidgetAction::triggered, &checkbox, &QCheckBox::setChecked);
  widget.setLayout(&layout);
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

void LabeledSeparator::setCheckState(Qt::CheckState state)
{
  setChecked(state != Qt::Unchecked);
  checkbox.setCheckState(state);
}

Qt::CheckState LabeledSeparator::checkState() const
{
  return checkbox.checkState();
}
