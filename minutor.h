/** Copyright (c) 2013, Sean Kasun */
#ifndef MINUTOR_H_
#define MINUTOR_H_

#include <QtWidgets/QMainWindow>
#include <QDir>
#include <QVariant>
#include <QSharedPointer>
#include <QSet>
#include <QVector3D>
#include <QtNetwork/QNetworkReply>

#include "ui_minutor.h"
#include "overlay/generatedstructure.h"

class QAction;
class QActionGroup;
class QMenu;
class QProgressDialog;
class MapView;
class LabelledSlider;
class DefinitionManager;
class DimensionIdentifier;
class Settings;
class DimensionInfo;
class WorldSave;
class Properties;
class OverlayItem;
class JumpTo;
class SearchChunksDialog;
class SearchPluginI;

class Location {
 public:
  Location(double x, double z, QString dim) : x(x), z(z), dimension(dim) {}
  Location(double x, double z) : x(x), z(z), dimension("minecraft:overworld") {}
  double x, z;
  QString dimension;
};

class Minutor : public QMainWindow {
  Q_OBJECT

 public:
  Minutor();
  ~Minutor();

  void loadWorld(QDir path);

  void savePNG(QString filename, bool autoclose = false,
               bool regionChecker = false, bool chunkChecker = false,
               int w_top = 0, int w_left = 0, int w_bottom = 0, int w_right = 0);

  void jumpToXZ(int blockX, int blockZ);  // jumps to the block coords
  void setViewLighting(bool value);       // set View->Ligthing
  void setViewMobspawning(bool value);    // set View->Mob_Spawning
  void setViewCavemode(bool value);       // set View->Cave_Mode
  void setViewDepthshading(bool value);   // set View->Depth_Shading
  void setViewBiomeColors(bool value);    // set View->Biome_Colors
  void setViewSeaGroundMode(bool value);  // set View->Sea_Ground_Mode
  void setViewSingleLayer(bool value);    // set View->Single_Layer
  void setViewSlimeChunks(bool value);    // set View->Slime_Chunks
  void setViewInhabitedTime(bool value);  // set View->Inhabited_Time
  void setDepth(int value);               // set Depth-Slider

  MapView *getMapview() const;
  void viewDimension(QString dim);        // view dimension matching dim_name

private slots:
  void openWorld();
  void open();
  void closeWorld();
  void reload();
  void save();
  void updatePlayerCache(QNetworkReply * reply);

  void jumpToLocation();
  void viewDimension(const DimensionInfo &dim);
  void toggleFlags();
  void toggleOverlays();

  void about();

  void updateDimensions();
  void rescanWorlds();
  void saveProgress(QString status, double value);
  void saveFinished();
  void addStructureFromChunk(QSharedPointer<GeneratedStructure> structure);
  void addOverlayItem(QSharedPointer<OverlayItem> item);
  QMenu* addOverlayItemMenu(QString path);
  void addOverlayItemType(QString path, QString type, QColor color, QString dimension = "");
  void showProperties(QVariant props);

  void openSearchEntityDialog();
  void openSearchBlockDialog();
  void openStatisticBlockDialog();

  void triggerJumpToPosition(QVector3D pos);

  void updateSearchResultPositions(QVector<QSharedPointer<OverlayItem> >);

  void toggleStructures(bool checked);
  void toggleEntities(bool checked);

  void updateToggleAllStructuresState();
  void updateToggleAllEntitiesState();

signals:
  void worldLoaded(bool isLoaded);

 private:
  Ui::Minutor m_ui;
  SearchChunksDialog* prepareSearchForm(const QSharedPointer<SearchPluginI> &searchPlugin);

  void createActions();
  void createMenus();
  void createStatusBar();
  void loadStructures(QDir path);
  QKeySequence generateUniqueKeyboardShortcut(QString *actionName);

  void insertToggleAllAction(QMenu* menu);
  void updateToggleAllState(QMenu* menu);

  /** Populates worldActions with one action for each world encountered in the default Minecraft saves directory.
  Each action opens the linked world upon triggering.
  The actions are sorted by world's folder name. */
  void getWorldList();

  MapView *mapview;
  LabelledSlider *depth;
  QProgressDialog *progress;
  bool progressAutoclose;

  QList<QAction*> worldActions;
  QList<QAction*> playerActions;
  QList<QAction*> entityOverlayActions;
  QList<QAction*> structureOverlayActions;
  QAction* separatorEntityOverlay;
  QAction* separatorStructureOverlay;

  // loaded world data
  QList<Location> locations;  // data of player related locations in this world
  DefinitionManager *dm;
  Settings *dialogSettings;
  JumpTo *dialogJumpTo;
  QDir currentWorld;
  QNetworkAccessManager qnam;
  QMap<QNetworkReply*, QString> pendingNetworkAccess;

  QSet<QString> overlayItemTypes;
  Properties * propView;
};

#endif  // MINUTOR_H_
