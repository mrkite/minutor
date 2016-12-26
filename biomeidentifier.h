/** Copyright (c) 2013, Sean Kasun */
#ifndef BIOMEIDENTIFIER_H_
#define BIOMEIDENTIFIER_H_

#include <QHash>
#include <QList>
#include <QString>
#include <QColor>
class JSONArray;


class BiomeInfo {
  // public methods
 public:
  BiomeInfo();
  QColor getBiomeGrassColor  ( int elevation );
  QColor getBiomeFoliageColor( int elevation );
  QColor getBiomeWaterColor( QColor watercolor );

  // public members
 public:
  int id;
  QString name;
  bool enabled;
  QColor colors[16];
  QColor watermodifier;
  bool   enabledwatermodifier;
  double alpha;
  double temperature;
  double humidity;

  // private methods and members
 private:
  typedef struct T_BiomeCorner {
    int red;
    int green;
    int blue;
  } T_BiomeCorner;
  static T_BiomeCorner grassCorners[3];
  static T_BiomeCorner foliageCorners[3];
  static QColor getBiomeColor( float temperature, float humidity, int elevation, T_BiomeCorner *corners );
};

class BiomeIdentifier {
 public:
  BiomeIdentifier();
  ~BiomeIdentifier();
  int addDefinitions(JSONArray *, int pack = -1);
  void enableDefinitions(int id);
  void disableDefinitions(int id);
  BiomeInfo &getBiome(int id);
 private:
  QHash<int, QList<BiomeInfo*>> biomes;
  QList<QList<BiomeInfo*> > packs;
};

#endif  // BIOMEIDENTIFIER_H_
