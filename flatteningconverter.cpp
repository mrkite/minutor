/** Copyright (c) 2018, EtlamGit */

#include <QDebug>
#include <assert.h>
#include <cmath>

#include "./FlatteningConverter.h"
#include "./json.h"



FlatteningConverter::FlatteningConverter() {}

FlatteningConverter::~FlatteningConverter() {}

FlatteningConverter& FlatteningConverter::Instance() {
  static FlatteningConverter singleton;
  return singleton;
}

const BlockData * FlatteningConverter::getPalette() {
  return palette;
}

void FlatteningConverter::enableDefinitions(int pack) {
//  if (pack < 0) return;
//  int len = packs[pack].length();
//  for (int i = 0; i < len; i++)
//    packs[pack][i]->enabled = true;
}

void FlatteningConverter::disableDefinitions(int pack) {
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

  // get the ancient block ID
  int bid;
  if (parentID == NULL) {
    bid = b->at("id")->asNumber();
  } else {
    bid = *parentID;
    int data = b->at("data")->asNumber();
    bid |= data << 8;
  }

  // try to translate old block name into new flatname
  QString flatname;
  if (b->has("name")) {
    flatname = "minecraft:" + b->at("name")->asString().toLower().replace(" ", "_");
  } else if (parentID != NULL) {
    flatname = palette[*parentID].name;
  } else {
    flatname = "Unknown";
  }

  // or use provided flatname instead
  if (b->has("flatname"))
    flatname = b->at("flatname")->asString();

  palette[bid].name = flatname;
  //  packs[pack].append(block);

  // recursive parsing of variants (with data)
  if (b->has("variants")) {
    JSONArray *variants = dynamic_cast<JSONArray *>(b->at("variants"));
    int vlen = variants->length();
    for (int j = 0; j < vlen; j++)
      parseDefinition(dynamic_cast<JSONObject *>(variants->at(j)), &bid, pack);
  }
}
