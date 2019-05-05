/** Copyright (c) 2013, Sean Kasun */
#ifndef DEFINITIONMANAGER_H_
#define DEFINITIONMANAGER_H_

#include <QtWidgets/QWidget>
#include <QHash>
#include <QString>
#include <QList>
#include <QVariant>
#include <QDateTime>

class QTableWidget;
class QTableWidgetItem;
class QCheckBox;
class BiomeIdentifier;
class BlockIdentifier;
class DimensionIdentifier;
class EntityIdentifier;
class FlatteningConverter;
class MapView;
class JSONData;
class DefinitionUpdater;

struct Definition {
  QString name;
  QString version;
  QString path;
  QString update;
  enum {Block, Biome, Dimension, Entity, Pack, Converter} type;
  int id;
  bool enabled;
  // for packs only
  int blockid, biomeid, dimensionid, entityid;
};

class DefinitionManager : public QWidget {
  Q_OBJECT

 public:
  explicit DefinitionManager(QWidget *parent = 0);
  ~DefinitionManager();
  void attachMapView(MapView *mapview);
  QSize minimumSizeHint() const;
  QSize sizeHint() const;

  void autoUpdate();

 signals:
  void packSelected(bool on);
  void packsChanged();
  void updateFinished();

 public slots:
  void selectedPack(QTableWidgetItem *, QTableWidgetItem *);
  void toggledPack(bool on);
  void addPack();
  void removePack();
  void exportPack();
  void checkForUpdates();
  void updatePack(DefinitionUpdater *updater,
                  QString filename,
                  QString version);

 private:
  QTableWidget *table;
  QList<QCheckBox *>checks;
  void installJson(QString path, bool overwrite = true, bool install = true);
  void installZip(QString path, bool overwrite = true, bool install = true);
  void checkAndRepair();
  void loadDefinition(QString path);
  void loadDefinition(JSONData *, int pack = -1);
  void removeDefinition(QString path);
  void refresh();
  QHash<QString, Definition> definitions;
  BiomeIdentifier     &biomeManager;
  BlockIdentifier     &blockManager;
  DimensionIdentifier &dimensionManager;
  EntityIdentifier    &entityManager;
  FlatteningConverter &flatteningConverter;
  QString selected;
  QList<QVariant> sorted;

  bool isUpdating;
  QList<DefinitionUpdater *> updateQueue;
};

#endif  // DEFINITIONMANAGER_H_
