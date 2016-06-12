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

Minutor::Minutor(): maxentitydistance(0) {
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
  dimensions = dm->dimensionIdentifer();
  connect(dimensions, SIGNAL(dimensionChanged(const DimensionInfo &)),
          this, SLOT(viewDimension(const DimensionInfo &)));
  settings = new Settings(this);
  connect(settings, SIGNAL(settingsUpdated()),
          this, SLOT(rescanWorlds()));


  if (settings->autoUpdate)
    dm->autoUpdate();

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
  loadWorld(currentWorld);
}

void Minutor::save() {
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
  savePNG(filename, false, false, false);
}

void Minutor::savePNG(QString filename, bool autoclose, bool regionChecker,
                      bool chunkChecker) {
  progressAutoclose = autoclose;
  if (!filename.isEmpty()) {
    WorldSave *ws = new WorldSave(filename, mapview, regionChecker,
                                  chunkChecker);
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
  locations.clear();
  for (int i = 0; i < players.size(); i++) {
    jumpMenu->removeAction(players[i]);
    delete players[i];
  }
  players.clear();
  jumpMenu->setEnabled(false);
  dimensions->removeDimensions(dimMenu);
  currentWorld = QString();
  emit worldLoaded(false);
}

void Minutor::jumpToLocation() {
  QAction *action = qobject_cast<QAction*>(sender());
  if (action) {
    Location loc = locations[action->data().toInt()];
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
  depthShadingAct->setChecked(value);
  toggleFlags();
}

void Minutor::setViewDepthshading(bool value) {
  lightingAct->setChecked(value);
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
  mapview->showOverlayItemTypes(overlayTypes);
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
                     .arg(2016)
                     .arg(qApp->organizationName()));
}

void Minutor::updateDimensions() {
  dimensions->getDimensions(currentWorld, dimMenu, this);
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

  // [View]
  jumpSpawnAct = new QAction(tr("Jump to &Spawn"), this);
  jumpSpawnAct->setShortcut(tr("F1"));
  jumpSpawnAct->setStatusTip(tr("Jump to world spawn"));
  connect(jumpSpawnAct, SIGNAL(triggered()),
          this,         SLOT(jumpToLocation()));
  connect(this,         SIGNAL(worldLoaded(bool)),
          jumpSpawnAct, SLOT(setEnabled(bool)));


  lightingAct = new QAction(tr("&Lighting"), this);
  lightingAct->setCheckable(true);
  lightingAct->setShortcut(tr("Ctrl+L"));
  lightingAct->setStatusTip(tr("Toggle lighting on/off"));
  connect(lightingAct, SIGNAL(triggered()),
          this,        SLOT(toggleFlags()));

  depthShadingAct = new QAction(tr("&Depth shading"), this);
  depthShadingAct->setCheckable(true);
  depthShadingAct->setShortcut(tr("Ctrl+D"));
  depthShadingAct->setStatusTip(tr("Toggle shading based on relative depth"));
  connect(depthShadingAct, SIGNAL(triggered()),
          this,            SLOT(toggleFlags()));

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
  caveModeAct->setEnabled(false);

  // [View->Entity Overlay]

  manageDefsAct = new QAction(tr("Manage &Definitions..."), this);
  manageDefsAct->setStatusTip(tr("Manage block and biome definitions"));
  connect(manageDefsAct, SIGNAL(triggered()),
          dm,            SLOT(show()));

  refreshAct = new QAction(tr("Refresh"), this);
  refreshAct->setShortcut(tr("F2"));
  refreshAct->setStatusTip(tr("Reloads all chunks, "
                              "but keeps the same position / dimension"));
  connect(refreshAct, SIGNAL(triggered()),
          mapview,    SLOT(clearCache()));

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
  jumpMenu = viewMenu->addMenu(tr("&Jump to Player"));
  jumpMenu->setEnabled(false);
  dimMenu = viewMenu->addMenu(tr("&Dimension"));
  dimMenu->setEnabled(false);
  viewMenu->addSeparator();
  viewMenu->addAction(lightingAct);
  viewMenu->addAction(mobSpawnAct);
  viewMenu->addAction(caveModeAct);
  viewMenu->addAction(depthShadingAct);
  // [View->Overlay]
  structureOverlayMenu = viewMenu->addMenu(tr("&Structure Overlay"));
  entityOverlayMenu    = viewMenu->addMenu(tr("&Entity Overlay"));
  populateEntityOverlayMenu();

  viewMenu->addSeparator();
  viewMenu->addAction(refreshAct);
  viewMenu->addSeparator();
  viewMenu->addAction(manageDefsAct);

  menuBar()->addSeparator();

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

void Minutor::loadWorld(QDir path) {
  closeWorld();  // just in case
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
    QDirIterator it(path);
    bool hasPlayers = false;
    while (it.hasNext()) {
      it.next();
      if (it.fileInfo().isFile()) {
        hasPlayers = true;
        NBT player(it.filePath());
        auto pos = player.at("Pos");
        double posX = pos->at(0)->toDouble();
        double posZ = pos->at(2)->toDouble();
        auto dim = player.at("Dimension");
        if (dim && (dim->toInt() == -1)) {
          posX *= 8;
          posZ *= 8;
        }
        QString playerName = it.fileInfo().completeBaseName();
        QRegExp id("[0-9a-z]{8,8}\\-[0-9a-z]{4,4}\\-[0-9a-z]{4,4}"
                   "\\-[0-9a-z]{4,4}\\-[0-9a-z]{12,12}");
        if (id.exactMatch(playerName)) {
          playerName = QString("Player %1").arg(players.length());
        }

        QAction *p = new QAction(this);
        p->setText(playerName);
        p->setData(locations.count());
        locations.append(Location(posX, posZ));
        connect(p, SIGNAL(triggered()),
                this, SLOT(jumpToLocation()));
        players.append(p);
        if (player.has("SpawnX")) {  // player has a bed
          p = new QAction(this);
          p->setText(playerName+"'s Bed");
          p->setData(locations.count());
          locations.append(Location(player.at("SpawnX")->toDouble(),
                                    player.at("SpawnZ")->toDouble()));
          connect(p, SIGNAL(triggered()),
                  this, SLOT(jumpToLocation()));
          players.append(p);
        }
      }
    }
    jumpMenu->addActions(players);
    jumpMenu->setEnabled(hasPlayers);
    path.cdUp();
  }

  if (path.cd("data")) {
    loadStructures(path);
    path.cdUp();
  }

  // show dimensions
  dimensions->getDimensions(path, dimMenu, this);
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
  addOverlayItemType(item->type(), item->color(), item->dimension());

  maxentitydistance = 50;

  const OverlayItem::Point& p = item->midpoint();
  overlayItems[item->type()].insertMulti(QPair<int, int>(p.x, p.z), item);

  mapview->addOverlayItem(item);
}

void Minutor::showProperties(QVariant props) {
  if (!props.isNull()) {
    propView->DisplayProperties(props);
    propView->show();
  }
}

void Minutor::loadStructures(const QDir &dataPath) {
  // attempt to parse all of the files in the data directory, looking for
  // generated structures
  for (auto &fileName : dataPath.entryList(QStringList() << "*.dat")) {
    NBT file(dataPath.filePath(fileName));
    auto data = file.at("data");

    auto items = GeneratedStructure::tryParse(data);
    for (auto &item : items) {
      addOverlayItem(item);
    }

    if (items.isEmpty()) {
      // try parsing it as a village.dat file
      int underidx = fileName.lastIndexOf('_');
      int dotidx = fileName.lastIndexOf('.');
      QString dimension = "overworld";
      if (underidx > 0) {
        dimension = fileName.mid(underidx + 1, dotidx - underidx - 1);
      }
      items = Village::tryParse(data, dimension);

      for (auto &item : items) {
        addOverlayItem(item);
      }
    }
  }
}
