/** Copyright (c) 2013, Sean Kasun */
#ifndef BIOMEIDENTIFIER_H_
#define BIOMEIDENTIFIER_H_

#include <QHash>
#include <QList>
#include <QString>
#include <QColor>
#include <QJsonArray>
#include <QJsonObject>


class BiomeInfo {
  // public methods
 public:
  BiomeInfo();
  QColor getBiomeGrassColor  ( QColor blockcolor, int elevation ) const;
  QColor getBiomeFoliageColor( QColor blockcolor, int elevation ) const;
  QColor getBiomeWaterColor( QColor watercolor ) const;

  bool   isOceanBiome() const { return ocean; };
  bool   isRiverBiome() const { return river; };

  // public members
 public:
  int     id;   // numerical ID
  QString nid;  // namespace ID
  QString name;
  bool    enabled;
  bool    ocean;
  bool    river;
  bool    swamp;
  bool    darkforest;
  bool    badlands;
  double  temperature;
  double  humidity;
  bool    enabledwatermodifier;
  QColor  watermodifier;
  QColor  colors[16];

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
  static QColor mixColor( QColor colorizer, QColor blockcolor );
};

class BiomeIdentifier {
 public:
  // singleton: access to global usable instance
  static BiomeIdentifier &Instance();

  int  addDefinitions(QJsonArray data, QJsonArray data18, int pack = -1);
  void enableDefinitions(int id);
  void disableDefinitions(int id);
  void updateBiomeDefinition();
  const BiomeInfo &getBiomeByChunk  (qint32 id) const;
  const BiomeInfo &getBiomeBySection(qint32 id) const;
  const BiomeInfo &getBiomeByName   (QString id) const;

private:
  // singleton: prevent access to constructor and copyconstructor
  BiomeIdentifier();
  ~BiomeIdentifier();
  BiomeIdentifier(const BiomeIdentifier &);
  BiomeIdentifier &operator=(const BiomeIdentifier &);

  void parseBiomeDefinitions0000(QJsonArray data,   int pack);
  void parseBiomeDefinitions2800(QJsonArray data18, int pack);
  void guessSpecialBiomes(QJsonObject b, BiomeInfo *biome);

  // legacy Biomes
  QHash<int, BiomeInfo*>    biomes;   // consolidated Biome mapping
  QList<QList<BiomeInfo*> > packs;    // raw data of all available packs
  // new Biomes after Cliffs & Caves update (1.18)
  QList<BiomeInfo*>         biomes18; // consolidated Biome mapping
  QList<QList<BiomeInfo*> > packs18;  // raw data of all available packs
};

#endif  // BIOMEIDENTIFIER_H_
