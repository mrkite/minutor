/** Copyright (c) 2013, Sean Kasun */
#ifndef MAPVIEW_H_
#define MAPVIEW_H_

#include <QtWidgets/QWidget>
#include <QSharedPointer>
#include "./chunkcache.h"
class DefinitionManager;
class BiomeIdentifier;
class BlockIdentifier;
class OverlayItem;

class MapView : public QWidget {
  Q_OBJECT

 public:
  /// Values for the individual flags
  enum {
    flgLighting     = 1,
    flgMobSpawn     = 2,
    flgCaveMode     = 4,
    flgDepthShading = 8,
    flgShowEntities = 16,
    flgBiomeColors  = 64
  };

  explicit MapView(QWidget *parent = 0);

  QSize minimumSizeHint() const;
  QSize sizeHint() const;

  void attach(DefinitionManager *dm);

  void setLocation(double x, double z);
  void setDimension(QString path, int scale);
  void setFlags(int flags);
  void addOverlayItem(QSharedPointer<OverlayItem> item);
  void showOverlayItemTypes(const QSet<QString>& itemTypes);

  // public for saving the png
  void renderChunk(Chunk *chunk);
  QString getWorldPath();


 public slots:
  void setDepth(int depth);
  void chunkUpdated(int x, int z);
  void redraw();

  // Clears the cache and redraws, causing all chunks to be re-loaded;
  // but keeps the viewport
  void clearCache();

 signals:
  void hoverTextChanged(QString text);
  void demandDepthChange(int value);
  void showProperties(QVariant properties);
  void addOverlayItemType(QString type, QColor color);

 protected:
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseDoubleClickEvent(QMouseEvent *event);
  void wheelEvent(QWheelEvent *event);
  void keyPressEvent(QKeyEvent *event);
  void resizeEvent(QResizeEvent *event);
  void paintEvent(QPaintEvent *event);

 private:
  void drawChunk(int x, int z);
  void getToolTip(int x, int z);
  int getY(int x, int z);
  QList<QSharedPointer<OverlayItem>> getItems(int x, int y, int z);

  int depth;
  double x, z;
  int scale;
  double zoom;
  int flags;
  ChunkCache cache;
  QImage image;
  DefinitionManager *dm;
  BlockIdentifier *blocks;
  BiomeIdentifier *biomes;
  uchar placeholder[16 * 16 * 4];  // no chunk found placeholder
  QSet<QString> overlayItemTypes;
  QMap<QString, QList<QSharedPointer<OverlayItem>>> overlayItems;
};

#endif  // MAPVIEW_H_
