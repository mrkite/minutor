/** Copyright (c) 2013, Sean Kasun */
#ifndef DIMENSIONIDENTIFIER_H_
#define DIMENSIONIDENTIFIER_H_

#include <QString>
#include <QList>
#include <QHash>
#include <QDir>

class QMenu;
class QAction;
class QActionGroup;
class JSONArray;

class DimensionInfo {
 public:
  DimensionInfo(QString path, int scale, QString name) : path(path),
    scale(scale), name(name) {}
  QString path;
  int scale;
  QString name;
};

class DimensionDef;

class DimensionIdentifier : public QObject {
  Q_OBJECT

 public:
  // singleton: access to global usable instance
  static DimensionIdentifier &Instance();

  int addDefinitions(JSONArray *, int pack = -1);
  void enableDefinitions(int id);
  void disableDefinitions(int id);
  void getDimensions(QDir path, QMenu *menu, QObject *parent);
  void removeDimensions(QMenu *menu);

 signals:
  void dimensionChanged(const DimensionInfo &dim);

 private slots:
  void viewDimension();

 private:
  // singleton: prevent access to constructor and copyconstructor
  DimensionIdentifier();
  ~DimensionIdentifier();
  DimensionIdentifier(const DimensionIdentifier &);
  DimensionIdentifier &operator=(const DimensionIdentifier &);

  void addDimension(QDir path, QString dir, QString name, int scale,
                    QObject *parent);
  QList<QAction *> items;
  QList<DimensionInfo> dimensions;
  QList<DimensionDef*> definitions;
  QList<QList<DimensionDef*> > packs;
  QActionGroup *group;

  QHash<QString, bool> foundDimensions;
};

#endif  // DIMENSIONIDENTIFIER_H_
