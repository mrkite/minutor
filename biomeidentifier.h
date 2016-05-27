/** Copyright (c) 2013, Sean Kasun */
#ifndef BIOMEIDENTIFIER_H_
#define BIOMEIDENTIFIER_H_

#include <QHash>
#include <QList>
#include <QString>
class JSONArray;

class BiomeInfo {
 public:
  BiomeInfo() {}
  QString name;
  bool enabled;
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
