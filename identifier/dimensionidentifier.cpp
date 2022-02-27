/** Copyright (c) 2013, Sean Kasun */

#include "dimensionidentifier.h"
#include "json/json.h"
#include "worldinfo.h"


//-------------------------------------------------------------------------------------------------
// DimensionInfo used for each defined & found Dimension

DimensionInfo::DimensionInfo()
  : name("Unknown Dimension")
  , path(".")
  , pathIsRegEx(false)
  , enabled(true)
  , scale(1)
  , minY(0)
  , maxY(255)
  , defaultY(255)
{}


//-------------------------------------------------------------------------------------------------
// DimensionIdentifier

DimensionIdentifier::DimensionIdentifier()
{
  dummy_dimension.enabled = false;
  dummy_dimension.name    = "Dummy Dimension";
}

DimensionIdentifier::~DimensionIdentifier() {
  for (int i = 0; i < packs.length(); i++) {
    for (int j = 0; j < packs[i].length(); j++)
      delete packs[i][j];
  }
}

DimensionIdentifier& DimensionIdentifier::Instance() {
  static DimensionIdentifier singleton;
  return singleton;
}


void DimensionIdentifier::enableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = true;
}

void DimensionIdentifier::disableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = false;
}

int DimensionIdentifier::addDefinitions(JSONArray *defs, int pack) {
  if (pack == -1) {
    pack = packs.length();
    packs.append(QList<DimensionInfo*>());
  }

  int len = defs->length();
  for (int i = 0; i < len; i++) {
    JSONObject *dimTag = dynamic_cast<JSONObject *>(defs->at(i));
    DimensionInfo *dim = new DimensionInfo();
    dim->enabled = true;
    // Minecraft namespace ID
    if (dimTag->has("id")) {
      dim->id = dimTag->at("id")->asString();
      // construct a default name from ID
      QString nid = QString(dim->id).replace("minecraft:","").replace("_"," ");
      QStringList parts = nid.toLower().split(' ', QString::SkipEmptyParts);
      for (int i = 0; i < parts.size(); i++)
        parts[i].replace(0, 1, parts[i][0].toUpper());
      dim->name = parts.join(" ");
    }
    // explicit name given
    if (dimTag->has("name"))
      dim->name = dimTag->at("name")->asString();
    // path is where the region folder is located
    if (dimTag->has("path"))
      dim->path = dimTag->at("path")->asString();
    // scale
    if (dimTag->has("scale"))
      dim->scale = dimTag->at("scale")->asNumber();
    // unused feature for RegEx in path
    if (dimTag->has("regex"))
      dim->pathIsRegEx = dimTag->at("regex")->asBool();
    if (dimTag->has("minY"))
      dim->minY = dimTag->at("minY")->asNumber();
    if (dimTag->has("maxY"))
      dim->maxY = dimTag->at("maxY")->asNumber();
    if (dimTag->has("defaultY"))
      dim->defaultY = dimTag->at("defaultY")->asNumber();

    definitions.append(dim);
    packs[pack].append(dim);
  }
  return pack;
}


int DimensionIdentifier::getDimensionIndex(const QString & dim_name) const
{
  for (int index = 0; index<definitions.length(); index++) {
    if (definitions[index]->name == dim_name)
      return index;
  }
  return -1;
}


const DimensionInfo & DimensionIdentifier::getDimensionInfo(int index) const
{
  if (index >= definitions.length())
    return dummy_dimension;
  else
    return *definitions[index];
}


const DimensionInfo & DimensionIdentifier::getDimensionInfo(const QString & dim_name) const
{
  for (DimensionInfo * dim: definitions) {
    if (dim->id == dim_name)
      return *dim;
  }
  return dummy_dimension;
}
