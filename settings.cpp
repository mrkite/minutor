/** Copyright (c) 2013, Sean Kasun */
#include <QtWidgets/QFileDialog>
#include <QSettings>
#include <QDir>

#include "settings.h"

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

  connect(m_ui.pushButton_UpdateNow, SIGNAL(clicked()),
          this, SLOT(clickedUpdateNow()));

  connect(m_ui.radioButton_depth_shift, &QRadioButton::toggled,
          this,                         &Settings::toggleModifier4DepthSlider);
  connect(m_ui.radioButton_depth_ctrl,  &QRadioButton::toggled,
          this,                         &Settings::toggleModifier4DepthSlider);
  connect(m_ui.radioButton_depth_alt,   &QRadioButton::toggled,
          this,                         &Settings::toggleModifier4DepthSlider);

  connect(m_ui.radioButton_zoom_shift, &QRadioButton::toggled,
          this,                        &Settings::toggleModifier4ZoomOut);
  connect(m_ui.radioButton_zoom_ctrl,  &QRadioButton::toggled,
          this,                        &Settings::toggleModifier4ZoomOut);
  connect(m_ui.radioButton_zoom_alt,   &QRadioButton::toggled,
          this,                        &Settings::toggleModifier4ZoomOut);

  // Load the settings:
  QSettings info;
  auto useDefault = info.value("usedefault", true).toBool();
  if (useDefault) {
    mcpath = getDefaultLocation();
  }
  else {
    mcpath = info.value("mcdir", "").toString();
  }
  autoUpdate    = info.value("autoupdate", true).toBool();
  verticalDepth = info.value("verticaldepth", true).toBool();
  modifier4DepthSlider = Qt::KeyboardModifier(info.value("modifier4DepthSlider", 0x02000000).toUInt());
  modifier4ZoomOut     = Qt::KeyboardModifier(info.value("modifier4ZoomOut",     0x04000000).toUInt());

  // Set the UI to the current settings' values:
  m_ui.lineEdit_Location->setText(mcpath);
  m_ui.lineEdit_Location->setDisabled(useDefault);
  m_ui.checkBox_DefaultLocation->setChecked(useDefault);
  m_ui.checkBox_VerticalDepth->setChecked(verticalDepth);
  m_ui.checkBox_AutoUpdate->setChecked(autoUpdate);
  switch (modifier4DepthSlider) {
  case Qt::ControlModifier:
    m_ui.radioButton_depth_ctrl->setChecked(true);
    break;
  case Qt::AltModifier:
    m_ui.radioButton_depth_alt->setChecked(true);
    break;
  default:
    m_ui.radioButton_depth_shift->setChecked(true);
    break;
  }
  switch (modifier4ZoomOut) {
  case Qt::ControlModifier:
    m_ui.radioButton_zoom_ctrl->setChecked(true);
    break;
  case Qt::AltModifier:
    m_ui.radioButton_zoom_alt->setChecked(true);
    break;
  default:
    m_ui.radioButton_zoom_shift->setChecked(true);
    break;
  }
  this->toggleModifier4DepthSlider();
  this->toggleModifier4ZoomOut();
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

void Settings::clickedUpdateNow()
{
  emit checkForUpdates();
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

void Settings::toggleModifier4DepthSlider() {
  if (m_ui.radioButton_depth_shift->isChecked()) {
    modifier4DepthSlider = Qt::ShiftModifier;
    m_ui.radioButton_zoom_shift->setEnabled(false);
    m_ui.radioButton_zoom_ctrl ->setEnabled(true);
    m_ui.radioButton_zoom_alt  ->setEnabled(true);
  }
  if (m_ui.radioButton_depth_ctrl->isChecked()) {
    modifier4DepthSlider = Qt::ControlModifier;
    m_ui.radioButton_zoom_shift->setEnabled(true);
    m_ui.radioButton_zoom_ctrl ->setEnabled(false);
    m_ui.radioButton_zoom_alt  ->setEnabled(true);
  }
  if (m_ui.radioButton_depth_alt->isChecked()) {
    modifier4DepthSlider = Qt::AltModifier;
    m_ui.radioButton_zoom_shift->setEnabled(true);
    m_ui.radioButton_zoom_ctrl ->setEnabled(true);
    m_ui.radioButton_zoom_alt  ->setEnabled(false);
  }
  QSettings info;
  info.setValue("modifier4DepthSlider", modifier4DepthSlider);
}

void Settings::toggleModifier4ZoomOut() {
  if (m_ui.radioButton_zoom_shift->isChecked()) {
    modifier4ZoomOut = Qt::ShiftModifier;
    m_ui.radioButton_depth_shift->setEnabled(false);
    m_ui.radioButton_depth_ctrl ->setEnabled(true);
    m_ui.radioButton_depth_alt  ->setEnabled(true);
  }
  if (m_ui.radioButton_zoom_ctrl->isChecked()) {
    modifier4ZoomOut = Qt::ControlModifier;
    m_ui.radioButton_depth_shift->setEnabled(true);
    m_ui.radioButton_depth_ctrl ->setEnabled(false);
    m_ui.radioButton_depth_alt  ->setEnabled(true);
  }
  if (m_ui.radioButton_zoom_alt->isChecked()) {
    modifier4ZoomOut = Qt::AltModifier;
    m_ui.radioButton_depth_shift->setEnabled(true);
    m_ui.radioButton_depth_ctrl ->setEnabled(true);
    m_ui.radioButton_depth_alt  ->setEnabled(false);
  }
  QSettings info;
  info.setValue("modifier4ZoomOut", modifier4ZoomOut);
}
