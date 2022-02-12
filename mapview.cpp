/** Copyright (c) 2013, Sean Kasun */
#include <QPainter>
#include <QResizeEvent>
#include <QMessageBox>
#include <cmath>
#include <assert.h>

#include "mapview.h"
#include "chunkcache.h"
#include "chunkrenderer.h"
#include "identifier/definitionmanager.h"
#include "identifier/blockidentifier.h"
#include "identifier/biomeidentifier.h"
#include "clamp.h"

MapView::MapView(QWidget *parent)
  : QWidget(parent)
  , depth(255)
  , scale(1)      // overworld coordinate mapping
  , zoomLevel(0)  // 1:1
  , cache(ChunkCache::Instance())
{
  adjustZoom(0, false);
  connect(&cache, SIGNAL(chunkLoaded(int, int)),
          this,   SLOT  (chunkUpdated(int, int)));
  connect(&cache, SIGNAL(structureFound(QSharedPointer<GeneratedStructure>)),
          this,   SLOT  (addStructureFromChunk(QSharedPointer<GeneratedStructure>)));
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);

  int offset = 0;
  for (int y = 0; y < 16; y++)
    for (int x = 0; x < 16; x++) {
      uchar color = ((x & 8) ^ (y & 8)) == 0 ? 0x44 : 0x88;
      placeholder[offset++] = color;
      placeholder[offset++] = color;
      placeholder[offset++] = color;
      placeholder[offset++] = 0xff;
    }
  // calculate exponential function for cave shade
  float cavesum = 0.0;
  for (int i=0; i<CAVE_DEPTH; i++) {
    caveshade[i] = 1/exp(i/(CAVE_DEPTH/2.0));
    cavesum += caveshade[i];
  }
  for (int i=0; i<CAVE_DEPTH; i++) {
    caveshade[i] = 1.5 * caveshade[i] / cavesum;
  }
}

QSize MapView::minimumSizeHint() const {
  return QSize(300, 300);
}
QSize MapView::sizeHint() const {
  return QSize(400, 400);
}

void MapView::attach(DefinitionManager *dm) {
  this->dm = dm;
  connect(dm, SIGNAL(packsChanged()),
          this, SLOT(redraw()));
}

void MapView::setLocation(double x, double z) {
  setLocation(x, depth, z, false, true);
}

void MapView::setLocation(double x, int y, double z, bool ignoreScale, bool useHeight) {
  this->x = ignoreScale ? x : x / scale;
  this->z = ignoreScale ? z : z / scale;
  if (useHeight == true && depth != y) {
    emit demandDepthValue(y);
  } else {
    redraw();
  }
}

MapView::BlockLocation *MapView::getLocation()
{
  currentLocation.x = x;
  currentLocation.y = depth;
  currentLocation.z = z;
  currentLocation.scale = scale;

  return &currentLocation;
}

void MapView::setDimension(QString path, int scale) {
  if (scale > 0) {
    this->x *= this->scale;
    this->z *= this->scale;  // undo current scale transform
    this->scale = scale;
    this->x /= scale;  // and do the new scale transform
    this->z /= scale;
  } else {
    this->scale = 1;  // no scaling because no relation to overworld
    this->x = 0;  // and we jump to the center spawn automatically
    this->z = 0;
  }
  cache.clear();
  cache.setPath(path);
  redraw();
}

void MapView::setDepth(int depth) {
  this->depth = depth;
  redraw();
}

void MapView::setFlags(int flags) {
  this->flags = flags;
}

int MapView::getFlags() const {
  return flags;
}

int MapView::getDepth() const {
  return depth;
}

void MapView::chunkUpdated(int x, int z) {
  drawChunk(x, z);
  update();
}

QString MapView::getWorldPath() {
  return cache.getPath();
}

void MapView::updateSearchResultPositions(const QVector<QSharedPointer<OverlayItem> > &searchResults)
{
  currentSearchResults = searchResults;
}

void MapView::clearCache() {
  cache.clear();
  redraw();
}

void MapView::adjustZoom(double steps, bool allowZoomOut)
{
  // get current zoom level as an index, rounded to nearest int
  int oldZoomIndex = (int)(floor(zoomLevel + 0.5));
  zoomLevel += steps;

  // use Fibonacci numbers to get natural zoom behaviour
  const float zoomTable[] = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89};
  const int zoomMin = allowZoomOut ? -4 : 0;
  const int zoomMax = (sizeof(zoomTable) / sizeof(float)) -1;

  if (zoomLevel < zoomMin) zoomLevel = zoomMin;
  if (zoomLevel > zoomMax) zoomLevel = zoomMax;

  int zoomIndex = (int)(floor(zoomLevel + 0.5 ));

  // check whether zoomIndex has changed since last adjustZoom call
  // don't return early if steps == 0, this is used for initialization
  if (zoomIndex == oldZoomIndex && steps != 0) return;

  // determine minimal zoomed value that is allowed
  // with current window size and available pyhiscal memory
  bool restrictZoom = true;
  int maxchunks = cache.getMemoryMax();
  int chunks    = cache.getCacheMax();
  do {
    // apply new zoom
    if (zoomIndex < 0)
      zoom = 1 / pow(2.0, -zoomIndex);
    else
      zoom = zoomTable[zoomIndex];

    // check
    int ppc = ceil(16*zoom);
    int cx = (imageChunks.width() +ppc-1) / ppc;
    int cz = (imageChunks.height()+ppc-1) / ppc;
    chunks = cx * cz;
    if ((1.2 * chunks) <= maxchunks)
      restrictZoom = false; // everything matches with a low margin of 20%
    else
      zoomIndex++;          // restrict zoom
  } while (restrictZoom);

  // we try to set higher margin than above (100%)!
  cache.setCacheMaxSize(2.0 * chunks);

  // no point in redrawing if the zoom level didn't change
  if (steps != 0) redraw();
}

static int lastMouseX = -1, lastMouseY = -1;
static bool dragging = false;
void MapView::mousePressEvent(QMouseEvent *event) {
  lastMouseX = event->x();
  lastMouseY = event->y();
  dragging = true;
}

void MapView::mouseMoveEvent(QMouseEvent *event) {
  if (!dragging) {
    int centerblockx = floor(this->x);
    int centerblockz = floor(this->z);

    int centerx = imageChunks.width() / 2;
    int centery = imageChunks.height() / 2;

    centerx -= (this->x - centerblockx) * zoom;
    centery -= (this->z - centerblockz) * zoom;

    int mx = floor(centerblockx - (centerx - event->x()) / zoom);
    int mz = floor(centerblockz - (centery - event->y()) / zoom);

    getToolTip(mx, mz);
    return;
  }
  x += (lastMouseX-event->x()) / zoom;
  z += (lastMouseY-event->y()) / zoom;
  lastMouseX = event->x();
  lastMouseY = event->y();

  redraw();
}

void MapView::mouseReleaseEvent(QMouseEvent * /* event */) {
  dragging = false;
}

void MapView::mouseDoubleClickEvent(QMouseEvent *event) {
  int centerblockx = floor(this->x);
  int centerblockz = floor(this->z);

  int centerx = imageChunks.width() / 2;
  int centery = imageChunks.height() / 2;

  centerx -= (this->x - centerblockx) * zoom;
  centery -= (this->z - centerblockz) * zoom;

  int mx = floor(centerblockx - (centerx - event->x()) / zoom);
  int mz = floor(centerblockz - (centery - event->y()) / zoom);

  // get the y coordinate
  int my = getY(mx, mz);

  QList<QVariant> properties;
  for (auto &item : getItems(mx, my, mz)) {
    properties.append(item->properties());
  }

  if (!properties.isEmpty()) {
    emit showProperties(properties);
  }
}

void MapView::wheelEvent(QWheelEvent *event) {
  Qt::KeyboardModifier modifier4DepthSlider = Qt::KeyboardModifier(QSettings().value("modifier4DepthSlider", Qt::ShiftModifier).toUInt());
  Qt::KeyboardModifier modifier4ZoomOut     = Qt::KeyboardModifier(QSettings().value("modifier4ZoomOut",     Qt::ControlModifier).toUInt());

  if ((event->modifiers() & modifier4DepthSlider) == modifier4DepthSlider) {
    // change depth
    emit demandDepthChange(event->delta() / 120);
  } else if ((event->modifiers() & modifier4ZoomOut) == modifier4ZoomOut) {
    // allow change zoom also to zoom OUT
    adjustZoom( event->angleDelta().y() / 120.0, true );
  } else {
    // normal change zoom
    adjustZoom( event->angleDelta().y() / 120.0, false );
  }
}

void MapView::keyPressEvent(QKeyEvent *event) {
  // default: 16 blocks / 1 chunk
  float stepSize = 16.0;
  bool allowZoomOut = false;

  if        ((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier) {
    // 1 block for fine tuning
    stepSize = 1.0;
  } else if ((event->modifiers() & Qt::AltModifier) == Qt::AltModifier) {
    // 8 chunks
    stepSize = 128.0;
  } else if ((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
    // 32 chunks / 1 Region
    stepSize = 512.0;
    allowZoomOut = true;
  }

  switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
      z -= stepSize / zoom;
      redraw();
      break;
    case Qt::Key_Down:
    case Qt::Key_S:
      z += stepSize / zoom;
      redraw();
      break;
    case Qt::Key_Left:
    case Qt::Key_A:
      x -= stepSize / zoom;
      redraw();
      break;
    case Qt::Key_Right:
    case Qt::Key_D:
      x += stepSize / zoom;
      redraw();
      break;
    case Qt::Key_PageUp:
    case Qt::Key_Q:
      adjustZoom(1, allowZoomOut);
      redraw();
      break;
    case Qt::Key_PageDown:
    case Qt::Key_E:
      adjustZoom(-1, allowZoomOut);
      redraw();
      break;
    case Qt::Key_Home:
    case Qt::Key_Plus:
    case Qt::Key_BracketLeft:
      emit demandDepthChange(+1);
      break;
    case Qt::Key_End:
    case Qt::Key_Minus:
    case Qt::Key_BracketRight:
      emit demandDepthChange(-1);
      break;
  }
}

void MapView::resizeEvent(QResizeEvent *event) {
  // adapt size of rendered images
  imageChunks   = QImage(event->size(), QImage::Format_RGB32);
  imageOverlays = QImage(event->size(), QImage::Format_RGBA8888);
  // restrict zoom and adapt ChunkCache
  adjustZoom(0, true);
  // redraw everything
  redraw();
}

void MapView::paintEvent(QPaintEvent * /* event */) {
  QPainter p(this);
  p.drawImage(QPoint(0, 0), imageChunks);
  p.drawImage(QPoint(0, 0), imageOverlays);
  p.end();
}

void MapView::redraw() {
  if (!this->isEnabled()) {
    // blank
    imageChunks.fill(0xeeeeee);
    update();
    return;
  }

  double chunksize = 16 * zoom;

  // first find the center block position
  int centerchunkx = floor(x / 16);
  int centerchunkz = floor(z / 16);
  // and the center of the screen
  int centerx = imageChunks.width() / 2;
  int centery = imageChunks.height() / 2;
  // and align for panning
  centerx -= (x - centerchunkx * 16) * zoom;
  centery -= (z - centerchunkz * 16) * zoom;
  // now calculate the topleft block on the screen
  int startx = centerchunkx - floor(centerx / chunksize) - 1;
  int startz = centerchunkz - floor(centery / chunksize) - 1;
  // and the dimensions of the screen in blocks
  int blockswide = imageChunks.width() / chunksize + 3;
  int blockstall = imageChunks.height() / chunksize + 3;

  for (int cz = startz; cz < startz + blockstall; cz++)
    for (int cx = startx; cx < startx + blockswide; cx++)
      drawChunk(cx, cz);

  // clear the overlay layer
  imageOverlays.fill(0);

  // add on the entity layer
  QPainter canvas(&imageOverlays);
  double halfviewwidth  = imageOverlays.width() / 2 / zoom;
  double halvviewheight = imageOverlays.height() / 2 / zoom;
  double x1 = x - halfviewwidth;
  double z1 = z - halvviewheight;
  double x2 = x + halfviewwidth;
  double z2 = z + halvviewheight;

  // draw the entities
  for (int cz = startz; cz < startz + blockstall; cz++) {
    for (int cx = startx; cx < startx + blockswide; cx++) {
      QSharedPointer<Chunk> chunk(cache.fetch(cx, cz));
      if (chunk) {
        // Entities from Chunks
        for (auto &type : overlayItemTypes) {
          auto range = chunk->entities.equal_range(type);
          for (auto it = range.first; it != range.second; ++it) {
            // don't show entities above our depth
            int entityY = (*it)->midpoint().y;
            // everything below the current block,
            // but also inside the current block
            if (entityY < depth + 1) {
              int entityX = static_cast<int>((*it)->midpoint().x) & 0x0f;
              int entityZ = static_cast<int>((*it)->midpoint().z) & 0x0f;
              int index = entityX + (entityZ << 4);
              int highY = chunk->depth[index];
              if ( (entityY+10 >= highY) ||
                   (entityY+10 >= depth) )
                (*it)->draw(x1, z1, zoom, &canvas);
            }
          }
        }

      }
    }
  }

  const OverlayItem::Cuboid viewingCuboid(OverlayItem::Point(x1 - 1, -4096, z1 - 1),
                                          OverlayItem::Point(x2 + 1, depth, z2 + 1));

  // draw the generated structures
  for (auto &type : overlayItemTypes) {
    drawOverlayItems(overlayItems[type], viewingCuboid, x1, z1, canvas);
  }

  drawOverlayItems(currentSearchResults, viewingCuboid, x1, z1, canvas);

  emit(coordinatesChanged(x, depth, z));

  update();
}


template<typename ListT>
void MapView::drawOverlayItems(const ListT &list, const OverlayItem::Cuboid& cuboid, double x1, double z1, QPainter& canvas)
{
  for (auto &item : list) {
    if (item->intersects(cuboid)) {
      item->draw(x1, z1, zoom, &canvas);
    }
  }
}

void MapView::drawChunk(int x, int z) {
  if (!this->isEnabled())
    return;

  // fetch the chunk
  QSharedPointer<Chunk> chunk(cache.fetch(x, z));
  if (chunk && !chunk->loaded) return;

  if (chunk && chunk->rendering) return;

  if (chunk && (chunk->renderedAt != depth ||
                chunk->renderedFlags != flags)) {
    //renderChunk(chunk);
    chunk->rendering = true;
    ChunkRenderer *renderer = new ChunkRenderer(x, z, depth, flags);
    connect(renderer, &ChunkRenderer::rendered,
            [chunk](int, int) { chunk->rendering = false; });
    connect(renderer, SIGNAL(rendered(int, int)),
            this,     SLOT(chunkUpdated(int, int)));
    QThreadPool::globalInstance()->start(renderer);
    return;
  }

  // this figures out where on the screen this chunk should be drawn

  // first find the center chunk
  int centerchunkx = floor(this->x / 16);
  int centerchunkz = floor(this->z / 16);
  // and the center chunk screen coordinates
  double centerx = imageChunks.width() / 2;
  double centery = imageChunks.height() / 2;
  // which need to be shifted to account for panning inside that chunk
  centerx -= (this->x - centerchunkx * 16) * zoom;
  centery -= (this->z - centerchunkz * 16) * zoom;
  // centerx,y now points to the top left corner of the center chunk
  // so now calculate our x,y in relation
  double chunksize = 16 * zoom;
  centerx += (x - centerchunkx) * chunksize;
  centery += (z - centerchunkz) * chunksize;

  const uchar* srcImageData = chunk ? chunk->getImage() : placeholder;
  QImage srcImage(srcImageData, 16, 16, QImage::Format_RGB32);

  QRectF targetRect(centerx, centery, chunksize, chunksize);

  QPainter canvas(&imageChunks);
  if (this->zoom < 1.0)
      canvas.setRenderHint(QPainter::SmoothPixmapTransform);
  canvas.drawImage(targetRect, srcImage);
}

void MapView::getToolTip(int x, int z) {
  int cx = floor(x / 16.0);
  int cz = floor(z / 16.0);
  QSharedPointer<Chunk> chunk(cache.fetch(cx, cz));
  int offset = (x & 0xf) + (z & 0xf) * 16;
  int y = 0;

  QString blockname = "Unknown Block";
  QString biomename = "Unknown Biome";
  QString blockstate;
  QMap<QString, int> entityIds;

  if ((chunk) && (chunk->highest >= chunk->lowest)) {
    int top = std::min(depth, chunk->highest);
    for (y = top; y >= chunk->lowest; y--) {
      const ChunkSection *section = chunk->getSectionByY(y);
      if (!section) {
        // skip entire section
        int section_idx = (y >> 4);
        y = (section_idx << 4) - 1;
        continue;
      }
      // get information about block
      const PaletteEntry & pdata = section->getPaletteEntry(offset, y);
      blockname = pdata.name;

      // For unknown legacy block IDs, add the legacy ID to the displayed name.
      // TODO: Hoist "Unknown Block" literal into constant
      if (blockname == "Unknown Block" && pdata.properties.contains(PaletteEntry::legacyBlockIdProperty)) {
        uint fullLegacyBlockId = pdata.properties[PaletteEntry::legacyBlockIdProperty].toUInt();
        // Legacy IDs with internal values > 4096 are virtual IDs with
        // higher-order bits encoding the data.
        uint baseLegacyBlockId = fullLegacyBlockId & 4095;
        uint legacyData = fullLegacyBlockId >> 12;
        QString displayName = QStringLiteral("Unknown [%1:%2]").arg(baseLegacyBlockId).arg(legacyData);
        blockname = displayName;
      }
      // in case of fully transparent blocks (meaning air)
      // -> we continue downwards
      auto & block = BlockIdentifier::Instance().getBlockInfo(pdata.hid);
      if (block.alpha == 0.0) continue;
      if (flags & MapView::flgSeaGround && block.isLiquid()) continue;

      // list all Block States
      for (auto key : pdata.properties.keys()) {
        // If needed, legacyBlockId is already encoded into the display name.
        if (key == PaletteEntry::legacyBlockIdProperty) {
          continue;
        }
        blockstate += key;
        blockstate += ":";
        blockstate += pdata.properties[key].toString();
        blockstate += " ";
      }
      blockstate.chop(1);
      break;
    }
    int biomeID = chunk->getBiomeID((x & 0xf), y, (z & 0xf));
    const BiomeInfo &biome = (chunk->version >=2800) ?
        BiomeIdentifier::Instance().getBiome((quint8)biomeID) :
        BiomeIdentifier::Instance().getBiome((qint32)biomeID);
    biomename = biome.name;

    // count Entity of each display type
    for (auto &item : getItems(x, y, z)) {
      entityIds[item->display()]++;
    }
  }

  QString entityStr;
  if (!entityIds.empty()) {
    QStringList entities;
    QMap<QString, int>::const_iterator it, itEnd = entityIds.cend();
    for (it = entityIds.cbegin(); it != itEnd; ++it) {
      if (it.value() > 1) {
        entities << it.key() + ":" + QString::number(it.value());
      } else {
        entities << it.key();
      }
    }
    entityStr = entities.join(", ");
  }

  QString hovertext = QString("X:%1 Y:%2 Z:%3 - %4 - %5")
                              .arg(x).arg(y).arg(z)
                              .arg(biomename)
                              .arg(blockname);
  if (blockstate.length() > 0)
    hovertext += " (" + blockstate + ")";
  if (entityStr.length() > 0)
    hovertext += " - " + entityStr;

#if defined(DEBUG) || defined(_DEBUG) || defined(QT_DEBUG)
  hovertext += " [Cache:"
            + QString().number(this->cache.getCacheUsage()) + "/"
            + QString().number(this->cache.getCacheMax()) + "]";
  hovertext += " Zoom:" + QString().number(zoomLevel);
#endif

  emit hoverTextChanged(hovertext);
}

void MapView::addStructureFromChunk(QSharedPointer<GeneratedStructure> structure) {
  // update menu (if necessary)
  emit addOverlayItemType(structure->type(), structure->color());
  // add to list with overlays
  addOverlayItem(structure);
}

void MapView::addOverlayItem(QSharedPointer<OverlayItem> item) {
  // test if item is already in list
  for (auto &it: overlayItems[item->type()]) {
    OverlayItem::Point p1 = it  ->midpoint();
    OverlayItem::Point p2 = item->midpoint();
    if ( (p1.x == p2.x) && (p1.y == p2.y) && (p1.z == p2.z) )
      return;  // skip if already present
  }
  // otherwise add item
  overlayItems[item->type()].push_back(item);
}

void MapView::clearOverlayItems() {
  overlayItems.clear();
}

void MapView::setVisibleOverlayItemTypes(const QSet<QString>& itemTypes) {
  overlayItemTypes = itemTypes;
}

int MapView::getY(int x, int z) {
  int cx = floor(x / 16.0);
  int cz = floor(z / 16.0);
  QSharedPointer<Chunk> chunk(cache.fetch(cx, cz));
  return chunk ? chunk->depth[(x & 0xf) + (z & 0xf) * 16] : -1;
}

QList<QSharedPointer<OverlayItem>> MapView::getItems(int x, int y, int z) {
  QList<QSharedPointer<OverlayItem>> ret;
  int cx = floor(x / 16.0);
  int cz = floor(z / 16.0);
  QSharedPointer<Chunk> chunk(cache.fetch(cx, cz));

  if (chunk) {
    double invzoom = 10.0 / zoom;
    for (auto &type : overlayItemTypes) {
      // generated structures
      for (auto &item : overlayItems[type]) {
        double ymin = chunk->lowest;
        double ymax = depth;
        if (item->intersects(OverlayItem::Point(x, ymin, z),
                             OverlayItem::Point(x, ymax, z))) {
          ret.append(item);
        }
      }

      // entities
      auto itemRange = chunk->entities.equal_range(type);
      for (auto itItem = itemRange.first; itItem != itemRange.second;
          ++itItem) {
        double ymin = y - 4;
        double ymax = depth + 4;

        if ((*itItem)->intersects(
            OverlayItem::Point(x - invzoom/2, ymin, z - invzoom/2),
            OverlayItem::Point(x + 1 + invzoom/2, ymax, z + 1 + invzoom/2))) {
          ret.append(*itItem);
        }
      }
    }
  }
  return ret;
}
