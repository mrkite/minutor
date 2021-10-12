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
#include "./minutor.h"
#include "./mapview.h"
#include "./labelledslider.h"
#include "./nbt.h"
#include "./json.h"
#include "./definitionmanager.h"
#include "./entityidentifier.h"
#include "./settings.h"
#include "./dimensionidentifier.h"
#include "./worldsave.h"
#include "./properties.h"
#include "./generatedstructure.h"
#include "./village.h"
#include "./jumpto.h"
#include "./pngexport.h"
#include "./searchchunkswidget.h"
#include "./searchentitypluginwidget.h"
#include "./searchblockpluginwidget.h"
#include "./searchresultwidget.h"

Minutor::Minutor()
  : searchMenu(nullptr)
  , searchEntityAction(nullptr)
  , searchBlockAction(nullptr)
{
  mapview = new MapView;
  connect(mapview, SIGNAL(hoverTextChanged(QString)),
          statusBar(), SLOT(showMessage(QString)));
  connect(mapview, SIGNAL(showProperties(QVariant)),
          this, SLOT(showProperties(QVariant)));
  connect(mapview, SIGNAL(addOverlayItemType(QString, QColor)),
          this, SLOT(addOverlayItemType(QString, QColor)));
  dm = new DefinitionManager(this);
  mapview->attach(dm);
  connect(dm,   SIGNAL(packsChanged()),
          this, SLOT(updateDimensions()));
  DimensionIdentifier *dimensions = &DimensionIdentifier::Instance();
  connect(dimensions, SIGNAL(dimensionChanged(const DimensionInfo &)),
          this, SLOT(viewDimension(const DimensionInfo &)));
  settings = new Settings(this);
  connect(settings, SIGNAL(settingsUpdated()),
          this, SLOT(rescanWorlds()));
  jumpTo = new JumpTo(this);

  if (settings->autoUpdate) {
    // get time of last update
    QSettings settings;
    QDateTime lastUpdateTime = settings.value("packupdate").toDateTime();

    // auto-update only once a week
    if (lastUpdateTime.addDays(7) < QDateTime::currentDateTime())
      dm->autoUpdate();
  }

  createActions();
  createMenus();
  createStatusBar();

  QBoxLayout *mainLayout;
  if (settings->verticalDepth) {
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
  connect(mapview, SIGNAL(demandDepthChange(int)),
          depth, SLOT(changeValue(int)));
  connect(mapview, SIGNAL(demandDepthValue(int)),
          depth, SLOT(setValue(int)));
  connect(this, SIGNAL(worldLoaded(bool)),
          mapview, SLOT(setEnabled(bool)));
  connect(this, SIGNAL(worldLoaded(bool)),
          depth, SLOT(setEnabled(bool)));

  QWidget *central = new QWidget;
  central->setLayout(mainLayout);

  setCentralWidget(central);
  layout()->setContentsMargins(0, 0, 0, 0);

  setWindowTitle(qApp->applicationName());

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
  WorldSave::findBounds(mapview->getWorldPath(),
                        &w_top, &w_left, &w_bottom, &w_right);
  PngExport pngoptions;
  pngoptions.setBounds(w_top, w_left, w_bottom, w_right);
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
  // clear jump menu
  locations.clear();
  for (int i = 0; i < playerActions.size(); i++) {
    jumpMenu->removeAction(playerActions[i]);
    delete playerActions[i];
  }
  playerActions.clear();
  jumpMenu->setEnabled(false);
  // clear dimensions menu
  DimensionIdentifier::Instance().removeDimensions(dimMenu);
  // clear overlays
  mapview->clearOverlayItems();
  // clear other stuff
  currentWorld = QString();
  emit worldLoaded(false);
}

void Minutor::jumpToLocation() {
  QAction *action = qobject_cast<QAction*>(sender());
  if (action) {
    Location loc = locations[action->data().toInt()];
    if ((loc.dimension == "minecraft:the_nether") && (this->dimMenu[0].isEnabled())) {
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
  lightingAct->setChecked(value);
  toggleFlags();
}

void Minutor::setViewMobspawning(bool value) {
  mobSpawnAct->setChecked(value);
  toggleFlags();
}

void Minutor::setViewCavemode(bool value) {
  caveModeAct->setChecked(value);
  toggleFlags();
}

void Minutor::setViewDepthshading(bool value) {
  depthShadingAct->setChecked(value);
  toggleFlags();
}

void Minutor::setViewBiomeColors(bool value) {
  biomeColorsAct->setChecked(value);
  toggleFlags();
}

void Minutor::setViewSeaGroundMode(bool value) {
  seaGroundAct->setChecked(value);
  toggleFlags();
}

void Minutor::setViewSingleLayer(bool value) {
  singleLayerAct->setChecked(value);
  toggleFlags();
}

void Minutor::setDepth(int value) {
  depth->setValue(value);
}

void Minutor::toggleFlags() {
  int flags = 0;

  if (lightingAct->isChecked())     flags |= MapView::flgLighting;
  if (mobSpawnAct->isChecked())     flags |= MapView::flgMobSpawn;
  if (caveModeAct->isChecked())     flags |= MapView::flgCaveMode;
  if (depthShadingAct->isChecked()) flags |= MapView::flgDepthShading;
  if (biomeColorsAct->isChecked())  flags |= MapView::flgBiomeColors;
  if (seaGroundAct->isChecked())    flags |= MapView::flgSeaGround;
  if (singleLayerAct->isChecked())  flags |= MapView::flgSingleLayer;
  mapview->setFlags(flags);

  QSet<QString> overlayTypes;
  for (auto action : structureActions) {
    if (action->isChecked()) {
      overlayTypes.insert(action->data().toMap()["type"].toString());
    }
  }
  for (auto action : entityActions) {
    if (action->isChecked()) {
      overlayTypes.insert(action->data().toString());
    }
  }
  mapview->setVisibleOverlayItemTypes(overlayTypes);
  mapview->redraw();
}

void Minutor::viewDimension(const DimensionInfo &dim) {
  for (auto action : structureActions) {
    QString dimension = action->data().toMap()["dimension"].toString();
    if (dimension.isEmpty() ||
        !dimension.compare(dim.name, Qt::CaseInsensitive)) {
      action->setVisible(true);
    } else {
      action->setVisible(false);
    }
  }
  mapview->setDimension(dim.path, dim.scale);
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
  DimensionIdentifier::Instance().getDimensions(currentWorld, dimMenu, this);
}

void Minutor::createActions() {
  getWorldList();

  // [File]
  openAct = new QAction(tr("&Open..."), this);
  openAct->setShortcut(tr("Ctrl+O"));
  openAct->setStatusTip(tr("Open a world"));
  connect(openAct, SIGNAL(triggered()),
          this,    SLOT(open()));

  reloadAct = new QAction(tr("&Reload"), this);
  reloadAct->setShortcut(tr("F5"));
  reloadAct->setStatusTip(tr("Reload current world"));
  connect(reloadAct, SIGNAL(triggered()),
          this,      SLOT(reload()));
  connect(this,      SIGNAL(worldLoaded(bool)),
          reloadAct, SLOT(setEnabled(bool)));

  saveAct = new QAction(tr("&Save PNG..."), this);
  saveAct->setShortcut(tr("Ctrl+S"));
  saveAct->setStatusTip(tr("Save as PNG"));
  connect(saveAct, SIGNAL(triggered()),
          this,    SLOT(save()));
  connect(this,    SIGNAL(worldLoaded(bool)),
          saveAct, SLOT(setEnabled(bool)));

  exitAct = new QAction(tr("E&xit"), this);
  exitAct->setShortcut(tr("Ctrl+Q"));
  exitAct->setStatusTip(tr("Exit %1").arg(qApp->applicationName()));
  connect(exitAct, SIGNAL(triggered()),
          this,    SLOT(close()));

  // [View->Jump]
  jumpSpawnAct = new QAction(tr("Jump to &Spawn"), this);
  jumpSpawnAct->setShortcut(tr("F1"));
  jumpSpawnAct->setStatusTip(tr("Jump to world spawn"));
  connect(jumpSpawnAct, SIGNAL(triggered()),
          this,         SLOT(jumpToLocation()));
  connect(this,         SIGNAL(worldLoaded(bool)),
          jumpSpawnAct, SLOT(setEnabled(bool)));

  jumpToAct = new QAction(tr("&Jump To"), this);
  jumpToAct->setShortcut(tr("CTRL+J"));
  jumpToAct->setStatusTip(tr("Jump to a location"));
  connect(jumpToAct, SIGNAL(triggered()),
          jumpTo,    SLOT(show()));
  connect(this,      SIGNAL(worldLoaded(bool)),
          jumpToAct, SLOT(setEnabled(bool)));


  // [View->Modes]
  lightingAct = new QAction(tr("&Lighting"), this);
  lightingAct->setCheckable(true);
  lightingAct->setShortcut(tr("Ctrl+L"));
  lightingAct->setStatusTip(tr("Toggle lighting on/off"));
  connect(lightingAct, SIGNAL(triggered()),
          this,        SLOT(toggleFlags()));

  mobSpawnAct = new QAction(tr("&Mob spawning"), this);
  mobSpawnAct->setCheckable(true);
  mobSpawnAct->setShortcut(tr("Ctrl+M"));
  mobSpawnAct->setStatusTip(tr("Toggle show mob spawning on/off"));
  connect(mobSpawnAct, SIGNAL(triggered()),
          this,        SLOT(toggleFlags()));

  caveModeAct = new QAction(tr("&Cave Mode"), this);
  caveModeAct->setCheckable(true);
  caveModeAct->setShortcut(tr("Ctrl+C"));
  caveModeAct->setStatusTip(tr("Toggle cave mode on/off"));
  connect(caveModeAct, SIGNAL(triggered()),
          this,        SLOT(toggleFlags()));

  depthShadingAct = new QAction(tr("&Depth shading"), this);
  depthShadingAct->setCheckable(true);
  depthShadingAct->setShortcut(tr("Ctrl+D"));
  depthShadingAct->setStatusTip(tr("Toggle shading based on relative depth"));
  connect(depthShadingAct, SIGNAL(triggered()),
          this,            SLOT(toggleFlags()));

  biomeColorsAct = new QAction(tr("&Biome Colors"), this);
  biomeColorsAct->setCheckable(true);
  biomeColorsAct->setShortcut(tr("Ctrl+B"));
  biomeColorsAct->setStatusTip(tr("Toggle draw biome colors or block colors"));
  connect(biomeColorsAct, SIGNAL(triggered()),
          this,           SLOT(toggleFlags()));
  
  seaGroundAct = new QAction(tr("Sea &Ground Mode"), this);
  seaGroundAct->setCheckable(true);
  seaGroundAct->setShortcut(tr("Ctrl+G"));
  seaGroundAct->setStatusTip(tr("Toggle sea ground mode on/off"));
  connect(seaGroundAct, SIGNAL(triggered()),
          this,           SLOT(toggleFlags()));

  singleLayerAct = new QAction(tr("Single Layer"), this);
  singleLayerAct->setCheckable(true);
  //singleLayerAct->setShortcut(tr("Ctrl+L"));  // both S and L are already used
  singleLayerAct->setStatusTip(tr("Toggle single layer on/off"));
  connect(singleLayerAct, SIGNAL(triggered()),
          this,           SLOT(toggleFlags()));

  // [View->Others]
  refreshAct = new QAction(tr("Refresh"), this);
  refreshAct->setShortcut(tr("F2"));
  refreshAct->setStatusTip(tr("Reloads all chunks, "
                              "but keeps the same position / dimension"));
  connect(refreshAct, SIGNAL(triggered()),
          mapview,    SLOT(clearCache()));

  manageDefsAct = new QAction(tr("Manage &Definitions..."), this);
  manageDefsAct->setStatusTip(tr("Manage block and biome definitions"));
  connect(manageDefsAct, SIGNAL(triggered()),
          dm,            SLOT(show()));

  // [Help]
  aboutAct = new QAction(tr("&About"), this);
  aboutAct->setStatusTip(tr("About %1").arg(qApp->applicationName()));
  connect(aboutAct, SIGNAL(triggered()),
          this,     SLOT(about()));

  settingsAct = new QAction(tr("Settings..."), this);
  settingsAct->setStatusTip(tr("Change %1 Settings")
                            .arg(qApp->applicationName()));
  connect(settingsAct, SIGNAL(triggered()),
          settings,    SLOT(show()));

  updatesAct = new QAction(tr("Check for updates..."), this);
  updatesAct->setStatusTip(tr("Check for updated packs"));
  connect(updatesAct, SIGNAL(triggered()),
          dm,         SLOT(checkForUpdates()));

  searchEntityAction = new QAction(tr("Search entity"), this);
  connect(searchEntityAction, SIGNAL(triggered()), this, SLOT(searchEntity()));

  searchBlockAction = new QAction(tr("Search block"), this);
  connect(searchBlockAction, SIGNAL(triggered()), this, SLOT(searchBlock()));
}

// actionName will be modified, a "&" is added
QKeySequence Minutor::generateUniqueKeyboardShortcut(QString *actionName) {
  // generate a unique keyboard shortcut
  QKeySequence sequence;
  // test all letters in given name
  QString testName(*actionName);
  for (int ampPos=0; ampPos < testName.length(); ++ampPos) {
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

void Minutor::populateEntityOverlayMenu() {
  EntityIdentifier &ei = EntityIdentifier::Instance();
  for (auto it = ei.getCategoryList().constBegin();
       it != ei.getCategoryList().constEnd(); it++) {
    QString category = it->first;
    QColor catcolor = it->second;

    QString actionName = category;
    QKeySequence sequence = generateUniqueKeyboardShortcut(&actionName);

    QPixmap pixmap(16, 16);
    QColor solidColor(catcolor);
    solidColor.setAlpha(255);
    pixmap.fill(solidColor);

    entityActions.push_back(new QAction(pixmap, actionName, this));
    entityActions.last()->setShortcut(sequence);
    entityActions.last()->setStatusTip(tr("Toggle viewing of %1")
                                       .arg(category));
    entityActions.last()->setEnabled(true);
    entityActions.last()->setData("Entity."+category);
    entityActions.last()->setCheckable(true);
    entityOverlayMenu->addAction(entityActions.last());
    connect(entityActions.last(), SIGNAL(triggered()),
            this, SLOT(toggleFlags()));
  }
}


void Minutor::createMenus() {
  // [File]
  fileMenu = menuBar()->addMenu(tr("&File"));
  worldMenu = fileMenu->addMenu(tr("&Open World"));

  worldMenu->addActions(worlds);
  if (worlds.size() == 0)  // no worlds found
    worldMenu->setEnabled(false);

  fileMenu->addAction(openAct);
  fileMenu->addAction(reloadAct);
  fileMenu->addSeparator();
  fileMenu->addAction(saveAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  // [View]
  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(jumpSpawnAct);
  viewMenu->addAction(jumpToAct);
  jumpMenu = viewMenu->addMenu(tr("&Jump to Player"));
  jumpMenu->setEnabled(false);
  dimMenu = viewMenu->addMenu(tr("&Dimension"));
  dimMenu->setEnabled(false);
  // [View->Modes]
  viewMenu->addSeparator();
  viewMenu->addAction(lightingAct);
  viewMenu->addAction(mobSpawnAct);
  viewMenu->addAction(caveModeAct);
  viewMenu->addAction(depthShadingAct);
  viewMenu->addAction(biomeColorsAct);
  viewMenu->addAction(seaGroundAct);
  viewMenu->addAction(singleLayerAct);
  // [View->Overlay]
  viewMenu->addSeparator();
  structureOverlayMenu = viewMenu->addMenu(tr("&Structure Overlay"));
  entityOverlayMenu    = viewMenu->addMenu(tr("&Entity Overlay"));
  populateEntityOverlayMenu();

  viewMenu->addSeparator();
  viewMenu->addAction(refreshAct);
  viewMenu->addAction(manageDefsAct);

  //menuBar()->addSeparator();

  // [Search]
  searchMenu = menuBar()->addMenu(tr("&Search"));
  searchMenu->addAction(searchEntityAction);
  searchMenu->addAction(searchBlockAction);

  // [Help]
  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAct);
  helpMenu->addSeparator();
  helpMenu->addAction(settingsAct);
  helpMenu->addAction(updatesAct);
}

void Minutor::createStatusBar() {
  statusBar()->showMessage(tr("Ready"));
}

QString Minutor::getWorldName(QDir path) {
  if (!path.exists("level.dat"))  // no level.dat?  no world
    return QString();

  NBT level(path.filePath("level.dat"));
  return level.at("Data")->at("LevelName")->toString();
}


void Minutor::getWorldList() {
  QDir mc(settings->mcpath);
  if (!mc.cd("saves"))
    return;

  QDirIterator it(mc);
  int key = 1;
  while (it.hasNext()) {
    it.next();
    if (it.fileInfo().isDir()) {
      QString name = getWorldName(it.filePath());
      if (!name.isNull()) {
        QAction *w = new QAction(this);
        w->setText(name);
        w->setData(it.filePath());
        if (key < 10) {
          w->setShortcut("Ctrl+"+QString::number(key));
          key++;
        }
        connect(w, SIGNAL(triggered()),
                this, SLOT(openWorld()));
        worlds.append(w);
      }
    }
  }
}

MapView *Minutor::getMapview() const
{
  return mapview;
}

void Minutor::loadWorld(QDir path) {
  // cleanup current state (just in case)
  closeWorld();
  currentWorld = path;

  NBT level(path.filePath("level.dat"));

  auto data = level.at("Data");
  // add level name to window title
  setWindowTitle(qApp->applicationName() + " - " +
                 data->at("LevelName")->toString());
  // save world spawn
  jumpSpawnAct->setData(locations.count());
  locations.append(Location(data->at("SpawnX")->toDouble(),
                            data->at("SpawnZ")->toDouble()));
  // show saved players
  if (path.cd("playerdata") || path.cd("players")) {
    QDirIterator it(path.absolutePath(), {"*.dat"}, QDir::Files);
    bool hasPlayers = false;
    while (it.hasNext()) {
      it.next();
      if (it.fileInfo().isFile()) {
        hasPlayers = true;
        NBT player(it.filePath());

        // player name
        QString playerName = it.fileInfo().completeBaseName();
        QRegExp id("[0-9a-z]{8,8}\\-[0-9a-z]{4,4}\\-[0-9a-z]{4,4}"
                   "\\-[0-9a-z]{4,4}\\-[0-9a-z]{12,12}");
        if (id.exactMatch(playerName)) {
          playerName = QString("Player %1").arg(playerActions.length());
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
    jumpMenu->addActions(playerActions);
    jumpMenu->setEnabled(hasPlayers);
    path.cdUp();
  }

  if (path.cd("data")) {
    loadStructures(path);
    path.cdUp();
  }

  // show dimensions
  DimensionIdentifier::Instance().getDimensions(path, dimMenu, this);
  emit worldLoaded(true);
  mapview->setLocation(locations.first().x, locations.first().z);
  toggleFlags();
}

void Minutor::rescanWorlds() {
  worlds.clear();
  getWorldList();
  worldMenu->clear();
  worldMenu->addActions(worlds);
  worldMenu->setEnabled(worlds.count() != 0);
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
    QMenu* cur = structureOverlayMenu;

    // generate a nested menu structure to match the path
    while (nextIt != endPathIt) {
      QList<QMenu*> results =
          cur->findChildren<QMenu*>(*pathIt, Qt::FindDirectChildrenOnly);
      if (results.empty()) {
        cur = cur->addMenu("&" + *pathIt);
        cur->setObjectName(*pathIt);
      } else {
        cur = results.front();
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

    structureActions.push_back(new QAction(pixmap, actionName, this));
    structureActions.last()->setShortcut(sequence);
    structureActions.last()->setStatusTip(tr("Toggle viewing of %1")
                                          .arg(type));
    structureActions.last()->setEnabled(true);
    structureActions.last()->setData(entityData);
    structureActions.last()->setCheckable(true);
    cur->addAction(structureActions.last());
    connect(structureActions.last(), SIGNAL(triggered()),
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

SearchChunksWidget* Minutor::prepareSearchForm(const QSharedPointer<SearchPluginI>& searchPlugin) {
  SearchChunksWidget* form = new SearchChunksWidget(searchPlugin);

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

void Minutor::searchBlock() {
  auto searchPlugin = QSharedPointer<SearchBlockPluginWidget>::create();
  auto searchBlockForm = prepareSearchForm(searchPlugin);
  searchBlockForm->showNormal();
}

void Minutor::searchEntity() {
  auto searchPlugin = QSharedPointer<SearchEntityPluginWidget>::create();
  auto searchEntityForm = prepareSearchForm(searchPlugin);
  searchEntityForm->showNormal();
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
