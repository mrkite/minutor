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

  // Load the settings:
  QSettings info;
  auto useDefault = info.value("usedefault", true).toBool();
  if (useDefault) {
    mcpath = getDefaultLocation();
  }
  else {
    mcpath = info.value("mcdir", "").toString();
  }
  autoUpdate = info.value("autoupdate", true).toBool();
  verticalDepth = info.value("verticaldepth", true).toBool();

  // Set the UI to the current settings' values:
  m_ui.checkBox_AutoUpdate->setChecked(autoUpdate);
  m_ui.lineEdit_Location->setText(mcpath);
  m_ui.lineEdit_Location->setDisabled(useDefault);
  m_ui.checkBox_DefaultLocation->setChecked(useDefault);
  m_ui.checkBox_VerticalDepth->setChecked(verticalDepth);
}

QString Settings::getDefaultLocation()
{
#ifdef Q_OS_MAC
  return QDir::homePath() +
    QDir::toNativeSeparators("/Library/Application Support/minecraft");
#elif defined Q_OS_WIN32
  // pretend to be minecraft
  QSettings ini(QSettings::IniFormat, QSettings::UserScope,
        ".minecraft", "minecraft1");
  return QFileInfo(ini.fileName()).absolutePath();
#else
  return QDir::homePath() + QDir::toNativeSeparators("/.minecraft");
#endif
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
  if (!dirName.isEmpty()) {
    emit locationChanged(dirName);
  }
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
  if (!def) {  // we unchecked default.. but we're still technically default
    return;
  }

  emit locationChanged(getDefaultLocation());
}

void Settings::toggleVerticalDepth(bool value) {
  verticalDepth = value;
  QSettings info;
  info.setValue("verticaldepth", value);
  emit settingsUpdated();
}
