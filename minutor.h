/** Copyright (c) 2013, Sean Kasun */
#ifndef MINUTOR_H_
#define MINUTOR_H_

#include <QtWidgets/QMainWindow>
#include <QDir>
#include <QVariant>
#include <QSharedPointer>
#include <QSet>

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

class Location {
 public:
  Location(double x, double z) : x(x), z(z) {}
  double x, z;
};

class Minutor : public QMainWindow {
  Q_OBJECT

 public:
  Minutor();

  void loadWorld(QDir path);

  void savePNG(QString filename, bool autoclose = false,
               bool regionChecker = false, bool chunkChecker = false);

  void jumpToXZ(int blockX, int blockZ);  // jumps to the block coords
  void setViewLighting(bool value);       // set View->Ligthing
  void setViewMobspawning(bool value);    // set View->Mob_Spawning
  void setViewCavemode(bool value);       // set View->Cave_Mode
  void setViewDepthshading(bool value);   // set View->Depth_Shading
  void setViewBiomeColors(bool value);    // set View->Biome_Colors
  void setDepth(int value);               // set Depth-Slider

 private slots:
  void openWorld();
  void open();
  void closeWorld();
  void reload();
  void save();

  void jumpToLocation();
  void viewDimension(const DimensionInfo &dim);
  void toggleFlags();

  void about();

  void updateDimensions();
  void rescanWorlds();
  void saveProgress(QString status, double value);
  void saveFinished();
  void addOverlayItem(QSharedPointer<OverlayItem> item);
  void addOverlayItemType(QString type, QColor color, QString dimension = "");
  void showProperties(QVariant props);

 signals:
  void worldLoaded(bool isLoaded);

 private:
  void createActions();
  void createMenus();
  void createStatusBar();
  void loadStructures(const QDir &dataPath);
  void populateEntityOverlayMenu();
  QKeySequence generateUniqueKeyboardShortcut(QString *actionName);

  QString getWorldName(QDir path);
  void getWorldList();

  MapView *mapview;
  LabelledSlider *depth;
  QProgressDialog *progress;
  bool progressAutoclose;

  QMenu *fileMenu, *worldMenu;
  QMenu *viewMenu, *jumpMenu, *dimMenu;
  QMenu *helpMenu;
  QMenu *structureOverlayMenu, *entityOverlayMenu;

  QList<QAction *>worlds;
  QAction *openAct, *reloadAct, *saveAct, *exitAct;
  QAction *jumpSpawnAct;
  QList<QAction *>players;
  QAction *lightingAct, *mobSpawnAct, *caveModeAct, *depthShadingAct, *biomeColorsAct;
  QAction *manageDefsAct;
  QAction *refreshAct;
  QAction *aboutAct;
  QAction *settingsAct;
  QAction *updatesAct;
  QList<QAction*> structureActions;
  QList<QAction*> entityActions;

  // loaded world data
  QList<Location> locations;
  DefinitionManager *dm;
  Settings *settings;
  DimensionIdentifier *dimensions;
  QDir currentWorld;

  //           type                 x    z
  typedef QMap<QString, QHash<QPair<int, int>,
          QSharedPointer<OverlayItem>>> OverlayMap;
  OverlayMap overlayItems;
  QSet<QString> overlayItemTypes;
  int maxentitydistance;
  Properties * propView;
};

#endif  // MINUTOR_H_
