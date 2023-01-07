/** Copyright (c) 2021, EtlamGit */
#ifndef LABELEDSEPARATOR_H
#define LABELEDSEPARATOR_H

#include <QWidgetAction>
#include <QCheckBox>
#include <QLabel>
#include <QFrame>
#include <QLayout>

class LabeledSeparator: public QWidgetAction
{
  Q_OBJECT

 public:
  explicit LabeledSeparator(QWidget* parent=0);
  explicit LabeledSeparator(const QString& text, QWidget* parent=0);

  void setText(const QString& text);

 private:
  QWidget widget;
  QHBoxLayout layout;
  QCheckBox checkbox;
  QLabel label;
  QFrame line;
};

#endif // LABELEDSEPARATOR_H
