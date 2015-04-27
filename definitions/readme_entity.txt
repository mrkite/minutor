FAQ and detailed description for vanilla_entity.json


The entity definition file contains all data needed to render the Entity Overlay.

With the "data" tag the available categories are defined:

  "data": [
    { "category": "Hostile", "catcolor": "#ff0000",   "entity": [ ... ] }
    { "category": "Passive", "catcolor": "#ffffff",   "entity": [ ... ] }
    { "category": "Others",  "catcolor": "lightgray", "entity": [ ... ] }
    { "category": "Items",   "catcolor": "#0000ff",   "entity": [ ... ] }
  ]

The "category" name is used in the View->EntityOverlay menu to select rendered
Entites by category. The name is a must field.

The "catcolor" defines the base color for that category. It is also used in
the View->EntityOverlay menu, but can be omitted. In that case a pseudo random
color is calculated based on the hashed category name.

The Entities belonging to that category are listed in the "entity" tag:

Each Entity definition follows this scheme:
  { "id": "ArmorStand", "catcolor": "Violet", "color": "Aquamarine" }

The SaveGameID used by Minecraft has to be used as "id" and is used to identify
the Entity. The base color can be given by "catcolor" but will be inherited from
the category if omitted. The highlight color (unique for each entity) is given
by "color". If omitted a pseudo random color is calculated based on the hashed id.



All colors can be defined in a syntax readable by Qt::QColor::setNamedColor( QString )
 http://qt-project.org/doc/qt-4.8/qcolor.html#setNamedColor

This is basically all hex definitions like #rrggbb but also all named colors from
 http://www.w3.org/TR/SVG/types.html#ColorKeywords
are possible.



The defined categories are based on the way Minecraft handles Mobs internally.
Hostile:
 Everything that can hurt the player and typically despawns (except boss Mobs)
Passive:
 All Mobs that typically don't despawn and only may hurt the player if attacked
Items:
 All items lying on the ground. They can not be further identified, as the
 SaveGameID is the same for all Items.
Others:
 All the rest. Mostely decorative and transportation stuff.



[EtlamGit]