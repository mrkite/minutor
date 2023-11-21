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
#include "chunkcache.h"
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

  ChunkCache const & cache = ChunkCache::Instance();
  connect(&cache, &ChunkCache::structureFound,
          this,   &Minutor::addStructureFromChunk);

  // Definition manager
  dm = new DefinitionManager(this);
  mapview->attach(dm);
  connect(dm,   SIGNAL(packsChanged()),
          this, SLOT(updateDimensions()));
  // world information
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

  connect(depth,   SIGNAL(valueChanged(int)),
          mapview, SLOT  (setDepth(int)));
  connect(mapview, SIGNAL(demandDepthChange(double)),
          depth,   SLOT  (changeValue(double)));
  connect(mapview, SIGNAL(demandDepthValue(double)),
          depth,   SLOT  (setValue(double)));
  connect(this,    SIGNAL(worldLoaded(bool)),
          mapview, SLOT  (setEnabled(bool)));
  connect(this,    SIGNAL(worldLoaded(bool)),
          depth,   SLOT  (setEnabled(bool)));

  // player cache request/response to Mojang API
  connect(&this->qnam, &QNetworkAccessManager::finished,
          this,        &Minutor::updatePlayerCache);

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
  auto submenues = m_ui.menu_Overlay->findChildren<QMenu*>();
  qDeleteAll(submenues);
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
  mapview->redraw();
}

void Minutor::toggleOverlays()
{
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

void Minutor::toggleStructures(bool checked)
{
  bool toggleEnabled = false;
  for (QAction * action: m_ui.menu_Overlay->actions()) {
    if (action == separatorStructureOverlay) {
      // start after this separator
      toggleEnabled = true;
    } else if (action == separatorEntityOverlay) {
      // stop before this separator
      toggleEnabled = false;
    } else if (action->menu() || action->isSeparator()) {
      // stuff we want to ignore
    } else { // action
      if (toggleEnabled)
        action->setChecked(checked);
    }
  }

  toggleOverlays();
}

void Minutor::toggleEntities(bool checked)
{
  bool toggleEnabled = false;
  for (QAction * action: m_ui.menu_Overlay->actions()) {
    if (action == separatorEntityOverlay) {
      // start after this separator
      toggleEnabled = true;
    } else if (action->menu() || action->isSeparator() || (action == separatorStructureOverlay)) {
      // stuff we want to ignore
    } else { // action
      if (toggleEnabled)
        action->setChecked(checked);
    }
  }

  toggleOverlays();
}

template<typename CheckBox, typename Iter>
void updateTristateCheckBox(CheckBox* cbox, Iter b_iter, Iter e_iter)
{
  auto is_checked = [](auto action) { return action->isChecked(); };
  if (std::any_of(b_iter, e_iter, is_checked)) {
    if (std::all_of(b_iter, e_iter, is_checked)) {
      cbox->setCheckState(Qt::Checked);
    } else {
      cbox->setCheckState(Qt::PartiallyChecked);
    }
  } else {
    cbox->setCheckState(Qt::Unchecked);
  }
}

void Minutor::updateToggleAllStructuresState()
{
  auto actions = m_ui.menu_Overlay->actions();
  auto b_iter = std::next(actions.begin(), actions.indexOf(separatorStructureOverlay) + 1);
  auto e_iter = std::next(actions.begin(), actions.indexOf(separatorEntityOverlay));
  updateTristateCheckBox(qobject_cast<LabeledSeparator*>(separatorStructureOverlay), b_iter, e_iter);
}

void Minutor::updateToggleAllEntitiesState()
{
  auto actions = m_ui.menu_Overlay->actions();
  auto b_iter = std::next(actions.begin(), actions.indexOf(separatorEntityOverlay) + 1);
  updateTristateCheckBox(qobject_cast<LabeledSeparator*>(separatorEntityOverlay), b_iter, actions.end());
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
  for (QAction * action : qAsConst(structureOverlayActions)) {
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
  // reload structures for that dimension (old format from data directory)
  loadStructures(path);
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

void Minutor::insertToggleAllAction(QMenu* menu)
{
  Q_ASSERT(menu);
  // todo: find good name
  auto action = new LabeledSeparator(tr("toggle all"), menu);
  menu->addAction(action);
  connect(action, &QAction::triggered, menu, [=](bool checked) {
    auto actions = menu->actions();
    std::for_each(std::next(actions.begin()), actions.end(),
                  [=](auto action) { action->setChecked(checked); });
  });
  connect(action, &QAction::triggered, this, &Minutor::toggleOverlays);
}

void Minutor::updateToggleAllState(QMenu* menu)
{
  Q_ASSERT(menu);
  auto actions = menu->actions();
  auto toggle_all_action = qobject_cast<LabeledSeparator*>(actions.constFirst());
  Q_ASSERT(toggle_all_action);
  updateTristateCheckBox(toggle_all_action, std::next(actions.begin()), actions.end());
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

  connect(separatorStructureOverlay, &LabeledSeparator::triggered,
          this,                      &Minutor::toggleStructures);
  connect(separatorEntityOverlay,    &LabeledSeparator::triggered,
          this,                      &Minutor::toggleEntities);

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
            this,   SLOT(toggleOverlays()));
    connect(action, SIGNAL(triggered()),
            this,   SLOT(updateToggleAllEntitiesState()));
  }

  // [Search]

  // [Help]
}

void Minutor::createStatusBar() {
  statusBar()->showMessage(tr("Ready"));
}

void Minutor::getWorldList() {
  QDir mc(dialogSettings->mcpath);
  if (mc.exists("saves")) {
    mc.cd("saves");
  }

  // Create an action for each world found:
  WorldInfo & wi(WorldInfo::Instance());
  QDirIterator it(mc);
  while (it.hasNext()) {
    it.next();
    if (!it.fileInfo().isDir() || !wi.parseWorldFolder(it.filePath())) {
      continue;
    }
    auto * w = new QAction(this);
    w->setText(wi.getLevelName());
    w->setData(it.filePath());
    connect(w, SIGNAL(triggered()),
            this, SLOT(openWorld()));
    worldActions.append(w);
  }
  wi.clear();

  // Sort the actions by the world name:
  std::sort(worldActions.begin(), worldActions.end(),
    [](auto * act1, auto * act2) {
      return (act1->text() < act2->text());
    }
  );

  // Assign Ctrl+number shortcuts to the first 10 worlds:
  int key = 1;
  for (auto & act: worldActions)
  {
    act->setShortcut("Ctrl+" + QString::number(key));
    key++;
    if (key >= 10) {
      break;
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

  WorldInfo & wi(WorldInfo::Instance());
  wi.parseWorldFolder(path);
  wi.parseWorldInfo();

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
        QString playerUUID = it.fileInfo().completeBaseName();
        QString playerName = playerUUID;
        QIcon   playerIcon;
        if (path.dirName() == "playerdata") {
          // player name via UUID
          QRegExp id("[0-9a-z]{8,8}\\-[0-9a-z]{4,4}\\-[0-9a-z]{4,4}"
                     "\\-[0-9a-z]{4,4}\\-[0-9a-z]{12,12}");
          if (id.exactMatch(playerUUID)) {
            QSettings settings;
            // when present, remove old style cache
            if (settings.contains("PlayerCache/"+playerUUID)) {
              settings.remove("PlayerCache/"+playerUUID);
            }
            // check cache for player name
            if (settings.contains("PlayerCache/"+playerUUID+"/name")) {
              playerName = settings.value("PlayerCache/"+playerUUID+"/name", playerUUID).toString();
            } else if (playerUUID[14]=='4') {
              // only version 4 UUIDs can be resolved at Mojang API
              // trigger HTTPS request to get player name
              QString url = playerUUID;
              url = "https://sessionserver.mojang.com/session/minecraft/profile/" + url.remove('-');

              QNetworkRequest request;
              request.setUrl(QUrl(url));
              request.setRawHeader("User-Agent", "Minutor");
              pendingNetworkAccess[this->qnam.get(request)] = playerUUID;
            }
            // check cache for player texture
            if (settings.contains("PlayerCache/"+playerUUID+"/texture")) {
              QString  data64 = settings.value("PlayerCache/"+playerUUID+"/texture", playerUUID).toString();
              QByteArray data = QByteArray::fromBase64(data64.toUtf8());
              QImage head(reinterpret_cast<unsigned char *>(data.data()), 8, 8, QImage::Format_ARGB32);
              playerIcon = QIcon(QPixmap::fromImage(head).scaled(16,16));
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
        p->setIcon(playerIcon);
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
          p->setIcon(playerIcon);
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

  // create Dimensions menu
  WorldInfo::Instance().getDimensionsInWorld(path, m_ui.menu_Dimension, this);

  // finalize
  emit worldLoaded(true);
  mapview->setLocation(locations.first().x, locations.first().z);
  toggleFlags();
  toggleOverlays();
}

void Minutor::updatePlayerCache(QNetworkReply * reply) {
  auto response = reply->readAll();
  if (response.length() == 0) {
    reply->deleteLater();
    return;
  }
  // we got a response
  // response to initial profil query
  if (reply->request().url().toString().contains("sessionserver.mojang.com")) {
    QJsonDocument json = QJsonDocument::fromJson(response);
    if (!json.isEmpty() && json.object().contains("name")) {
      QString playerName = json["name"].toString();
      // reconstruct player UUID
      QString playerUUID = pendingNetworkAccess[reply];
      // store in player cache
      QSettings settings;
      settings.setValue("PlayerCache/"+playerUUID+"/name", playerName);

      // get URL to skin texture
      QString value = json["properties"][0]["value"].toString();
      QJsonDocument vjson = QJsonDocument::fromJson(QByteArray::fromBase64(value.toUtf8()));
      if (!vjson.isEmpty()) {
        QString url = vjson.object()["textures"].toObject()["SKIN"].toObject()["url"].toString();
        if (!url.isEmpty()) {
          QNetworkRequest request;
          request.setUrl(QUrl(url));
          request.setRawHeader("User-Agent", "Minutor");
          pendingNetworkAccess[this->qnam.get(request)] = playerUUID;
        }
      }
    }
    pendingNetworkAccess.remove(reply);
  }
  // response with skin texture
  if (reply->request().url().toString().contains("textures.minecraft.net")) {
    QImage skin;
    skin.loadFromData(response);
    QImage head(skin.copy(QRect(8,8,8,8)));
    // reconstruct player UUID
    QString playerUUID = pendingNetworkAccess[reply];
    // store in player cache
    QSettings settings;
    QString data = QByteArray(reinterpret_cast<char*>(head.bits()), 4*8*8).toBase64();
    settings.setValue("PlayerCache/"+playerUUID+"/texture", data);

    pendingNetworkAccess.remove(reply);
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

QMenu* Minutor::addOverlayItemMenu(QString path) {
  // for vanilla the path is empty (minecraft:)
  if ((path == "") || (path == "minecraft"))
    return m_ui.menu_Overlay;

  // split type name to get nested menu levels
  QList<QString> pathlist = path.split('.');

  // generate a nested menu structure to match the path
  QMenu* menu = m_ui.menu_Overlay;
  for (auto &p: pathlist) {
    QMenu* submenu = menu->findChild<QMenu*>(p);
    if (!submenu) {
      // create new sub-menu
      QMenu * submenu = new QMenu("&" + p, menu);
      submenu->setObjectName(p);
      // insert at alphabetical correct position
      QAction * insertBeforeAction = nullptr;
      for (QAction * action: menu->actions()) {
        if (action == separatorEntityOverlay) {
          // insert at least before Entities
          insertBeforeAction = separatorEntityOverlay;
          break;
        } else if (action->menu()) {
          // sub-menu means nested structures from a mod -> place before
          if (action->text() > submenu->title()) {
            insertBeforeAction = action;
            break;
          }
        } // ignore all other actions
      }
      menu->insertMenu(insertBeforeAction, submenu);
      menu = submenu;
      // add a "toggle all" action for this new sub-menu
      insertToggleAllAction(menu);
    } else {
      // continue with this sub-menu as parent
      menu = submenu;
    }
  }
  return menu;
}

void Minutor::addOverlayItemType(QString path, QString type,
                                 QColor color,
                                 QString dimension) {
  if (!overlayItemTypes.contains(type)) {
    overlayItemTypes.insert(type);

    // generate a nested menu structure to match the path
    QMenu* menu = addOverlayItemMenu(path);

    // generate a unique keyboard shortcut
    QString actionName = type.split('.').last();
    QString actionNameShortcut = actionName;
    QKeySequence sequence = generateUniqueKeyboardShortcut(&actionNameShortcut);

    // define color pixmap
    QPixmap pixmap(16, 16);
    QColor solidColor(color);
    solidColor.setAlpha(255);
    pixmap.fill(solidColor);

    // create Action
    QMap<QString, QVariant> entityData;
    entityData["type"] = type;
    entityData["dimension"] = dimension;

    structureOverlayActions.push_back(new QAction(pixmap, actionNameShortcut, this));
    structureOverlayActions.last()->setShortcut(sequence);
    structureOverlayActions.last()->setStatusTip(tr("Toggle viewing of %1")
                                          .arg(type));
    structureOverlayActions.last()->setEnabled(true);
    structureOverlayActions.last()->setData(entityData);
    structureOverlayActions.last()->setCheckable(true);

    // insert at alphabetically position
    QAction* insertBeforeAction = NULL;
    for (QAction * action: menu->actions()) {
      if (action == separatorEntityOverlay) {
        // insert at least before Entities
        insertBeforeAction = separatorEntityOverlay;
        break;
      } else if (action->menu()) {
        // sub-menu means nested structures from a mod -> place before
        insertBeforeAction = action;
        break;
      } else if (action->isSeparator() || (action == separatorStructureOverlay)) {
        // separator or other stuff we want to ignore
      } else { // action
        QString actionText = action->text();
        actionText.remove('&');
        if (actionName < actionText) {
          insertBeforeAction = action;
          break;
        }
      }
    }
    menu->insertAction(insertBeforeAction, structureOverlayActions.last());
    connect(structureOverlayActions.last(), SIGNAL(triggered()),
            this,                           SLOT(toggleOverlays()));
    if (menu != m_ui.menu_Overlay) {
      connect(structureOverlayActions.last(), &QAction::triggered,
              menu, [this, menu]() { updateToggleAllState(menu); });
      updateToggleAllState(menu);
    } else {
      connect(structureOverlayActions.last(), SIGNAL(triggered()),
              this,                           SLOT(updateToggleAllStructuresState()));
      updateToggleAllStructuresState();
    }
  }
}

void Minutor::addStructureFromChunk(QSharedPointer<GeneratedStructure> structure) {
  // update menu (if necessary)
  QString type = structure->type();
  QString path;
  if (!type.contains("minecraft:")) {
    // not vanilla -> structure from a mod
    QStringList mod = type.split(QRegularExpression("[.:]"));
    path = mod[1];
  }

  addOverlayItemType(path, type, structure->color());
  // add to list with overlays
  mapview->addOverlayItem(structure);
}

void Minutor::addOverlayItem(QSharedPointer<OverlayItem> item) {
  // create menu entries (if necessary)
  QString type = item->type();
  QString path;
  addOverlayItemType(path, type, item->color(), item->dimension());

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
  SearchChunksDialog* form = new SearchChunksDialog(searchPlugin, this);

  form->setAttribute(Qt::WA_DeleteOnClose);

  form->setSearchCenter(mapview->getLocation()->getPos3D());

  // map is moved
  connect(mapview, &MapView::coordinatesChanged,
          form,    qOverload<int,int,int>(&SearchChunksDialog::setSearchCenter) );
  // item from result list is highlighted -> jump
  connect(form,    &SearchChunksDialog::jumpTo,
          this,    &Minutor::triggerJumpToPosition );
  // update overlay elements
  connect(form,    &SearchChunksDialog::updateSearchResultPositions,
          this,    &Minutor::updateSearchResultPositions );

  // pre-fill depth range
  form->setRangeY(depth->minimum(), depth->maximum());

  return form;
}

void Minutor::openSearchEntityDialog() {
  // prepare dialog
  auto searchPlugin = QSharedPointer<SearchEntityPlugin>::create();
  auto searchEntityForm = prepareSearchForm(searchPlugin);
  // show dialog
  searchEntityForm->setWindowTitle(m_ui.action_SearchEntity->statusTip());
  searchEntityForm->showNormal();
}

void Minutor::openSearchBlockDialog() {
  // prepare dialog
  auto searchPlugin = QSharedPointer<SearchBlockPlugin>::create();
  auto searchBlockForm = prepareSearchForm(searchPlugin);
  // show dialog
  searchBlockForm->setWindowTitle(m_ui.action_SearchBlock->statusTip());
  searchBlockForm->showNormal();
}

void Minutor::openStatisticBlockDialog() {
  // prepare dialog
  StatisticDialog *dialog = new StatisticDialog(this);

  // update current location
  dialog->setSearchCenter(mapview->getLocation()->getPos3D());

  connect(mapview, &MapView::coordinatesChanged,
          dialog,  qOverload<int,int,int>(&StatisticDialog::setSearchCenter) );

  // pre-fill depth range
  dialog->setRangeY(depth->minimum(), depth->maximum());

  // show dialog
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

void Minutor::loadStructures(QDir path) {
  // check if data directory is present
  if (!path.cd("data")) return;

  // attempt to parse all of the files in the data directory, looking for
  // generated structures
  for (auto &fileName : path.entryList(QStringList() << "*.dat")) {
    NBT file(path.filePath(fileName));
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
