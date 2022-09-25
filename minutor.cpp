/** Copyright (c) 2013, Sean Kasun */
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QAction>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTreeWidget>
#include <QProgressDialog>
#include <QDir>
#include <QRegExp>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include "minutor.h"
#include "mapview.h"
#include "labelledseparator.h"
#include "labelledslider.h"
#include "nbt/nbt.h"
#include "identifier/definitionmanager.h"
#include "identifier/entityidentifier.h"
#include "identifier/dimensionidentifier.h"
#include "settings.h"
#include "worldinfo.h"
#include "worldsave.h"
#include "overlay/properties.h"
#include "overlay/generatedstructure.h"
#include "overlay/village.h"
#include "jumpto.h"
#include "pngexport.h"
#include "search/searchchunksdialog.h"
#include "search/searchentityplugin.h"
#include "search/searchblockplugin.h"
#include "search/statisticdialog.h"

Minutor::Minutor()
{
  m_ui.setupUi(this);

  // central MapView wiget
  mapview = new MapView;
  connect(mapview,     SIGNAL(hoverTextChanged(QString)),
          statusBar(), SLOT(showMessage(QString)));
  connect(mapview, SIGNAL(showProperties(QVariant)),
          this,    SLOT(showProperties(QVariant)));
  connect(mapview, SIGNAL(addOverlayItemType(QString, QColor)),
          this,    SLOT(addOverlayItemType(QString, QColor)));
  dm = new DefinitionManager(this);
  mapview->attach(dm);
  connect(dm,   SIGNAL(packsChanged()),
          this, SLOT(updateDimensions()));
  WorldInfo & wi = WorldInfo::Instance();
  connect(&wi,  SIGNAL(dimensionChanged(const DimensionInfo &)),
          this, SLOT(viewDimension(const DimensionInfo &)));

  // "Settings" dialog
  dialogSettings = new Settings(this);
  connect(dialogSettings, SIGNAL(settingsUpdated()),
          this,           SLOT(rescanWorlds()));

  // "Jump To" dialog
  dialogJumpTo = new JumpTo(this);

  if (dialogSettings->autoUpdate) {
    // get time of last update
    QSettings settings;
    QDateTime lastUpdateTime = settings.value("packupdate").toDateTime();

    // auto-update only once a week
    if (lastUpdateTime.addDays(7) < QDateTime::currentDateTime())
      dm->autoUpdate();
  }

  setWindowTitle(qApp->applicationName());
  // create dynamic part of menu
  getWorldList();
  createMenus();
  // connect actions to handlers
  createActions();
  // configure status bar
  createStatusBar();

  // central layout
  QBoxLayout *mainLayout;
  if (dialogSettings->verticalDepth) {
    mainLayout = new QHBoxLayout;
    depth = new LabelledSlider(Qt::Vertical);
    mainLayout->addWidget(mapview, 1);
    mainLayout->addWidget(depth);
  } else {
    mainLayout = new QVBoxLayout;
    depth = new LabelledSlider(Qt::Horizontal);
    mainLayout->addWidget(depth);
    mainLayout->addWidget(mapview, 1);
  }
  depth->setValue(255);
  mainLayout->setSpacing(0);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  connect(depth, SIGNAL(valueChanged(int)),
          mapview, SLOT(setDepth(int)));
  connect(mapview, SIGNAL(demandDepthChange(double)),
          depth, SLOT(changeValue(double)));
  connect(mapview, SIGNAL(demandDepthValue(double)),
          depth, SLOT(setValue(double)));
  connect(this, SIGNAL(worldLoaded(bool)),
          mapview, SLOT(setEnabled(bool)));
  connect(this, SIGNAL(worldLoaded(bool)),
          depth, SLOT(setEnabled(bool)));

  m_ui.centralwidget->setLayout(mainLayout);
  layout()->setContentsMargins(0, 0, 0, 0);

  propView = new Properties(this);

  emit worldLoaded(false);
}

Minutor::~Minutor() {
  // wait for sheduled tasks
  QThreadPool::globalInstance()->waitForDone();
}


void Minutor::openWorld() {
  QAction *action = qobject_cast<QAction*>(sender());
  if (action)
    loadWorld(action->data().toString());
}

void Minutor::open() {
  QString dirName = QFileDialog::getExistingDirectory(this, tr("Open World"));
  if (!dirName.isEmpty()) {
    QDir path(dirName);
    if (!path.exists("level.dat")) {
      QMessageBox::warning(this,
                           tr("Couldn't open world"),
                           tr("%1 is not a valid Minecraft world")
                           .arg(dirName),
                           QMessageBox::Cancel);
      return;
    }
    loadWorld(dirName);
  }
}

void Minutor::reload() {
  auto loc = *(mapview->getLocation());

  loadWorld(currentWorld);
  mapview->setLocation(loc.x, loc.y, loc.z, false, true);
}

void Minutor::save() {
  int w_top, w_left, w_right, w_bottom;
  WorldSave::findWorldBounds(mapview->getWorldPath(),
                             &w_top, &w_left, &w_bottom, &w_right);
  PngExport pngoptions;
  pngoptions.setBoundsFromChunks(w_top, w_left, w_bottom, w_right);
  pngoptions.exec();
  if (pngoptions.result() == QDialog::Rejected)
    return;

  // get filname to save to
  QFileDialog fileDialog(this);
  fileDialog.setDefaultSuffix("png");
  QString filename = fileDialog.getSaveFileName(this, tr("Save world as PNG"),
                                                QString(), "PNG Images (*.png)");

  // check if filename was given
  if (filename.isEmpty())
    return;

  // add .png suffix if not present
  QFile file(filename);
  QFileInfo fileinfo(file);
  if (fileinfo.suffix().isEmpty()) {
    filename.append(".png");
  }

  // save world to PNG image
  savePNG(filename, false,
          pngoptions.getRegionChecker(), pngoptions.getChunkChecker(),
          pngoptions.getTop(), pngoptions.getLeft(),
          pngoptions.getBottom(), pngoptions.getRight());
}

void Minutor::savePNG(QString filename, bool autoclose,
                      bool regionChecker, bool chunkChecker,
                      int w_top, int w_left, int w_bottom, int w_right) {
  progressAutoclose = autoclose;
  if (!filename.isEmpty()) {
    WorldSave *ws = new WorldSave(filename, mapview,
                                  regionChecker, chunkChecker,
                                  w_top, w_left, w_bottom, w_right);
    progress = new QProgressDialog();
    progress->setCancelButton(NULL);
    progress->setMaximum(100);
    progress->show();
    connect(ws, SIGNAL(progress(QString, double)),
            this, SLOT(saveProgress(QString, double)));
    connect(ws, SIGNAL(finished()),
            this, SLOT(saveFinished()));
    QThreadPool::globalInstance()->start(ws);
  }
}


void Minutor::saveProgress(QString status, double value) {
  progress->setValue(value*100);
  progress->setLabelText(status);
}

void Minutor::saveFinished() {
  progress->hide();
  delete progress;
  if (progressAutoclose)
    this->close();
}

void Minutor::closeWorld() {
  // clear "Jump to Player" menu
  locations.clear();
  for (int i = 0; i < playerActions.size(); i++) {
    m_ui.menu_JumpPlayer->removeAction(playerActions[i]);
    delete playerActions[i];
  }
  playerActions.clear();
  m_ui.menu_JumpPlayer->setEnabled(false);

  // clear "Dimensions" menu
  WorldInfo::Instance().clearDimensionsMenu(m_ui.menu_Dimension);
  // clear overlays
  mapview->clearOverlayItems();
  // clear other stuff
  currentWorld = QDir();
  emit worldLoaded(false);

  // clear "Structures Overlays"
  for (int i = 0; i < structureOverlayActions.size(); i++) {
    m_ui.menu_Overlay->removeAction(structureOverlayActions[i]);
    delete structureOverlayActions[i];
  }
  structureOverlayActions.clear();
  overlayItemTypes.clear();
}

void Minutor::jumpToLocation() {
  QAction *action = qobject_cast<QAction*>(sender());
  if (action) {
    Location loc = locations[action->data().toInt()];
    if ((loc.dimension == "minecraft:the_nether") && (m_ui.menu_Dimension[0].isEnabled())) {
      // dimMenu[0] defaults to Overworld
      loc.x *= 8;
      loc.z *= 8;
    }
    mapview->setLocation(loc.x, loc.z);
  }
}

void Minutor::jumpToXZ(int blockX, int blockZ) {
  mapview->setLocation(blockX, blockZ);
}

void Minutor::setViewLighting(bool value) {
  m_ui.action_Lighting->setChecked(value);
  toggleFlags();
}

void Minutor::setViewMobspawning(bool value) {
  m_ui.action_MobSpawning->setChecked(value);
  toggleFlags();
}

void Minutor::setViewCavemode(bool value) {
  m_ui.action_CaveMode->setChecked(value);
  toggleFlags();
}

void Minutor::setViewDepthshading(bool value) {
  m_ui.action_DepthShading->setChecked(value);
  toggleFlags();
}

void Minutor::setViewBiomeColors(bool value) {
  m_ui.action_BiomeColors->setChecked(value);
  toggleFlags();
}

void Minutor::setViewSeaGroundMode(bool value) {
  m_ui.action_SeaGround->setChecked(value);
  toggleFlags();
}

void Minutor::setViewSingleLayer(bool value) {
  m_ui.action_SingleLayer->setChecked(value);
  toggleFlags();
}

void Minutor::setViewSlimeChunks(bool value) {
  m_ui.action_SlimeChunks->setChecked(value);
  toggleFlags();
}

void Minutor::setViewInhabitedTime(bool value) {
  m_ui.action_InhabitedTime->setChecked(value);
  toggleFlags();
}

void Minutor::setDepth(int value) {
  depth->setValue(value);
}

void Minutor::toggleFlags() {
  int flags = 0;

  if (m_ui.action_Lighting->isChecked())      flags |= MapView::flgLighting;
  if (m_ui.action_MobSpawning->isChecked())   flags |= MapView::flgMobSpawn;
  if (m_ui.action_CaveMode->isChecked())      flags |= MapView::flgCaveMode;
  if (m_ui.action_DepthShading->isChecked())  flags |= MapView::flgDepthShading;
  if (m_ui.action_BiomeColors->isChecked())   flags |= MapView::flgBiomeColors;
  if (m_ui.action_SeaGround->isChecked())     flags |= MapView::flgSeaGround;
  if (m_ui.action_SingleLayer->isChecked())   flags |= MapView::flgSingleLayer;
  if (m_ui.action_SlimeChunks->isChecked())   flags |= MapView::flgSlimeChunks;
  if (m_ui.action_InhabitedTime->isChecked()) flags |= MapView::flgInhabitedTime;
  mapview->setFlags(flags);

  QSet<QString> overlayTypes;
  for (auto action : structureOverlayActions) {
    if (action->isChecked()) {
      overlayTypes.insert(action->data().toMap()["type"].toString());
    }
  }
  for (auto action : entityOverlayActions) {
    if (action->isChecked()) {
      overlayTypes.insert(action->data().toString());
    }
  }
  mapview->setVisibleOverlayItemTypes(overlayTypes);
  mapview->redraw();
}

void Minutor::viewDimension(QString dim_string)
{
  // when missing, add default namespace
  if (!dim_string.contains(":"))
    dim_string.insert(0,"minecraft:");
  if (dim_string.startsWith("minecraft:")) {
    // vanilla dimension
    const DimensionInfo & dim = DimensionIdentifier::Instance().getDimensionInfo(dim_string);
    if (dim.name != "Dummy Dimension") {
      viewDimension(dim);
      return;
    }
  }
  // custom dimension
  WorldInfo & wi(WorldInfo::Instance());
  const QList<DimensionInfo> & dimensions = wi.getDimensions();
  for (auto & dim: dimensions) {
    if (dim.name == dim_string) {
      viewDimension(dim);
      return;
    }
  }
}


void Minutor::viewDimension(const DimensionInfo &dim) {
  // update visability of Structure Overlays
  for (auto action : structureOverlayActions) {
    QString dimension = action->data().toMap()["dimension"].toString();
    if (dimension.isEmpty() ||
        !dimension.compare(dim.name, Qt::CaseInsensitive)) {
      action->setVisible(true);
    } else {
      action->setVisible(false);
    }
  }

  // change depth slider or adjust default Y when dimension is activated
  WorldInfo & wi(WorldInfo::Instance());
  if (wi.getDataVersion() < 2800 ) {
    // legacy versions before Cliffs & Caves (up to 1.17)
    depth->setRange(0, 255);
    dialogJumpTo->updateYrange(0, 255);
    if (dim.id == "minecraft:overworld") {
      depth->setValue(127);   // cloud level
    } else if (dim.id == "minecraft:the_nether") {
      depth->setValue(95);    // somewhere below Nether ceiling
    } else {
      depth->setValue(255);   // top
    }
  } else {
    // after Cliffs & Caves (1.18+)
    depth->setRange(dim.minY, dim.maxY);
    depth->setValue(dim.defaultY);
    dialogJumpTo->updateYrange(dim.minY, dim.maxY);
  }

  // ensure selected action is checked
  for (auto & action: m_ui.menu_Dimension->actions())
    if (action->text() == dim.name) {
      action->setChecked(true);
      break;
    }

  // clear current map & update scale
  QString path = QDir(currentWorld).absoluteFilePath(dim.path);
  mapview->setDimension(path, dim.scale);
}

void Minutor::about() {
  QMessageBox::about(this, tr("About %1").arg(qApp->applicationName()),
                     tr("<b>%1</b> v%2<br/>\n"
                        "&copy; Copyright %3, %4")
                     .arg(qApp->applicationName())
                     .arg(qApp->applicationVersion())
                     .arg("2010 - 2021")
                     .arg(qApp->organizationName()));
}

void Minutor::updateDimensions() {
  WorldInfo::Instance().getDimensionsInWorld(currentWorld, m_ui.menu_Dimension, this);
}

void Minutor::createActions() {
  // [File]
  connect(m_ui.action_Open, SIGNAL(triggered()),
          this,             SLOT(open()));

  connect(m_ui.action_Reload, SIGNAL(triggered()),
          this,               SLOT(reload()));
  connect(this,               SIGNAL(worldLoaded(bool)),
          m_ui.action_Reload, SLOT(setEnabled(bool)));

  connect(m_ui.action_SavePNG, SIGNAL(triggered()),
          this,                SLOT(save()));
  connect(this,                SIGNAL(worldLoaded(bool)),
          m_ui.action_SavePNG, SLOT(setEnabled(bool)));

  m_ui.action_Exit->setStatusTip(tr("Exit %1").arg(qApp->applicationName()));
  connect(m_ui.action_Exit, SIGNAL(triggered()),
          this,             SLOT(close()));

  // [View->Jump]
  connect(m_ui.action_JumpSpawn, SIGNAL(triggered()),
          this,                  SLOT(jumpToLocation()));
  connect(this,                  SIGNAL(worldLoaded(bool)),
          m_ui.action_JumpSpawn, SLOT(setEnabled(bool)));

  connect(m_ui.action_JumpTo, SIGNAL(triggered()),
          dialogJumpTo,       SLOT(show()));
  connect(this,               SIGNAL(worldLoaded(bool)),
          m_ui.action_JumpTo, SLOT(setEnabled(bool)));


  // [View->Modes]
  connect(m_ui.action_Lighting, SIGNAL(triggered()),
          this,                 SLOT(toggleFlags()));

  connect(m_ui.action_MobSpawning, SIGNAL(triggered()),
          this,                    SLOT(toggleFlags()));

  connect(m_ui.action_CaveMode, SIGNAL(triggered()),
          this,                 SLOT(toggleFlags()));

  connect(m_ui.action_DepthShading, SIGNAL(triggered()),
          this,                     SLOT(toggleFlags()));

  connect(m_ui.action_BiomeColors, SIGNAL(triggered()),
          this,                    SLOT(toggleFlags()));

  connect(m_ui.action_SeaGround, SIGNAL(triggered()),
          this,                  SLOT(toggleFlags()));

  connect(m_ui.action_SingleLayer, SIGNAL(triggered()),
          this,                    SLOT(toggleFlags()));

  connect(m_ui.action_SlimeChunks, SIGNAL(triggered()),
          this,                    SLOT(toggleFlags()));

  connect(m_ui.action_InhabitedTime, SIGNAL(triggered()),
          this,                      SLOT(toggleFlags()));

  // [View->Others]
//  m_ui.action_Refresh->setStatusTip(tr("Reloads all chunks, "
//                                       "but keeps the same position / dimension"));
  connect(m_ui.action_Refresh, SIGNAL(triggered()),
          mapview,             SLOT(clearCache()));

  // [Search]
  connect(m_ui.action_SearchEntity,   &QAction::triggered,
          this,                       &Minutor::openSearchEntityDialog);
  connect(this,                       &Minutor::worldLoaded,
          m_ui.action_SearchEntity,   &QAction::setEnabled);

  connect(m_ui.action_SearchBlock,    &QAction::triggered,
          this,                       &Minutor::openSearchBlockDialog);
  connect(this,                       &Minutor::worldLoaded,
          m_ui.action_SearchBlock,    &QAction::setEnabled);

  connect(m_ui.action_StatisticBlock, &QAction::triggered,
          this,                       &Minutor::openStatisticBlockDialog);
  connect(this,                       &Minutor::worldLoaded,
          m_ui.action_StatisticBlock, &QAction::setEnabled);

  // [Help]
  m_ui.action_About->setStatusTip(tr("About %1").arg(qApp->applicationName()));
  connect(m_ui.action_About, SIGNAL(triggered()),
          this,              SLOT(about()));

  m_ui.action_Settings->setStatusTip(tr("Change %1 Settings").arg(qApp->applicationName()));
  connect(m_ui.action_Settings, SIGNAL(triggered()),
          dialogSettings,       SLOT(show()));

  connect(m_ui.action_ManageDefinitions, SIGNAL(triggered()),
          dm,                            SLOT(show()));

  connect(dialogSettings, SIGNAL(checkForUpdates()),
          dm,             SLOT(checkForUpdates()));

}

// actionName will be modified, a "&" is added
QKeySequence Minutor::generateUniqueKeyboardShortcut(QString *actionName) {
  // generate a unique keyboard shortcut
  QKeySequence sequence;
  // test all letters in given name
  QString testName(*actionName);
  for (int ampPos = testName.indexOf(":") + 1; ampPos < testName.length(); ++ampPos) {
    QChar c = testName[ampPos];
    sequence = QKeySequence(QString("Ctrl+")+c);
    for (auto m : menuBar()->findChildren<QMenu*>()) {
      for (auto a : m->actions()) {
        if (a->shortcut() == sequence) {
          sequence = QKeySequence();
          break;
        }
      }
      if (sequence.isEmpty())
        break;  // already eliminated this as a possbility
    }
    if (!sequence.isEmpty()) {  // not eliminated, this one is ok
      *actionName = testName.mid(0, ampPos) + "&" + testName.mid(ampPos);
      break;
    }
  }
  return sequence;
}


void Minutor::createMenus() {
  // [File]
  m_ui.menu_OpenWorld->addActions(worldActions);
  if (worldActions.size() == 0)  // no worlds found
    m_ui.menu_OpenWorld->setEnabled(false);

  // [View]

  // [Overlay]
  separatorEntityOverlay    = new LabeledSeparator("Entity Overlays", this);
  separatorStructureOverlay = new LabeledSeparator("Structure Overlays", this);
  m_ui.menu_Overlay->addAction(separatorStructureOverlay);
  m_ui.menu_Overlay->addAction(separatorEntityOverlay);

  EntityIdentifier &ei = EntityIdentifier::Instance();
  for (auto it = ei.getCategoryList().constBegin();
       it != ei.getCategoryList().constEnd(); it++) {
    QString category = it->first;
    QColor  catcolor = it->second;

    QString actionName = category;
    QKeySequence sequence = generateUniqueKeyboardShortcut(&actionName);

    QPixmap pixmap(16, 16);
    QColor solidColor(catcolor);
    solidColor.setAlpha(255);
    pixmap.fill(solidColor);

    // create new QAction
    QAction* action = new QAction(pixmap, actionName, this);
    action->setShortcut(sequence);
    action->setStatusTip(tr("Toggle viewing of %1").arg(category));
    action->setEnabled(true);
    action->setData("Entity."+category);
    action->setCheckable(true);
    // put it into menu
    entityOverlayActions.push_back(action);
    m_ui.menu_Overlay->addAction(action); // add at bottom
    // connect handler
    connect(action, SIGNAL(triggered()),
            this,   SLOT(toggleFlags()));
  }

  // [Search]

  // [Help]
}

void Minutor::createStatusBar() {
  statusBar()->showMessage(tr("Ready"));
}

void Minutor::getWorldList() {
  QDir mc(dialogSettings->mcpath);
  if (mc.exists("saves"))
    mc.cd("saves");

  WorldInfo & wi(WorldInfo::Instance());
  QDirIterator it(mc);
  int key = 1;
  while (it.hasNext()) {
    it.next();
    if (it.fileInfo().isDir()) {
      if (wi.parseFolder(it.filePath())) {
        QAction *w = new QAction(this);
        w->setText(wi.getLevelName());
        w->setData(it.filePath());
        if (key < 10) {
          w->setShortcut("Ctrl+"+QString::number(key));
          key++;
        }
        connect(w, SIGNAL(triggered()),
                this, SLOT(openWorld()));
        worldActions.append(w);
      }
    }
  }
  wi.clear();
}

MapView *Minutor::getMapview() const
{
  return mapview;
}

void Minutor::loadWorld(QDir path) {
  // cleanup current state (just in case)
  closeWorld();
  currentWorld = path;

  WorldInfo & wi(WorldInfo::Instance());
  wi.parseFolder(path);
  wi.parseDimensions();

  // add level name to window title
  setWindowTitle(qApp->applicationName() + " - " + wi.getLevelName());

  // Jump to: world spawn
  m_ui.action_JumpSpawn->setData(locations.count());
  locations.append(Location(wi.getSpawnX(), wi.getSpawnZ() ));

  // Jump to: known players
  if (path.cd("playerdata") || path.cd("players")) {
    QDirIterator it(path.absolutePath(), {"*.dat"}, QDir::Files);
    bool hasPlayers = false;
    while (it.hasNext()) {
      it.next();
      if (it.fileInfo().isFile()) {
        hasPlayers = true;
        NBT player(it.filePath());

        // player name from file name (old)
        QString playerName = it.fileInfo().completeBaseName();
        if (path.dirName() == "playerdata") {
          // player name via UUID
          QRegExp id("[0-9a-z]{8,8}\\-[0-9a-z]{4,4}\\-[0-9a-z]{4,4}"
                     "\\-[0-9a-z]{4,4}\\-[0-9a-z]{12,12}");
          if (id.exactMatch(playerName)) {
            QSettings settings;
            if (settings.contains("PlayerCache/"+playerName)) {
              playerName = settings.value("PlayerCache/"+playerName, playerName).toString();
            } else if (playerName[14]=='4') {
              // only version 4 UUIDs can be resolved at Mojang API
              // trigger HTTPS request to get player name
              QString url = playerName;
              url = "https://api.mojang.com/user/profiles/" + url.remove('-') + "/names";

              QNetworkRequest request;
              request.setUrl(QUrl(url));
              request.setRawHeader("User-Agent", "Minutor");
              connect(&this->qnam, &QNetworkAccessManager::finished,
                      this,        &Minutor::updatePlayerCache);
              this->qnam.get(request);
            }
          } else continue;
        }

        // current position of player
        auto pos = player.at("Pos");
        double posX = pos->at(0)->toDouble();
        double posZ = pos->at(2)->toDouble();
        // current dimension
        QString dimension;
        auto dim = player.at("Dimension");
        if (dynamic_cast<const Tag_Int*>(dim)) {
          // in very old versions this was an Tag_Int
          switch (dim->toInt()) {
            case -1: dimension = "minecraft:the_nether"; break;
            case +1: dimension = "minecraft:the_end"; break;
            default: dimension = "minecraft:overworld";
          }
        } if (dynamic_cast<const Tag_String*>(dim)) {
          // now dimension is given as string
          dimension = dim->toString();
        }
        QAction *p = new QAction(this);
        p->setText(playerName);
        p->setData(locations.count());
        locations.append(Location(posX, posZ, dimension));
        connect(p, SIGNAL(triggered()),
                this, SLOT(jumpToLocation()));
        playerActions.append(p);

        // spawn location (bed)
        if (player.has("SpawnX")) {
          dimension = "minecraft:overworld";
          if (player.has("SpawnDimension"))
            dimension = player.at("SpawnDimension")->toString();
          p = new QAction(this);
          p->setText(playerName+"'s Bed");
          p->setData(locations.count());
          locations.append(Location(player.at("SpawnX")->toDouble(),
                                    player.at("SpawnZ")->toDouble(),
                                    dimension));
          connect(p, SIGNAL(triggered()),
                  this, SLOT(jumpToLocation()));
          playerActions.append(p);
        }
      }
    }
    m_ui.menu_JumpPlayer->addActions(playerActions);
    m_ui.menu_JumpPlayer->setEnabled(hasPlayers);
    path.cdUp();
  }

  if (path.cd("data")) {
    loadStructures(path);
    path.cdUp();
  }

  // create Dimensions menu
  WorldInfo::Instance().getDimensionsInWorld(path, m_ui.menu_Dimension, this);

  // finalize
  emit worldLoaded(true);
  mapview->setLocation(locations.first().x, locations.first().z);
  toggleFlags();
}

void Minutor::updatePlayerCache(QNetworkReply* reply) {
  auto response = reply->readAll();
  if (response.length() > 0) {
    // we got a response
    auto json = QJsonDocument::fromJson(response).array();
    if (json.size() > 0) {
      // at least one entry available, last one is the current one
      QJsonObject name0 = json.at(json.size()-1).toObject();
      if (name0.contains("name")) {
        // reconstruct player UUID
        QString playerUUID = reply->url().path().mid(15, 32);
        playerUUID.insert(20, '-');
        playerUUID.insert(16, '-');
        playerUUID.insert(12, '-');
        playerUUID.insert(8, '-');
        // store in player cache
        QSettings settings;
        QString playerName = name0.value("name").toString();
        settings.setValue("PlayerCache/"+playerUUID, playerName);
      }
    }
  }

  reply->deleteLater();
}

void Minutor::rescanWorlds() {
  worldActions.clear();
  getWorldList();
  m_ui.menu_OpenWorld->clear();
  m_ui.menu_OpenWorld->addActions(worldActions);
  m_ui.menu_OpenWorld->setEnabled(worldActions.count() != 0);
  // we don't care about the auto-update toggle, since that only happens
  // on startup anyway.
}

void Minutor::addOverlayItemType(QString type, QColor color,
                                 QString dimension) {
  if (!overlayItemTypes.contains(type)) {
    overlayItemTypes.insert(type);
    QList<QString> path = type.split('.');
    QList<QString>::const_iterator pathIt, nextIt, endPathIt = path.end();
    nextIt = path.begin();
    nextIt++;  // skip first part
    pathIt = nextIt++;
    QMenu* cur = m_ui.menu_Overlay;

    // generate a nested menu structure to match the path
    while (nextIt != endPathIt) {
      QList<QMenu*> results =
          cur->findChildren<QMenu*>(*pathIt, Qt::FindDirectChildrenOnly);
      if (results.empty()) {
        cur = cur->addMenu("&" + *pathIt);
        cur->setObjectName(*pathIt);
      } else {
        cur->addMenu( results.front() );
      }
      pathIt = ++nextIt;
    }
    // generate a unique keyboard shortcut
    QString actionName = path.last();
    QKeySequence sequence = generateUniqueKeyboardShortcut(&actionName);

    QPixmap pixmap(16, 16);
    QColor solidColor(color);
    solidColor.setAlpha(255);
    pixmap.fill(solidColor);

    QMap<QString, QVariant> entityData;
    entityData["type"] = type;
    entityData["dimension"] = dimension;

    structureOverlayActions.push_back(new QAction(pixmap, actionName, this));
    structureOverlayActions.last()->setShortcut(sequence);
    structureOverlayActions.last()->setStatusTip(tr("Toggle viewing of %1")
                                          .arg(type));
    structureOverlayActions.last()->setEnabled(true);
    structureOverlayActions.last()->setData(entityData);
    structureOverlayActions.last()->setCheckable(true);
    // insert before Entitys
    cur->insertAction(separatorEntityOverlay, structureOverlayActions.last());
    connect(structureOverlayActions.last(), SIGNAL(triggered()),
            this, SLOT(toggleFlags()));
  }
}

void Minutor::addOverlayItem(QSharedPointer<OverlayItem> item) {
  // create menu entries (if necessary)
  addOverlayItemType(item->type(), item->color(), item->dimension());

//  const OverlayItem::Point& p = item->midpoint();
//  overlayItems[item->type()].insertMulti(QPair<int, int>(p.x, p.z), item);

  mapview->addOverlayItem(item);
}

void Minutor::showProperties(QVariant props) {
  if (!props.isNull()) {
    propView->DisplayProperties(props);
    propView->show();
  }
}

SearchChunksDialog* Minutor::prepareSearchForm(const QSharedPointer<SearchPluginI>& searchPlugin) {
  SearchChunksDialog* form = new SearchChunksDialog(searchPlugin);

  form->setAttribute(Qt::WA_DeleteOnClose);

  const auto currentLocation = mapview->getLocation();
  form->setSearchCenter(currentLocation->x, currentLocation->y, currentLocation->z);

  connect(mapview, SIGNAL(coordinatesChanged(int,int,int)),
          form, SLOT(setSearchCenter(int,int,int))
          );

  connect(form, SIGNAL(jumpTo(QVector3D)),
          this, SLOT(triggerJumpToPosition(QVector3D))
          );

  connect(form, SIGNAL(updateSearchResultPositions(QVector<QSharedPointer<OverlayItem> >)),
          this, SLOT(updateSearchResultPositions(QVector<QSharedPointer<OverlayItem> >))
          );

  return form;
}

void Minutor::openSearchEntityDialog() {
  auto searchPlugin = QSharedPointer<SearchEntityPlugin>::create();
  auto searchEntityForm = prepareSearchForm(searchPlugin);
  searchEntityForm->setWindowTitle(m_ui.action_SearchEntity->statusTip());
  searchEntityForm->showNormal();
}

void Minutor::openSearchBlockDialog() {
  auto searchPlugin = QSharedPointer<SearchBlockPlugin>::create();
  auto searchBlockForm = prepareSearchForm(searchPlugin);
  searchBlockForm->setWindowTitle(m_ui.action_SearchBlock->statusTip());
  searchBlockForm->showNormal();
}

void Minutor::openStatisticBlockDialog() {
  StatisticDialog *dialog = new StatisticDialog(this);
  dialog->setWindowTitle(m_ui.action_StatisticBlock->statusTip());
  dialog->showNormal();
}

void Minutor::triggerJumpToPosition(QVector3D pos) {
  mapview->setLocation(pos.x(), pos.y(), pos.z(), true, false);
}

void Minutor::updateSearchResultPositions(QVector<QSharedPointer<OverlayItem> > items) {
  mapview->updateSearchResultPositions(items);
  mapview->redraw();
}

void Minutor::loadStructures(const QDir &dataPath) {
  // attempt to parse all of the files in the data directory, looking for
  // generated structures
  for (auto &fileName : dataPath.entryList(QStringList() << "*.dat")) {
    NBT file(dataPath.filePath(fileName));
    auto data = file.at("data");

    auto items = GeneratedStructure::tryParseDatFile(data);
    for (auto &item : items) {
      addOverlayItem(item);
    }

    if (items.isEmpty()) {
      // try parsing it as a villages.dat file
      int underidx = fileName.lastIndexOf('_');
      int dotidx = fileName.lastIndexOf('.');
      QString dimension = "overworld";
      if (underidx > 0) {
        dimension = fileName.mid(underidx + 1, dotidx - underidx - 1);
      }
      items = Village::tryParseDatFile(data, dimension);

      for (auto &item : items) {
        addOverlayItem(item);
      }
    }
  }
}
