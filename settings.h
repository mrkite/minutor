/** Copyright (c) 2013, Sean Kasun */
#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QDialog>
#include "ui_settings.h"


class Settings : public QDialog {
  Q_OBJECT
 public:
  explicit Settings(QWidget *parent = 0);

  QString mcpath;
  bool verticalDepth;
  bool autoUpdate;
  Qt::KeyboardModifier modifier4DepthSlider;
  Qt::KeyboardModifier modifier4ZoomOut;

  /** Returns the default path to be used for Minecraft location. */
  static QString getDefaultLocation();

 signals:
  void settingsUpdated();
  void locationChanged(const QString &loc);
  void checkForUpdates();

 private slots:
  void toggleAutoUpdate(bool on);
  void clickedUpdateNow();
  void browseLocation(bool on);
  void toggleDefaultLocation(bool on);
  void pathChanged(const QString &path);
  void toggleVerticalDepth(bool on);
  void toggleModifier4DepthSlider();
  void toggleModifier4ZoomOut();

 private:
  Ui::Settings m_ui;
};

#endif  // SETTINGS_H_
