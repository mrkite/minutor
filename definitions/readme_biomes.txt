FAQ and detailed description for vanilla_biomes.json


The biome definition file contains all data needed to describe available Biomes.

Inside the "data" tag available biomes are defined:

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
      "rainfall": 0.9
    },
    [...]
  ]

"id"    - the SaveGameID used by Minecraft
"color" - color used in Biome Overly (e.g. AMIDST color code)
 if color is omitted a pseudo random color is calculated based on the hashed name

some special values can be derived from the Minecraft source code:

"watercolor"  - special color of water (in swamps)
"temperature" - default 0.5
"rainfall"    - default 0.5



All colors can be defined in a syntax readable by Qt::QColor::setNamedColor( QString )
 http://qt-project.org/doc/qt-4.8/qcolor.html#setNamedColor

This is basically all hex definitions like #rrggbb but also all named colors from
 http://www.w3.org/TR/SVG/types.html#ColorKeywords
are possible.



[EtlamGit]