/*
Copyright (c) 2013, Sean Kasun
Copyright (c) 2016, EtlamGit
*/

#include <assert.h>

#include "./biomeidentifier.h"
#include "./json.h"

// --------- --------- --------- ---------
// BiomeInfo
// --------- --------- --------- ---------

static BiomeInfo unknownBiome;

BiomeInfo::BiomeInfo()
  : name("Unknown Biome")
  , enabled(false)
  , watermodifier(255,255,255)
  , enabledwatermodifier(false)
  , alpha(1.0)
  , temperature(0.5)
  , humidity(0.5)
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

// will be in STD with C++17
template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
  return (n<lower)? lower : (n>upper)? upper : n;
}

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
  elevation = (elevation < 0) ? 0 : elevation;

  // get local temperature and humidity
  temperature = clamp( temperature - (float)elevation*0.00166667f, 0.0f, 1.0f );
  humidity    = clamp( humidity, 0.0f, 1.0f );
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

  int r = (int)clamp( red,   0.0f, 255.0f );
  int g = (int)clamp( green, 0.0f, 255.0f );
  int b = (int)clamp( blue,  0.0f, 255.0f );

  return QColor::QColor(r,g,b);
}

QColor BiomeInfo::getBiomeGrassColor( int elevation )
{
  return getBiomeColor( this->temperature, this->humidity, elevation, grassCorners );
}

QColor BiomeInfo::getBiomeFoliageColor( int elevation )
{
  return getBiomeColor( this->temperature, this->humidity, elevation, foliageCorners );
}

QColor BiomeInfo::getBiomeWaterColor( QColor watercolor )
{
  if (this->enabledwatermodifier) {
    // calculate modified color components
    int r = (int)( watercolor.red()   * watermodifier.red()   / 255 );
    int g = (int)( watercolor.green() * watermodifier.green() / 255 );
    int b = (int)( watercolor.blue()  * watermodifier.blue()  / 255 );
    // return combined modified color
    return QColor::QColor(r, g, b );
  } else
    return watercolor;
}


// --------- --------- --------- ---------
// BiomeIdentifier
// --------- --------- --------- ---------

BiomeIdentifier::BiomeIdentifier() {
  unknownBiome.name = "Unknown";
}

BiomeIdentifier::~BiomeIdentifier() {
  for (int i = 0; i < packs.length(); i++) {
    for (int j = 0; j < packs[i].length(); j++)
      delete packs[i][j];
  }
}

BiomeInfo &BiomeIdentifier::getBiome(int biome) {
  QList<BiomeInfo*> &list = biomes[biome];
  // search backwards for priority sorting to work
  for (int i = list.length() - 1; i >= 0; i--)
    if (list[i]->enabled)
      return *list[i];
  return unknownBiome;
}

void BiomeIdentifier::enableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = true;
}
void BiomeIdentifier::disableDefinitions(int pack) {
  if (pack < 0) return;
  int len = packs[pack].length();
  for (int i = 0; i < len; i++)
    packs[pack][i]->enabled = false;
}

static int clamp(int v, int min, int max) {
  return (v < max ? (v > min ? v : min) : max);
}

int BiomeIdentifier::addDefinitions(JSONArray *defs, int pack) {
  if (pack == -1) {
    pack = packs.length();
    packs.append(QList<BiomeInfo *>());
  }

  int len = defs->length();
  for (int i = 0; i < len; i++) {
    JSONObject *b = dynamic_cast<JSONObject *>(defs->at(i));
    int id = b->at("id")->asNumber();

    BiomeInfo *biome = new BiomeInfo();
    biome->enabled = true;
    if (b->has("name"))
      biome->name = b->at("name")->asString();

    // check for "alpha" (0: transparent / 1: saturated)
    // probably never used
    if (b->has("alpha"))
      biome->alpha = b->at("alpha")->asNumber();

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

    // pre-calculate light spectrum
    for (int i = 0; i < 16; i++) {
      // calculate light attenuation similar to Minecraft
      // except base 90% here, were Minecraft is using 80% per level
      double light_factor = pow(0.90,15-i);
      biome->colors[i].setRgb(light_factor*biomecolor.red(),
                              light_factor*biomecolor.green(),
                              light_factor*biomecolor.blue(),
                              255*biome->alpha );
    }

    biomes[id].append(biome);
    packs[pack].append(biome);
  }

  return pack;
}
