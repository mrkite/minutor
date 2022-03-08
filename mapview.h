/** Copyright (c) 2013, Sean Kasun */
#ifndef MAPVIEW_H_
#define MAPVIEW_H_

#include <QtWidgets/QWidget>
#include <QSharedPointer>
#include "chunkcache.h"

class DefinitionManager;
class BiomeIdentifier;
class BlockIdentifier;
class OverlayItem;

class MapView : public QWidget {
  Q_OBJECT

 public:
  /// Values for the individual flags
  enum {
    flgLighting     = 1 << 0,
    flgMobSpawn     = 1 << 1,
    flgCaveMode     = 1 << 2,
    flgDepthShading = 1 << 3,
    flgBiomeColors  = 1 << 4,
    flgSeaGround    = 1 << 5,
    flgSingleLayer  = 1 << 6,
    flgSlimeChunks  = 1 << 7
  };

  typedef struct {
    float x, y, z;
    int scale;
    QVector3D getPos3D() const {
      return QVector3D(x,y,z);
    }
  } BlockLocation;

  explicit MapView(QWidget *parent = 0);

  QSize minimumSizeHint() const;
  QSize sizeHint() const;

  void attach(DefinitionManager *dm);

  void setLocation(double x, double z);
  void setLocation(double x, int y, double z, bool ignoreScale, bool useHeight);
  BlockLocation *getLocation();
  void setDimension(QString path, int scale);
  void setFlags(int flags);
  int  getFlags() const;
  int  getDepth() const;
  void addOverlayItem(QSharedPointer<OverlayItem> item);
  void clearOverlayItems();
  void setVisibleOverlayItemTypes(const QSet<QString>& itemTypes);

  // public for saving the png
  QString getWorldPath();

  void updateSearchResultPositions(const QVector<QSharedPointer<OverlayItem> > &searchResults);


 public slots:
  void setDepth(int depth);
  void chunkUpdated(int x, int z);
  void redraw();

  // Clears the cache and redraws, causing all chunks to be re-loaded;
  // but keeps the viewport
  void clearCache();

 signals:
  void hoverTextChanged(QString text);
  void demandDepthChange(double value);
  void demandDepthValue(double value);
  void showProperties(QVariant properties);
  void addOverlayItemType(QString type, QColor color);
  void coordinatesChanged(int x, int y, int z);

 protected:
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseDoubleClickEvent(QMouseEvent *event);
  void wheelEvent(QWheelEvent *event);
  void keyPressEvent(QKeyEvent *event);
  void resizeEvent(QResizeEvent *event);
  void paintEvent(QPaintEvent *event);

 private slots:
  void addStructureFromChunk(QSharedPointer<GeneratedStructure> structure);

 private:
  void drawChunk(int x, int z);
  void getToolTip(int x, int z);
  int getY(int x, int z);
  QList<QSharedPointer<OverlayItem>> getItems(int x, int y, int z);
  void adjustZoom(double steps, bool allowZoomOut, bool cursorSource);

  template<typename ListT>
  void drawOverlayItems(const ListT& list, const OverlayItem::Cuboid& cuboid, double x1, double z1, QPainter& canvas);

  static const int CAVE_DEPTH = 16;  // maximum depth caves are searched in cave mode
  float caveshade[CAVE_DEPTH];

  int depth;
  double x, z;
  int scale;
  double zoomLevel;
  double zoom;
  int flags;
  int lastMouseX = -1, lastMouseY = -1;
  ChunkCache &cache;
  QImage imageChunks;
  QImage imageOverlays;
  DefinitionManager *dm;
  uchar placeholder[16 * 16 * 4];  // no chunk found placeholder
  QSet<QString> overlayItemTypes;
  QMap<QString, QList<QSharedPointer<OverlayItem>>> overlayItems;
  BlockLocation currentLocation;

  QVector<QSharedPointer<OverlayItem> > currentSearchResults;
};

#endif  // MAPVIEW_H_
