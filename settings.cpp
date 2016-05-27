/** Copyright (c) 2013, Sean Kasun */
#include <QtWidgets/QFileDialog>
#include <QSettings>
#include <QDir>

#include "./settings.h"

Settings::Settings(QWidget *parent) : QDialog(parent) {
  m_ui.setupUi(this);

  setWindowTitle(tr("%1 Settings").arg(qApp->applicationName()));

  connect(m_ui.checkBox_DefaultLocation, SIGNAL(toggled(bool)),
          this, SLOT(toggleDefaultLocation(bool)));
  connect(m_ui.checkBox_DefaultLocation, SIGNAL(toggled(bool)),
          m_ui.lineEdit_Location, SLOT(setDisabled(bool)));

  connect(this, SIGNAL(locationChanged(const QString)),
          m_ui.lineEdit_Location, SLOT(setText(const QString &)));

  connect(m_ui.lineEdit_Location, SIGNAL(textChanged(const QString &)),
          this, SLOT(pathChanged(const QString &)));

  connect(m_ui.pushButton_Browse, SIGNAL(clicked(bool)),
          this, SLOT(browseLocation(bool)));

  connect(m_ui.checkBox_VerticalDepth, SIGNAL(toggled(bool)),
          this, SLOT(toggleVerticalDepth(bool)));

  connect(m_ui.checkBox_AutoUpdate, SIGNAL(toggled(bool)),
          this, SLOT(toggleAutoUpdate(bool)));

  // set stored settings
  QSettings info;
  m_ui.checkBox_AutoUpdate->setChecked(info.value("autoupdate",
                                                  true).toBool());
  m_ui.lineEdit_Location->setText(info.value("mcdir", "").toString());
  m_ui.lineEdit_Location->setDisabled(
      m_ui.checkBox_DefaultLocation->isChecked());
  m_ui.checkBox_DefaultLocation->setChecked(info.value("usedefault",
                                                       true).toBool());
  m_ui.checkBox_VerticalDepth->setChecked(info.value("verticaldepth",
                                                     true).toBool());
}

void Settings::toggleAutoUpdate(bool up) {
  autoUpdate = up;
  QSettings info;
  info.setValue("autoupdate", up);
  emit settingsUpdated();
}

void Settings::browseLocation(bool /* on */) {
  QString dirName = QFileDialog::getExistingDirectory(this,
                                                      tr("Find Minecraft"));
  if (!dirName.isEmpty())
    emit locationChanged(dirName);
}

void Settings::pathChanged(const QString &path) {
  mcpath = path;
  QSettings info;
  info.setValue("mcdir", path);
  // save settings
  emit settingsUpdated();
}

void Settings::toggleDefaultLocation(bool def) {
  QSettings info;
  info.setValue("usedefault", def);
  if (!def)  // we unchecked default.. but we're still technically default
    return;

  QString mc;
#ifdef Q_OS_MAC
  mc = QDir::homePath() +
      QDir::toNativeSeparators("/Library/Application Support/minecraft");
#elif defined Q_OS_WIN32
  // pretend to be minecraft
  QSettings ini(QSettings::IniFormat, QSettings::UserScope,
                ".minecraft", "minecraft1");
  mc = QFileInfo(ini.fileName()).absolutePath();
#else
  mc = QDir::homePath()+QDir::toNativeSeparators("/.minecraft");
#endif
  emit locationChanged(mc);
}

void Settings::toggleVerticalDepth(bool value) {
  verticalDepth = value;
  QSettings info;
  info.setValue("verticaldepth", value);
  emit settingsUpdated();
}
