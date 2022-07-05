/** Copyright (c) 2013, Sean Kasun */
#ifndef DIMENSIONIDENTIFIER_H_
#define DIMENSIONIDENTIFIER_H_

#include <QString>
#include <QList>
#include <QHash>
#include <QDir>
#include <QJsonArray>


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


class DimensionIdentifier {
 public:
  // singleton: access to global usable instance
  static DimensionIdentifier &Instance();

  // definition parsing
  int  addDefinitions(QJsonArray defs, int pack = -1);
  void enableDefinitions(int id);
  void disableDefinitions(int id);

  int getDimensionIndex(const QString & dim_name) const;
  const DimensionInfo & getDimensionInfo(int index) const;
  const DimensionInfo & getDimensionInfo(const QString & dim_name) const;

 private:
  // singleton: prevent access to constructor and copyconstructor
  DimensionIdentifier();
  ~DimensionIdentifier();
  DimensionIdentifier(const DimensionIdentifier &);
  DimensionIdentifier &operator=(const DimensionIdentifier &);

  // dimension storage
  QList<DimensionInfo*>         definitions;  // definition of possible Dimensions
  QList<QList<DimensionInfo*> > packs;

  DimensionInfo dummy_dimension; // dummy in case no matching dimension is available
};

#endif  // DIMENSIONIDENTIFIER_H_
