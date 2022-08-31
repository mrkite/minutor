/** Copyright (c) 2013, Sean Kasun */

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QStandardPaths>
#include <QtWidgets/QFileDialog>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonParseError>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <algorithm>
#include <zlib.h>
#include "definitionmanager.h"
#include "biomeidentifier.h"
#include "blockidentifier.h"
#include "dimensionidentifier.h"
#include "entityidentifier.h"
#include "flatteningconverter.h"
#include "mapview.h"
#include "zipreader.h"
#include "definitionupdater.h"


static quint32 stableHash(const quint8 *data, int size)
{
  constexpr const int step = sizeof(void *);
  quint32 crc = crc32(0L, Z_NULL, 0);
  for (int i = 0; i < size; i += step)
    crc = crc32(crc, data + i, std::min(step, size - i));
  return crc;
}

static quint32 stableHash(const char *data, int size = -1)
{
  if (size < 0)
    size = strlen(data);
  return stableHash(reinterpret_cast<const quint8 *>(data), size);
}

static quint32 stableHash(const QByteArray &data)
{
  return stableHash(data.data(), data.size());
}

static quint32 stableHash(const QString &str)
{
  return stableHash(str.toUtf8());
}


DefinitionManager::DefinitionManager(QWidget *parent) :
    QWidget(parent),
    biomeManager(BiomeIdentifier::Instance()),
    blockManager(BlockIdentifier::Instance()),
    dimensionManager(DimensionIdentifier::Instance()),
    entityManager(EntityIdentifier::Instance()),
    flatteningConverter(FlatteningConverter::Instance()),
    isUpdating(false)
{
  setWindowFlags(Qt::Window);
  setWindowTitle(tr("Definitions"));

  QVBoxLayout *layout = new QVBoxLayout;
  QStringList labels;
  labels << tr("Name") << tr("Version") << tr("Type");
  table = new QTableWidget(0, 3);
  table->setHorizontalHeaderLabels(labels);
  table->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->horizontalHeader()->setHighlightSections(false);
  table->verticalHeader()->hide();
  table->setShowGrid(false);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  layout->addWidget(table, 1);

  QWidget *buttonBar = new QWidget;
  QHBoxLayout *buttons = new QHBoxLayout;
  QPushButton *add = new QPushButton(tr("Add Pack..."));
  connect(add,  &QPushButton::clicked,
          this, &DefinitionManager::addPack);
  buttons->addWidget(add);
  QPushButton *remove = new QPushButton(tr("Remove Pack"));
  connect(remove, &QPushButton::clicked,
          this,   &DefinitionManager::removePack);
  connect(this,   &DefinitionManager::packSelected,
          remove, &QPushButton::setEnabled);
  buttons->addWidget(remove);
  QPushButton *save = new QPushButton(tr("Export Pack..."));
  connect(save, &QPushButton::clicked,
          this, &DefinitionManager::exportPack);
  connect(this, &DefinitionManager::packSelected,
          save, &QPushButton::setEnabled);
  buttons->addWidget(save);
  buttonBar->setLayout(buttons);
  layout->addWidget(buttonBar, 0);

  emit packSelected(false);
  setLayout(layout);

  // check & repair definition files
  this->checkAndRepair();

  // we load the definitions in backwards order for priority
  QSettings settings;
  sorted = settings.value("packs").toStringList();
  for (int i = sorted.length() - 1; i >= 0; i--)
    loadDefinition(sorted[i]);

  // hook up table selection signal
  connect(table, &QTableWidget::currentItemChanged,
          this,  &DefinitionManager::selectedPack );
  // fill out table
  refresh();
}

DefinitionManager::~DefinitionManager() {}

void DefinitionManager::checkAndRepair()
{
  // create definition data folder on disk
  QString destdir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
  QDir::root().mkpath(destdir);

  // get known definition packs from application default settings storage
  QSettings settings;
  QStringList known_packs = settings.value("packs").toStringList();

  // in Minutor up to 2.2.0 we used hash without seed, which is incompatible to Qt5.12
  // force clean old hashed files generated without an extra seed
  // this assumes, that these hashes will never occur otherwise
  const QStringList old_hashed_list {
    "1050220429", "1241760321", "1443276275", "1798448990", "2422344665",
    "797458294", "1758375291", "1959512777", "2171806498", "4286207260",
  };
  for ( const auto& old_hashed_file : old_hashed_list  ) {
    QString old_path = destdir + "/" + old_hashed_file + ".json";
    QFile::remove(old_path);
    known_packs.removeOne(old_path);
  }

  // repair when definitions is on disk, but missing in settings
  QDirIterator on_disk(destdir, QDir::Files | QDir::Readable);
  while (on_disk.hasNext()) {
    on_disk.next();
    if (!known_packs.contains(on_disk.filePath())) {
      known_packs.append(on_disk.filePath());
    }
  }

  // copy over built-in definitions if necessary
  QDirIterator build_in(":/definitions", QDir::Files | QDir::Readable);
  while (build_in.hasNext()) {
    build_in.next();
    installJson(build_in.filePath(), false, false);
  }
  // all changed definitions are now in sorted -> copy over
  known_packs.append(sorted);

  // remove duplicates
  QMutableListIterator<QString> itD(known_packs);
  while (itD.hasNext()) {
    if (known_packs.count(itD.next()) > 1)
      itD.remove();
  }

  // repair when definition is in settings, but file is missing
  QMutableListIterator<QString> itF(known_packs);
  while (itF.hasNext()) {
    if (!QFile::exists(itF.next()))
      itF.remove();
  }

  // store repaired list of definitions
  settings.setValue("packs", known_packs);
}

void DefinitionManager::refresh() {
  table->clearContents();
  table->setRowCount(0);
  QStringList types;
  types << tr("block") << tr("biome") << tr("dimension")
        << tr("entity") << tr("pack") << tr("converter");
  for (int i = 0; i < sorted.length(); i++) {
    Definition &def = definitions[sorted[i]];
    int row = table->rowCount();
    table->insertRow(row);
    QTableWidgetItem *name = new QTableWidgetItem(def.name);
    name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    name->setData(Qt::UserRole, def.path);
    table->setItem(row, 0, name);
    QTableWidgetItem *ver = new QTableWidgetItem(def.version);
    ver->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ver->setData(Qt::UserRole, def.path);
    table->setItem(row, 1, ver);
    QTableWidgetItem *type = new QTableWidgetItem(types[def.type]);
    type->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    type->setData(Qt::UserRole, def.path);
    table->setItem(row, 2, type);
  }
}

void DefinitionManager::selectedPack(QTableWidgetItem *item,
                                     QTableWidgetItem *) {
  emit packSelected(item != NULL);
  if (item != NULL)
    selected = item->data(Qt::UserRole).toString();
  else
    selected = QString();
}

void DefinitionManager::toggledPack(bool onoff) {
  if (definitions.contains(selected)) {
    Definition &def = definitions[selected];
    def.enabled = onoff;
    switch (def.type) {
      case Definition::Block:
        if (onoff)
          blockManager.enableDefinitions(def.id);
        else
          blockManager.disableDefinitions(def.id);
        break;
      case Definition::Biome:
        if (onoff)
          biomeManager.enableDefinitions(def.id);
        else
          biomeManager.disableDefinitions(def.id);
        break;
      case Definition::Dimension:
        if (onoff)
          dimensionManager.enableDefinitions(def.id);
        else
          dimensionManager.disableDefinitions(def.id);
        break;
      case Definition::Entity:
        if (onoff)
          entityManager.enableDefinitions(def.id);
        else
          entityManager.disableDefinitions(def.id);
        break;
      case Definition::Pack:
        if (onoff) {
          blockManager.enableDefinitions(def.blockid);
          biomeManager.enableDefinitions(def.biomeid);
          dimensionManager.enableDefinitions(def.dimensionid);
          entityManager.enableDefinitions(def.entityid);
        } else {
          blockManager.disableDefinitions(def.blockid);
          biomeManager.disableDefinitions(def.biomeid);
          dimensionManager.disableDefinitions(def.dimensionid);
          entityManager.disableDefinitions(def.entityid);
        }
        break;
    }
  }
  emit packsChanged();
  refresh();
}

void DefinitionManager::addPack() {
  QString packName = QFileDialog::getOpenFileName(this, tr("Open Pack"),
    QString(), tr("Definitions (*.zip *.json)"));
  if (!packName.isEmpty()) {
    if (packName.endsWith(".json", Qt::CaseInsensitive))  // raw json
      installJson(packName);
    else
      installZip(packName);
    emit packsChanged();
    QSettings settings;
    settings.setValue("packs", sorted);
    refresh();
  }
}

void DefinitionManager::installJson(QString path, bool overwrite,
                                    bool install) {
  QString destdir = QStandardPaths::writableLocation(
      QStandardPaths::DataLocation);

  QFile f(path);
  f.open(QIODevice::ReadOnly);
  QJsonParseError e;
  QJsonDocument json_doc = QJsonDocument::fromJson(f.readAll(), &e);
  f.close();
  if (json_doc.isNull()) {
    QMessageBox::warning(this, tr("Couldn't install %1").arg(path),
                         e.errorString(),
                         QMessageBox::Cancel);
    return;
  }
  QJsonObject def = json_doc.object();

  QString key = def.value("name").toString() + def.value("type").toString();
  QString exeversion = def.value("version").toString();

  // check if intermediate qHash(..,0) name is present
  QString dest0 = destdir + "/" + QString("%1").arg(qHash(key, 0)) + ".json";
  if (QFile::exists(dest0)) {
    QFile::remove(dest0);
  }

  QString dest = destdir + "/" + QString("%1").arg(stableHash(key)) + ".json";

  // check if build in version is newer than version on disk
  if (QFile::exists(dest)) {
    QFile f(dest);
    f.open(QIODevice::ReadOnly);
    json_doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (json_doc.isNull()) {
      return;
    }
    def = json_doc.object();
    QString fileversion = def.value("version").toString();

    if (DefinitionUpdater::versionCompare(exeversion, fileversion) > 0) {
      // force overwriting outdated local copy
      QFile::remove(dest);
      QFile::copy(path, dest);
      QFile::setPermissions(dest, QFile::ReadOwner|QFile::WriteOwner);
    }
  }

  // import new definition (if file is not present)
  if (!QFile::exists(dest) || overwrite) {
    if (QFile::exists(dest) && install) {
      removeDefinition(dest);
    }
    if (!QFile::copy(path, dest)) {
      QMessageBox::warning(this, tr("Couldn't install %1").arg(path),
                           tr("Copy error"),
                           QMessageBox::Cancel);
      return;
    }
    // fix permissions since we might be copying a readonly resource.
    QFile::setPermissions(dest, QFile::ReadOwner|QFile::WriteOwner);
    sorted.prepend(dest);
    if (install)
      loadDefinition(dest);
  }
}

void DefinitionManager::installZip(QString path, bool overwrite,
                                   bool install) {
  QString destdir = QStandardPaths::writableLocation(
      QStandardPaths::DataLocation);
  ZipReader zip(path);
  if (!zip.open()) {
    QMessageBox::warning(this, tr("Couldn't install %1").arg(path),
                         tr("Corrupt zip"),
                         QMessageBox::Cancel);
    return;
  }
  // fetch the pack info
  QJsonObject info;
  QJsonParseError e;
  QJsonDocument json_doc = QJsonDocument::fromJson(zip.get("pack_info.json"), &e);
  if (json_doc.isEmpty()) {
    QMessageBox::warning(this, tr("Couldn't install %1").arg(path),
                         tr("pack_info.json : %1").arg(e.errorString()),
                         QMessageBox::Cancel);
    zip.close();
    return;
  }
  info = json_doc.object();
  // let's verify all the jsons in the pack
  for (int i = 0; i < info.value("data").toArray().size(); i++) {
    QJsonDocument def = QJsonDocument::fromJson(zip.get(info.value("data").toArray().at(i).toString()), &e);
    if (def.isEmpty()) {
      QMessageBox::warning(this, tr("Couldn't install %1").arg(path),
                           tr("%1: %2")
                           .arg(info.value("data").toArray().at(i).toString(),
                                e.errorString()), QMessageBox::Cancel);
      zip.close();
      return;
    }
  }
  zip.close();

  QString key = info.value("name").toString() + info.value("type").toString();
  QString dest = destdir + "/" + QString("%1").arg(stableHash(key)) + ".zip";
  if (!QFile::exists(dest) || overwrite) {
    if (QFile::exists(dest) && install)
      removeDefinition(dest);
    if (!QFile::copy(path, dest)) {
      QMessageBox::warning(this,
                           tr("Couldn't install %1").arg(path),
                           tr("Copy error"),
                           QMessageBox::Cancel);
      return;
    }
    sorted.prepend(dest);
    if (install)
      loadDefinition(dest);
  }
}

void DefinitionManager::removePack() {
  // find selected pack
  if (definitions.contains(selected)) {
    int ret = QMessageBox::question(this, tr("Delete Pack"),
                                    tr("Are you sure you want to delete %1?")
                                    .arg(definitions[selected].name),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);
    if (ret == QMessageBox::Yes)
      removeDefinition(selected);
  }
}

void DefinitionManager::exportPack() {
  // find selected pack
  if (definitions.contains(selected)) {
    QString fname = definitions[selected].name;
    switch (definitions[selected].type) {
      case Definition::Block: fname+="_blocks"; break;
      case Definition::Biome: fname+="_biomes"; break;
      case Definition::Dimension: fname+="_dims"; break;
      default: break;
    }
    if (selected.endsWith(".zip"))
      fname += ".zip";
    else
      fname += ".json";
    QString dest = QFileDialog::getSaveFileName(this, tr("Save Pack As"),
      fname, tr("Definitions (*.zip *.json)"));
    if (!dest.isEmpty()) {
      if (!QFile::copy(selected, dest)) {
        QMessageBox::warning(this,
                             tr("Couldn't write to %1").arg(dest),
                             tr("Copy error"),
                             QMessageBox::Cancel);
      }
    }
  }
}

QSize DefinitionManager::minimumSizeHint() const {
  return QSize(300, 300);
}
QSize DefinitionManager::sizeHint() const {
  return QSize(400, 300);
}

void DefinitionManager::loadDefinition(QString path) {
  // determine if we're loading a single json or a pack
  if (path.endsWith(".json", Qt::CaseInsensitive)) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonDocument json_doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (json_doc.isNull()) {
      return;
    }
    QJsonObject def = json_doc.object();
    Definition d;
    d.name = def.value("name").toString();
    d.version = def.value("version").toString();
    d.path = path;
    d.update = def.value("update").toString();
    QString type = def.value("type").toString();
    d.enabled = true;  // should look this up
    if (type == "block") {
      d.id = flatteningConverter.addDefinitions(
          def.value("data").toArray());
      d.type = Definition::Converter;
    } else if (type == "flatblock") {
      d.id = blockManager.addDefinitions(
          def.value("data").toArray());
      d.type = Definition::Block;
    } else if (type == "biome") {
      d.id = biomeManager.addDefinitions(
            def.value("data").toArray(),
            def.value("data18").toArray());
      d.type = Definition::Biome;
    } else if (type == "dimension") {
      d.id = dimensionManager.addDefinitions(
          def.value("data").toArray());
      d.type = Definition::Dimension;
    } else if (type == "entity") {
      d.id = entityManager.addDefinitions(
          def.value("data").toArray());
      d.type = Definition::Entity;
    } else {
      return; // unknown type
    }
    definitions.insert(path, d);
  } else {
    ZipReader zip(path);
    if (!zip.open())
      return;
    QJsonDocument json_doc = QJsonDocument::fromJson(zip.get("pack_info.json"));
    if (json_doc.isNull()) {
      zip.close();
      return;
    }
    QJsonObject info = json_doc.object();
    Definition d;
    d.name = info.value("name").toString();
    d.version = info.value("version").toString();
    d.update = info.value("update").toString();
    d.path = path;
    d.enabled = true;
    d.id = 0;
    d.type = Definition::Pack;
    d.blockid = -1;
    d.biomeid = -1;
    d.dimensionid = -1;
    d.entityid = -1;
    for (int i = 0; i < info.value("data").toArray().size(); i++) {
      QJsonDocument json_doc = QJsonDocument::fromJson(zip.get(info.value("data").toArray().at(i).toString()));
      if (json_doc.isNull()) {
        continue;
      }
      QJsonObject def = json_doc.object();
      QString type = def.value("type").toString();
      if (type == "block") {
//        d.blockid = flatteningConverter->addDefinitions(
//            def.value("data").toArray(), d.blockid);
      } else if (type == "flatblock") {
          d.blockid = blockManager.addDefinitions(
              def.value("data").toArray(), d.blockid);
      } else if (type == "biome") {
        d.biomeid = biomeManager.addDefinitions(
              def.value("data").toArray(),
              def.value("data18").toArray(), d.biomeid);
      } else if (type == "dimension") {
        d.dimensionid = dimensionManager.addDefinitions(
            def.value("data").toArray(), d.dimensionid);
      } else if (type == "entity") {
        d.entityid = entityManager.addDefinitions(
            def.value("data").toArray(), d.entityid);
      }
    }
    definitions.insert(path, d);
    zip.close();
  }
}
void DefinitionManager::removeDefinition(QString path) {
  // find the definition and remove it from disk
  Definition &def = definitions[path];
  if (def.path == path) {
    switch (def.type) {
      case Definition::Block:
        blockManager.disableDefinitions(def.id);
        break;
      case Definition::Biome:
        biomeManager.disableDefinitions(def.id);
        break;
      case Definition::Dimension:
        dimensionManager.disableDefinitions(def.id);
        break;
      case Definition::Entity:
        entityManager.disableDefinitions(def.id);
        break;
      case Definition::Pack:
        blockManager.disableDefinitions(def.blockid);
        biomeManager.disableDefinitions(def.biomeid);
        dimensionManager.disableDefinitions(def.dimensionid);
        entityManager.disableDefinitions(def.entityid);
        break;
    }
    definitions.remove(path);
    QFile::remove(path);
    sorted.removeOne(path);
    QSettings settings;
    settings.setValue("packs", sorted);
    emit packsChanged();
    refresh();
  }
}

void DefinitionManager::checkForUpdates() {
  // show update dialog
  if (!isUpdating)
    autoUpdate();
  if (!isUpdating) {  // nothing needs updating
    // hide update dialog
    // show completion
  }
}

void DefinitionManager::autoUpdate() {
  for (int i = 0; i < sorted.length(); i++) {
    QString name = sorted[i];
    Definition &def = definitions[name];
    if (!def.update.isEmpty()) {
      isUpdating = true;
      auto updater = new DefinitionUpdater(name, def.update, def.version);
      connect(updater, &DefinitionUpdater::updated,
              this,    &DefinitionManager::updatePack);
      updateQueue.append(updater);
      updater->update();
    }
  }
  QSettings settings;
  settings.setValue("packupdate", QDateTime::currentDateTime());  // store current time
  settings.remove("packupdates");  // remove now unused entries from registry
}

void DefinitionManager::updatePack(DefinitionUpdater *updater,
                                   QString filename,
                                   QString version) {
  Q_UNUSED(filename);
  Q_UNUSED(version);

  updateQueue.removeOne(updater);
  delete updater;

  if (updateQueue.isEmpty())
    emit updateFinished();
}
