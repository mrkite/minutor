FAQ and detailed description for vanilla_biomes.json


The biome definition file contains all data needed to describe available Biomes.

Starting with Minecraft 1.18 Biomes are stored as string. To be backwards compatible
the new tag "data18" is used to define Biomes:

  "data18": [
    {
      "id": "minecraft:ocean",
      "name": "Ocean",
      "color": "#000070"
    },
    {
      "id": "minecraft:plains",
      "color": "#8db360",
      "temperature": 0.8,
      "humidity": 0.4
    },
    [...]


"id"    - the named string ID used by Minecraft
"name"  - used for display purpose, can be ommited and will be guessed from "id"
"color" - color used in Biome Overlay (e.g. AMIDST color code)
 if color is omitted a pseudo random color is calculated based on the hashed name


Some special values can be derived from the Minecraft source code:

"watermodifier" - special color of water (in swamps)
"temperature"   - default 0.5
"humidity"      - default 0.5


Some stuff is hard coded inside Minecraft source code. To handle these edge cases,
some flags are guessed based on the name of a Biome. To override these guesses
some additional boolean flags can be used:

"ocean"      - used for Drawned spawn detection (guess trigger is "ocean")
"river"      - used for Drawned spawn detection (guess trigger is "river")
"swamp"      - used for hard coded foliage colors (guess trigger is "swamp")
"darkforest" - used for hard coded foliage colors (guess trigger is "dark forest")
"badlands"   - used for hard coded foliage colors (guess trigger is "badlands" and "mesa")


For Minecraft up to 1.17 Biomes were stored as numerical ID for that we have
the "data" tag to define Biomes:

  "data": [
    {
      "id": 0,
      "name": "Ocean",
      "color": "#000070"
    },
    [...]
    {
      "id": 6,
      "name": "Swampland",
      "color": "#07f9b2",
      "watercolor" : 14745518,
      "temperature": 0.8,
      "humidity": 0.9
    },
    [...]
  ]

"id"    - the SaveGameID used by Minecraft
all others like with "data18"



All colors can be defined in a syntax readable by Qt::QColor::setNamedColor( QString )
 http://qt-project.org/doc/qt-4.8/qcolor.html#setNamedColor

This is basically all hex definitions like #rrggbb but also all named colors from
 http://www.w3.org/TR/SVG/types.html#ColorKeywords
are possible.



[EtlamGit]