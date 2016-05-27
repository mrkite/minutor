/** Copyright (c) 2013, Sean Kasun */
#ifndef BLOCKIDENTIFIER_H_
#define BLOCKIDENTIFIER_H_

#include <QString>
#include <QMap>
#include <QHash>
#include <QList>

class JSONArray;
class JSONObject;

// bit masks for the flags
#define BlockTransparent 1
#define BlockSolid 2
#define BlockLiquid 4

// mobs can't spawn on transparent, but need 2 blocks of transparent,
// non solid, non liquid above

class BlockInfo {
 public:
  BlockInfo();
  bool isOpaque();
  bool isLiquid();
  bool doesBlockHaveSolidTopSurface(int data);
  bool isBlockNormalCube();
  bool renderAsNormalBlock();
  bool canProvidePower();

  // special blocks used during mob spawning detection
  bool isBedrock();
  bool isHopper();
  bool isStairs();
  bool isHalfSlab();
  bool isSnow();

  void setName(const QString &newname);
  const QString &getName();

  int id;
  double alpha;
  quint8 mask;
  bool enabled;
  bool transparent;
  bool liquid;
  bool rendernormal;
  bool providepower;
  bool spawninside;
  quint32 colors[16];

 private:
  QString name;
  // cache special blocks used during mob spawning detection
  bool    bedrock;
  bool    hopper;
  bool    stairs;
  bool    halfslab;
  bool    snow;
};

class BlockIdentifier {
 public:
  BlockIdentifier();
  ~BlockIdentifier();
  int addDefinitions(JSONArray *, int pack = -1);
  void enableDefinitions(int id);
  void disableDefinitions(int id);
  BlockInfo &getBlock(int id, int data);
 private:
  void clearCache();
  void parseDefinition(JSONObject *block, BlockInfo *parent, int pack);
  QMap<quint32, QList<BlockInfo *>> blocks;
  QList<QList<BlockInfo*> > packs;
  BlockInfo *cache[65536];
};

#endif  // BLOCKIDENTIFIER_H_
