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
  bool fineZoom;
  bool zoomOut;


  /** Returns the default path to be used for Minecraft location. */
  static QString getDefaultLocation();

 signals:
  void settingsUpdated();
  void locationChanged(const QString &loc);

 private slots:
  void toggleAutoUpdate(bool on);
  void browseLocation(bool on);
  void toggleDefaultLocation(bool on);
  void pathChanged(const QString &path);
  void toggleVerticalDepth(bool on);

  void on_checkBox_zoom_out_toggled(bool checked);

  void on_checkBox_fine_zoom_toggled(bool checked);

private:
  Ui::Settings m_ui;
};

#endif  // SETTINGS_H_
