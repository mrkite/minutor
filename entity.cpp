/** Copyright 2014 EtlamGit */
#include <QPainter>
#include "entity.h"
#include "identifier/entityidentifier.h"
#include "nbt/nbt.h"

Entity::Entity(const Point &positionInfo)
    : extraColor(QColor::fromRgb(0,255,0))
    , pos(positionInfo)
{}

QSharedPointer<OverlayItem> Entity::TryParse(const Tag* tag) {
  EntityIdentifier& ei = EntityIdentifier::Instance();

  QSharedPointer<OverlayItem> ret;
  auto pos = tag->at("Pos");
  if (pos && pos != &NBT::Null) {
    Point p(pos->at(0)->toDouble(), pos->at(1)->toDouble(), pos->at(2)->toDouble());
    Entity* entity = new Entity(p);
    auto id = tag->at("id");
    if (id && id != &NBT::Null) {
      QString type = id->toString().toLower().remove("minecraft:");
      EntityInfo const & info = ei.getEntityInfo(type);

      QMap<QString, QVariant> props = tag->getData().toMap();

      // get something more descriptive if its an item
      if (type == "item") {
        auto itemId = tag->at("Item")->at("id");

        QString itemtype = itemId->toString();
        entity->setDisplay(itemtype.mid(itemtype.indexOf(':') + 1));
      } else {  // or just use the Entity's name
        if (info.name == "Name unknown")
          entity->setDisplay(type);       // use Minecraft internal name if not found
        else
          entity->setDisplay(info.name);  // use name as defined in JSON
      }
      entity->setType("Entity." + info.category);
      entity->setColor(info.brushColor);
      entity->setExtraColor(info.penColor);
      entity->setProperties(props);

      // parse POI of villagers / PiglinBrutes
      if (props.contains("Brain")) {
        QMap<QString, QVariant> brain = props["Brain"].toMap();
        if (brain.contains("memories")) {
          QMap<QString, QVariant> memories = brain["memories"].toMap();
          // home is location of bed
          entity->tryParseMemory(memories, "minecraft:home",               QColor(0,0,255));

          // location of job site
          entity->tryParseMemory(memories, "minecraft:job_site",           QColor(255,0,0));
          entity->tryParseMemory(memories, "minecraft:potential_job_site", QColor(255,0,0));

          // meeting point is location of bell
          entity->tryParseMemory(memories, "minecraft:meeting_point",      QColor(255,255,0));
        }
      }

      ret.reset(entity);
    }
  }
  return ret;
}

void Entity::tryParseMemory(const QMap<QString, QVariant> &memories,
                            const QString memory,
                            QColor color) {
  if (memories.contains(memory)) {
    QMap<QString, QVariant> location = memories[memory].toMap();
    QList<QVariant> pos;
    if (location.contains("value")) {
      QMap<QString, QVariant> value = location["value"].toMap();
      pos = value["pos"].toList();
    } else if (location.contains("pos")) {
      pos = location["pos"].toList();
    } else return;
    POI p(pos[0].toInt(), pos[2].toInt());
    p.color = color;
    this->poiList.append(p);
  }
}


bool Entity::intersects(const OverlayItem::Cuboid& cuboid) const {
  return cuboid.min.x <= pos.x && cuboid.max.x >= pos.x &&
         cuboid.min.y <= pos.y && cuboid.max.y >= pos.y &&
         cuboid.min.z <= pos.z && cuboid.max.z >= pos.z;
}

void Entity::draw(double offsetX, double offsetZ, double scale,
                  QPainter *canvas) const {
  QPoint center((pos.x - offsetX) * scale,
                (pos.z - offsetZ) * scale);

  foreach( POI p, poiList ) {
    // location of POI
    QPoint poiPos((p.x+0.5 - offsetX) * scale,
                  (p.z+0.5 - offsetZ) * scale);
    // line
    QColor penColor = p.color; // or use extraColor ?
    penColor.setAlpha(128);
    QPen pen = canvas->pen();
    pen.setColor(penColor);
    pen.setWidth(2);
    canvas->setPen(pen);
    canvas->drawLine(center, poiPos);
    // POI dot
    QColor poiColor = p.color;
    poiColor.setAlpha(192);
    pen.setColor(poiColor);
    pen.setWidth(2);
    canvas->setPen(pen);
    canvas->setBrush(poiColor);
    canvas->drawEllipse(poiPos, RADIUS/2, RADIUS/2);
  }

  QColor penColor = extraColor;
  penColor.setAlpha(192);
  QPen pen = canvas->pen();
  pen.setColor(penColor);
  pen.setWidth(2);
  canvas->setPen(pen);

  QColor brushColor = color();
  brushColor.setAlpha(128);
  canvas->setBrush(brushColor);
  canvas->drawEllipse(center, RADIUS, RADIUS);
}

Entity::Point Entity::midpoint() const {
  return pos;
}
