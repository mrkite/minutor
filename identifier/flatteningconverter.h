/** Copyright (c) 2018, EtlamGit */
#ifndef FLATTENINGCONVERTER_H_
#define FLATTENINGCONVERTER_H_

#include "paletteentry.h"

class JSONArray;
class JSONObject;


class FlatteningConverter {
public:
  // singleton: access to global usable instance
  static FlatteningConverter &Instance();

  int addDefinitions(JSONArray *, int pack = -1);
  void enableDefinitions(int id);
  void disableDefinitions(int id);
//  const BlockData * getPalette();
  PaletteEntry * getPalette();
  const static int paletteLength = 16*4096;  // 4 bit data + 12 bit ID (4096)

private:
  // singleton: prevent access to constructor and copyconstructor
  FlatteningConverter();
  ~FlatteningConverter();
  FlatteningConverter(const FlatteningConverter &);
  FlatteningConverter &operator=(const FlatteningConverter &);

  void parseDefinition(JSONObject *block, int *parentID, int pack);
  PaletteEntry palette[paletteLength];
//  QList<QList<BlockInfo*> > packs;
};

#endif  // FLATTENINGCONVERTER_H_
