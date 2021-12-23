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
  DimensionInfo();

  QString id;
  QString name;
  QString path;
  bool pathIsRegEx;
  bool enabled;
  int  scale;
  int  minY;
  int  maxY;
  int  defaultY;
};


class DimensionIdentifier : public QObject {
  Q_OBJECT

 public:
  // singleton: access to global usable instance
  static DimensionIdentifier &Instance();

  // definition parsing
  int  addDefinitions(JSONArray *, int pack = -1);
  void enableDefinitions(int id);
  void disableDefinitions(int id);
  // Dimesnion view menu
  void clearDimensionsMenu(QMenu *menu);
  void getDimensionsInWorld(QDir path, QMenu *menu, QObject *parent);

 signals:
  void dimensionChanged(const DimensionInfo &dim);    // dimension changed in menu

 private slots:
  void changeViewToDimension();                       // dimension changed in menu

 private:
  // singleton: prevent access to constructor and copyconstructor
  DimensionIdentifier();
  ~DimensionIdentifier();
  DimensionIdentifier(const DimensionIdentifier &);
  DimensionIdentifier &operator=(const DimensionIdentifier &);

  void addDimensionMenu(QDir path, QString dir, QString name, QObject *parent);

  // GUI menu
  QList<QAction *> currentMenuActions;
  QActionGroup *   menuActionGroup;
  QList<QString>   foundDimensionDirs;  // all directories where we already found a Dimension
  // dimension storage
  QList<DimensionInfo*>         definitions;  // definition of possible Dimensions
  QList<QList<DimensionInfo*> > packs;
};

#endif  // DIMENSIONIDENTIFIER_H_
