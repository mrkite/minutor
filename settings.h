/** Copyright (c) 2013, Sean Kasun */
#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QDialog>
#include "./ui_settings.h"


class Settings : public QDialog {
  Q_OBJECT
 public:
  explicit Settings(QWidget *parent = 0);

  bool autoUpdate;
  bool verticalDepth;
  QString mcpath;

 signals:
  void settingsUpdated();
  void locationChanged(const QString &loc);

 private slots:
  void toggleAutoUpdate(bool on);
  void browseLocation(bool on);
  void toggleDefaultLocation(bool on);
  void pathChanged(const QString &path);
  void toggleVerticalDepth(bool on);

 private:
  Ui::Settings m_ui;
};

#endif  // SETTINGS_H_
