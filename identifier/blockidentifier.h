/** Copyright (c) 2013, Sean Kasun */
#ifndef BLOCKIDENTIFIER_H_
#define BLOCKIDENTIFIER_H_

#include <QString>
#include <QMap>
#include <QHash>
#include <QList>
#include <QColor>
#include <QJsonArray>
#include <QJsonObject>


class BlockInfo {
 public:
  BlockInfo();

  bool hasVariants() const;

  // special block attribute used during mob spawning detection
  bool isOpaque() const;
  bool isLiquid() const;
  bool doesBlockHaveSolidTopSurface() const;
  bool isBlockNormalCube() const;
  bool renderAsNormalBlock() const;
  bool canProvidePower() const;

  // special block type used during mob spawning detection
  bool isBedrock() const;
  bool isHopper() const;
  bool isStairs() const;
  bool isHalfSlab() const;
  bool isSnow() const;

  // special blocks with Biome based Grass, Foliage and Water colors
  bool biomeWater() const;
  bool biomeGrass() const;
  bool biomeFoliage() const;

  void setName(const QString &newname);
  void setBiomeGrass(bool value);
  void setBiomeFoliage(bool value);
  const QString &getName() const;

  // enabled for complete definition pack
  bool    enabled;

  // internal state
  double  alpha;
  QString blockstate;
  bool    variants;  // block_state dependant variants
  bool    transparent;
  bool    liquid;
  bool    rendernormal;
  bool    providepower;
  bool    spawninside;
  bool    spawnontop;   // special override used for Slabs/Stairs
  QColor  colors[16];

 private:
  QString name;
  // cache special block attributes used during mob spawning detection
  bool    bedrock;
  bool    hopper;
  bool    snow;
  bool    water;
  bool    grass;
  bool    foliage;
};

class BlockIdentifier {
 public:
  // singleton: access to global usable instance
  static BlockIdentifier &Instance();

  int  addDefinitions(QJsonArray defs, int pack = -1);
  void enableDefinitions(int id);
  void disableDefinitions(int id);
  const BlockInfo &getBlockInfo(uint hid) const;
  bool             hasBlockInfo(uint hid) const;

  QList<quint32> getKnownIds() const;

 private:
  // singleton: prevent access to constructor and copyconstructor
  BlockIdentifier();
  ~BlockIdentifier();
  BlockIdentifier(const BlockIdentifier &);
  BlockIdentifier &operator=(const BlockIdentifier &);

  void parseDefinition(QJsonObject block, BlockInfo *parent, int pack);
  QHash<uint, BlockInfo*>   blocks;
  QList<QList<BlockInfo*> > packs;
};

#endif  // BLOCKIDENTIFIER_H_
