/** Copyright (c) 2018, EtlamGit */

#include <QDebug>
#include <assert.h>
#include <cmath>

#include "flatteningconverter.h"
#include "json/json.h"

const QString PaletteEntry::legacyBlockIdProperty = "lbid";

FlatteningConverter::FlatteningConverter() {
  // To ensure that unknown legacy (pre-flattening) block IDs
  // are shown as unknown, and to track their legacy ID to
  // help definition pack creators, prepopulate the palette array.

  // TODO: Hoist "Unknown Block" literal into constant
  QString unknownBlockName = "Unknown Block";
  uint unknownBlockHID = qHash(unknownBlockName);
  for (int idx = 0; idx < paletteLength; idx++) {
    palette[idx].hid = unknownBlockHID;
    palette[idx].name = unknownBlockName;
    palette[idx].properties[PaletteEntry::legacyBlockIdProperty] = idx;
  }
}

FlatteningConverter::~FlatteningConverter() {}

FlatteningConverter& FlatteningConverter::Instance() {
  static FlatteningConverter singleton;
  return singleton;
}

//const BlockData * FlatteningConverter::getPalette() {
PaletteEntry * FlatteningConverter::getPalette() {
  return palette;
}

void FlatteningConverter::enableDefinitions(int /*pack*/) {
//  if (pack < 0) return;
//  int len = packs[pack].length();
//  for (int i = 0; i < len; i++)
//    packs[pack][i]->enabled = true;
}

void FlatteningConverter::disableDefinitions(int /*pack*/) {
//  if (pack < 0) return;
//  int len = packs[pack].length();
//  for (int i = 0; i < len; i++)
//    packs[pack][i]->enabled = false;
}

int FlatteningConverter::addDefinitions(JSONArray *defs, int pack) {
//  if (pack == -1) {
//    pack = packs.length();
//    packs.append(QList<BlockInfo*>());
//  }
  int len = defs->length();
  for (int i = 0; i < len; i++)
    parseDefinition(dynamic_cast<JSONObject *>(defs->at(i)), NULL, pack);
  return pack;
}

void FlatteningConverter::parseDefinition(
        JSONObject *b,
        int *parentID,
        int pack) {
  // try to translate old block name into new flatname
  QString flatname;
  if (b->has("name")) {
    flatname = b->at("name")->asString().toLower().replace(" ", "_");
    // Put in minecraft: namespace if not in another (e.g., mod) one
    if (flatname.indexOf(':') == -1) {
        flatname = "minecraft:" + flatname;
    }
  } else if (parentID != NULL) {
    flatname = palette[*parentID].name;
  } else {
    flatname = "Unknown";
  }

  // or use provided flatname instead
  if (b->has("flatname"))
    flatname = b->at("flatname")->asString();

  // get the ancient block ID
  int bid, data(0);
  if (parentID == NULL) {
    bid = b->at("id")->asNumber();
  } else {
    bid = *parentID;
    data = b->at("data")->asNumber();
    // Shift enough to never overlap 0-4095 range
    bid |= data << 12;
  }

  palette[bid].name = flatname;
  palette[bid].hid  = qHash(palette[bid].name);
  if ((parentID == NULL) && (data == 0)) {
    // spread main block type for data == 0
    // Spread values must be spaced by 4096 to not collide
    // with non-spread IDs.
    for (int d=1; d<16; d++) {
      int sid = bid | (d<<12);
      palette[sid].name = flatname;
      palette[sid].hid  = palette[bid].hid;
    }
  }
  //  packs[pack].append(block);

  // get optional mask value (or guess default)
  int mask = 0;
  if (b->has("mask")) {
    mask = b->at("mask")->asNumber();
  } else if (b->has("variants")) {
    mask = 0x0f;
  }

  // recursive parsing of variants (with data)
  if (b->has("variants")) {
    JSONArray *variants = dynamic_cast<JSONArray *>(b->at("variants"));
    int vlen = variants->length();
    for (int j = 0; j < vlen; j++) {
      parseDefinition(dynamic_cast<JSONObject *>(variants->at(j)), &bid, pack);
    }
    // spread variants in masked bid
    // Variants must be spaced at least 4096 apart to ensure
    // no collisions with non-variant IDs (e.g., from mods)
    for (int j = vlen; j < 16; j++) {
      int id  = bid | (j << 12);
      int mid = bid | ((j & mask) << 12);
      palette[id].name = palette[mid].name;
      palette[id].hid  = palette[mid].hid;
    }

  }
}
