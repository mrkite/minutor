/*
Copyright (c) 2013, Sean Kasun
Copyright (c) 2016, EtlamGit
*/

#include <assert.h>
#include <cmath>
#include <QtCore>

#include "biomeidentifier.h"
#include "json/json.h"
#include "clamp.h"

// --------- --------- --------- ---------
// BiomeInfo
// --------- --------- --------- ---------

static BiomeInfo unknownBiome;

BiomeInfo::BiomeInfo()
  : id(-1)
  , nid("")
  , name("Unknown Biome")
  , enabled(false)
  , temperature(0.5)
  , humidity(0.5)
  , enabledwatermodifier(false)
  , watermodifier(255,255,255)
{}


// Biome based Grass and Foliage color is based on an interpolation in special
// textures provided in the Minecraft assets. It is also possible to do the
// interpolation based only on the color values at the threee corners of that
// color triangle.

// .\assets\minecraft\textures\colormap\grass.png
// last checked with Minecraft 1.11.2
BiomeInfo::T_BiomeCorner BiomeInfo::grassCorners[3] =
{
  { 191, 183,  85 },  // lower left  - temperature = 1.0
  { 128, 180, 151 },  // lower right - temperature = 0.0
  {  71, 205,  51 }   // upper left  - humidity = 1.0
};

// .\assets\minecraft\textures\colormap\foliage.png
// last checked with Minecraft 1.11.2
BiomeInfo::T_BiomeCorner BiomeInfo::foliageCorners[3] =
{
  { 174, 164,  42 },  // lower left  - temperature = 1.0
  {  96, 161, 123 },  // lower right - temperature = 0.0
  {  26, 191,   0 }   // upper left  - humidity = 1.0
};

/*
This method is inspired from Mineways, Copyright (c) 2014, Eric Haines
https://github.com/erich666/Mineways/blob/master/Win/biomes.h
*/
// do an interpolation inside the color triangle given by the corners
// temperature: is the base value for the current Biome
// humidity:    is the base value for the current Biome
// elevation:   is number of meters above a height of 64
QColor BiomeInfo::getBiomeColor( float temperature, float humidity, int elevation, T_BiomeCorner *corners )
{
  // check elevation
  // todo: probably negative values are allowed with latest Minecraft versions
//  elevation = (elevation < 0) ? 0 : elevation;

  // get local temperature and humidity
  // perlin noise generator omitted due to performance reasons
  temperature = std::clamp( temperature - (float)elevation*0.00166667f, 0.0f, 1.0f );
  humidity    = std::clamp( humidity, 0.0f, 1.0f );
  humidity   *= temperature;  // scale by temperature to stay in triangle

  // lambda values for barycentric coordinates
  float lambda[3];
  lambda[0] = temperature - humidity;
  lambda[1] = 1.0f - temperature;
  lambda[2] = humidity;

  float red = 0.0f, green = 0.0f, blue = 0.0f;
  for ( int i = 0; i < 3; i++ )
  {
    red   += lambda[i] * corners[i].red;
    green += lambda[i] * corners[i].green;
    blue  += lambda[i] * corners[i].blue;
  }

  int r = (int)std::clamp( red,   0.0f, 255.0f );
  int g = (int)std::clamp( green, 0.0f, 255.0f );
  int b = (int)std::clamp( blue,  0.0f, 255.0f );

  return QColor::fromRgb(r,g,b);
}

QColor BiomeInfo::mixColor( QColor colorizer, QColor blockcolor )
{
  // get hue components
  float hueC = colorizer.hslHueF();
  float hueB = blockcolor.hslHueF();  // monochrome blocks result in -1
  // get saturation components
  float satC = colorizer.hslSaturationF();
  float satB = blockcolor.hslSaturationF();
  // get lightness components
  float ligC = colorizer.lightnessF();
  float ligB = blockcolor.lightnessF();

  // mix final color
  float hue = hueC;
  float sat = satC;
  if ((satB != 0) && (hueB != -1.0f)) {
    // when block contains some color component
    hue = (hueC + hueB ) / 2.0f;
    sat = (satC + satB ) / 2.0f;
  }
//float lig = std::clamp( ligC + ligB - 0.5f, 0.0f, 1.0f );  // more brightness contrast
  float lig = (ligC + ligB ) / 2.0f;
  return QColor::fromHslF( hue, sat, lig );
}


QColor BiomeInfo::getBiomeGrassColor( QColor blockcolor, int elevation ) const
{
  QColor colorizer;
  // remove variants from ID
  int id = this->id & 0x7f;
  // swampland
  if (id == 6) {
    // perlin noise generator omitted due to performance reasons
    // otherwise the random temperature distribution selects
    // (below -0.1°C) ‭4C.76.3C‬ or ‭6A.70.39 (above -0.1°C)
    colorizer = QColor::fromRgb(0x6a,0x70,0x39);  // hard wired
  }
  // roofed forest
  else if (id == 29) {
    colorizer = getBiomeColor( this->temperature, this->humidity, elevation, grassCorners );
    // average with 0x28340A
    colorizer.setRed  ( (colorizer.red()   + 0x28)>>1 );
    colorizer.setGreen( (colorizer.green() + 0x34)>>1 );
    colorizer.setBlue ( (colorizer.blue()  + 0x0A)>>1 );
  }
  // mesa
  else if ((id == 37) || (id == 38) || (id == 39)) {
    colorizer = QColor::fromRgb(0x90,0x81,0x4d);  // hard wired
  } else
    // standard way
    colorizer = getBiomeColor( this->temperature, this->humidity, elevation, grassCorners );

  return mixColor( colorizer, blockcolor );
}

QColor BiomeInfo::getBiomeFoliageColor( QColor blockcolor, int elevation ) const
{
  QColor colorizer;
  // remove variants from ID
  int id = this->id & 0x7f;
  // swampland
  if (id == 6) {
    colorizer = QColor::fromRgb(0x6a,0x70,0x39);  // hard wired
  }
  // mesa
  else if ((id == 37) || (id == 38) || (id == 39)) {
    colorizer = QColor::fromRgb(0x9e,0x81,0x4d);  // hard wired
  } else
    // standard way
    colorizer = getBiomeColor( this->temperature, this->humidity, elevation, foliageCorners );

  return mixColor( colorizer, blockcolor );
}

QColor BiomeInfo::getBiomeWaterColor( QColor watercolor ) const
{
  if (this->enabledwatermodifier) {
    // calculate modified color components
    int r = (int)( watercolor.red()   * watermodifier.redF() );
    int g = (int)( watercolor.green() * watermodifier.greenF() );
    int b = (int)( watercolor.blue()  * watermodifier.blueF() );
    // return combined modified color
    return QColor::fromRgb(r, g, b);
  } else
    return watercolor;
}


// --------- --------- --------- ---------
// BiomeIdentifier
// --------- --------- --------- ---------

BiomeIdentifier::BiomeIdentifier() {}

BiomeIdentifier::~BiomeIdentifier() {
  for (int i = 0; i < packs.length(); i++) {
    for (int j = 0; j < packs[i].length(); j++)
      delete packs[i][j];
  }
}

BiomeIdentifier& BiomeIdentifier::Instance() {
  static BiomeIdentifier singleton;
  return singleton;
}

// legacy Biomes
const BiomeInfo &BiomeIdentifier::getBiome(int id) const {
  auto itr = this->biomes.find(id);
  if (itr == this->biomes.end()) {
    return unknownBiome;
  } else {
    return *(*itr);
  }
}

// new Biomes after Cliffs & Caves update (1.18)
const BiomeInfo &BiomeIdentifier::getBiome(quint8 id) const {
  if (id < this->biomes18.length())
    return *(this->biomes18.at(id));
  else
    return unknownBiome;
}

const BiomeInfo &BiomeIdentifier::getBiome(QString id) const {
  for (const BiomeInfo* biome : this->biomes18) {
    if (biome->nid == id) return *(biome);
  }
#if defined(DEBUG) || defined(_DEBUG) || defined(QT_DEBUG)
  qWarning() << "Unknown Biome:" << id;
#endif
  return unknownBiome;
}

void BiomeIdentifier::enableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = true;
  updateBiomeDefinition();
}
void BiomeIdentifier::disableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = false;
  updateBiomeDefinition();
}

int BiomeIdentifier::addDefinitions(JSONArray *data, JSONArray *data18, int pack) {
  if (pack == -1) {
    pack = packs.length();
    packs  .append(QList<BiomeInfo *>());
    packs18.append(QList<BiomeInfo *>());
  }

  if (data)   parseBiomeDefinitions0000(data, pack);
  if (data18) parseBiomeDefinitions2800(data18, pack);

  updateBiomeDefinition();
  return pack;
}

// legacy Biome definitions before Cliffs & Caves (up to 1.17)
void BiomeIdentifier::parseBiomeDefinitions0000(JSONArray *data, int pack) {
  int len = data->length();
  for (int i = 0; i < len; i++) {
    JSONObject *b = dynamic_cast<JSONObject *>(data->at(i));
    if (b->has("id")) {
      int id = b->at("id")->asNumber();

      BiomeInfo *biome = new BiomeInfo();
      biome->enabled = true;
      biome->id = id;
      if (b->has("name"))
        biome->name = b->at("name")->asString();

      // get temperature definition
      if (b->has("temperature"))
        biome->temperature = b->at("temperature")->asNumber();

      // get humidity definition
      if (b->has("humidity"))
        biome->humidity = b->at("humidity")->asNumber();

      // get watermodifier definition
      if (b->has("watermodifier")) {
        biome->watermodifier.setNamedColor(b->at("watermodifier")->asString());
        biome->enabledwatermodifier = true;
        assert(biome->watermodifier.isValid());
      }

      // get color definition
      QColor biomecolor;
      if (b->has("color")) {
        QString colorname = b->at("color")->asString();
        if (colorname.length() == 6) {
          // check if this is an old color definition with missing '#'
          bool ok;
          colorname.toInt(&ok,16);
          if (ok)
            colorname.push_front('#');
        }
        biomecolor.setNamedColor(colorname);
        assert(biomecolor.isValid());
      } else {
        // use hashed by name instead
        quint32 hue = qHash(biome->name);
        biomecolor.setHsv(hue % 360, 255, 255);
      }

      // pre-calculate light spectrum for view mode "Biome Colors"
      for (int i = 0; i < 16; i++) {
        // calculate light attenuation similar to Minecraft
        // except base 90% here, were Minecraft is using 80% per level
        double light_factor = pow(0.90,15-i);
        biome->colors[i].setRgb(light_factor*biomecolor.red(),
                                light_factor*biomecolor.green(),
                                light_factor*biomecolor.blue());
      }

      packs[pack].append(biome);
    }
  }
}

// new Biome definitions with Cliffs & Caves (1.18)
void BiomeIdentifier::parseBiomeDefinitions2800(JSONArray *data18, int pack) {
  int len = data18->length();
  for (int i = 0; i < len; i++) {
    JSONObject *b = dynamic_cast<JSONObject *>(data18->at(i));
    if (b->has("id")) {
      BiomeInfo *biome = new BiomeInfo();
      biome->enabled = true;
      biome->nid = b->at("id")->asString();
      biome->id = i;

      if (b->has("name"))
        biome->name = b->at("name")->asString();
      else {
        // construct the name from NID
        QString nid = QString(biome->nid).replace("minecraft:","").replace("_"," ");
        QStringList parts = nid.toLower().split(' ', QString::SkipEmptyParts);
        for (int i = 0; i < parts.size(); i++)
          parts[i].replace(0, 1, parts[i][0].toUpper());
        biome->name = parts.join(" ");
      }

      // get temperature definition
      if (b->has("temperature"))
        biome->temperature = b->at("temperature")->asNumber();

      // get humidity definition
      if (b->has("humidity"))
        biome->humidity = b->at("humidity")->asNumber();

      // get watermodifier definition
      biome->enabledwatermodifier = true;
      if (b->has("watermodifier")) {
        biome->watermodifier.setNamedColor(b->at("watermodifier")->asString());
        assert(biome->watermodifier.isValid());
      } else biome->watermodifier.setNamedColor("#3f76e4");

      // get color definition
      QColor biomecolor;
      if (b->has("color")) {
        QString colorname = b->at("color")->asString();
        if (colorname.length() == 6) {
          // check if this is an old color definition with missing '#'
          bool ok;
          colorname.toInt(&ok,16);
          if (ok)
            colorname.push_front('#');
        }
        biomecolor.setNamedColor(colorname);
        assert(biomecolor.isValid());
      } else {
        // use hashed by name instead
        quint32 hue = qHash(biome->name);
        biomecolor.setHsv(hue % 360, 255, 255);
      }

      // pre-calculate light spectrum for view mode "Biome Colors"
      for (int i = 0; i < 16; i++) {
        // calculate light attenuation similar to Minecraft
        // except base 90% here, were Minecraft is using 80% per level
        double light_factor = pow(0.90,15-i);
        biome->colors[i].setRgb(light_factor*biomecolor.red(),
                                light_factor*biomecolor.green(),
                                light_factor*biomecolor.blue());
      }

      packs18[pack].append(biome);
    }
  }
}


void BiomeIdentifier::updateBiomeDefinition()
{
  // start from scratch
  biomes.clear();
  biomes18.clear();

  for (int pack = 0; pack < packs.length(); pack++) {
    // legacy Biomes
    for (int i = 0; i < packs[pack].length(); i++) {
      BiomeInfo *bi = packs[pack][i];
      if (bi->enabled) {
        biomes[bi->id] = bi;
      }
    }

    // new Biomes after Cliffs & Caves update (1.18)
    for (int i = 0; i < packs18[pack].length(); i++) {
      BiomeInfo *bi = packs18[pack][i];
      if (bi->enabled) {
        biomes18.append(bi);
      }
    }
  }

}
