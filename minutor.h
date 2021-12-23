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
#include <QWidgetAction>

#include "ui_minutor.h"

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
class SearchChunksWidget;
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
  void setDepth(int value);               // set Depth-Slider

  MapView *getMapview() const;

private slots:
  void openWorld();
  void open();
  void closeWorld();
  void reload();
  void save();
  void updatePlayerCache(QNetworkReply*);

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

  void openSearchBlockWidget();
  void openSearchEntityWidget();

  void triggerJumpToPosition(QVector3D pos);

  void updateSearchResultPositions(QVector<QSharedPointer<OverlayItem> >);

signals:
  void worldLoaded(bool isLoaded);

 private:
  Ui::Minutor m_ui;
  SearchChunksWidget* prepareSearchForm(const QSharedPointer<SearchPluginI> &searchPlugin);

  void createActions();
  void createMenus();
  void createStatusBar();
  void loadStructures(const QDir &dataPath);
  QKeySequence generateUniqueKeyboardShortcut(QString *actionName);

  QString getWorldName(QDir path);
  void getWorldList();

  MapView *mapview;
  LabelledSlider *depth;
  QProgressDialog *progress;
  bool progressAutoclose;

  QList<QAction*> worldActions;
  QList<QAction*> playerActions;
  QList<QAction*> entityOverlayActions;
  QList<QAction*> structureOverlayActions;
  QWidgetAction* separatorEntityOverlay;
  QWidgetAction* separatorStructureOverlay;

  // loaded world data
  QList<Location> locations;  // data of player related locations in this world
  DefinitionManager *dm;
  Settings *dialogSettings;
  JumpTo *dialogJumpTo;
  QDir currentWorld;
  int  currentWorldVersion;
  QNetworkAccessManager qnam;

  QSet<QString> overlayItemTypes;
  Properties * propView;
};

#endif  // MINUTOR_H_
