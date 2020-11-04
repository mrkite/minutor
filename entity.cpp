/** Copyright 2014 EtlamGit */
#include <QPainter>
#include "./entity.h"
#include "./entityidentifier.h"
#include "./nbt.h"

Entity::Entity(const Point &positionInfo)
    : extraColor(QColor::fromRgb(0,255,0))
    , pos(positionInfo)
    , hasExtraR(false)
    , hasExtraB(false)
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
          if (memories.contains("minecraft:home")) {
            auto home  = memories["minecraft:home"].toMap();
            auto value = home["value"].toMap();
            auto pos   = value["pos"].toList();
            entity->posB = Point(pos[0].toDouble(), pos[1].toDouble(), pos[2].toDouble());
            entity->hasExtraB = true;
          }
          // location of job site
          if (memories.contains("minecraft:job_site")) {
            auto job   = memories["minecraft:job_site"].toMap();
            auto value = job["value"].toMap();
            auto pos   = value["pos"].toList();
            entity->posR = Point(pos[0].toDouble(), pos[1].toDouble(), pos[2].toDouble());
            entity->hasExtraR = true;
          }
          // location of potential job site
          if (memories.contains("minecraft:potential_job_site")) {
            auto job   = memories["minecraft:potential_job_site"].toMap();
            auto value = job["value"].toMap();
            auto pos   = value["pos"].toList();
            entity->posR = Point(pos[0].toDouble(), pos[1].toDouble(), pos[2].toDouble());
            entity->hasExtraR = true;
          }
        }
      }

      ret.reset(entity);
    }
  }
  return ret;
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

  if (hasExtraB) {
    QPoint extraPos((posB.x+0.5 - offsetX) * scale,
                    (posB.z+0.5 - offsetZ) * scale);

    QColor extraColor = QColor(0,0,255);
    extraColor.setAlpha(128);
    QPen pen = canvas->pen();
    pen.setColor(extraColor);
    pen.setWidth(2);
    canvas->setPen(pen);
    extraColor.setAlpha(192);
    canvas->setBrush(extraColor);

    canvas->drawLine(center, extraPos);
    canvas->drawEllipse(extraPos, RADIUS/2, RADIUS/2);
  }

  if (hasExtraR) {
    QPoint extraPos((posR.x+0.5 - offsetX) * scale,
                    (posR.z+0.5 - offsetZ) * scale);

    QColor extraColor = QColor(255,0,0);
    extraColor.setAlpha(128);
    QPen pen = canvas->pen();
    pen.setColor(extraColor);
    pen.setWidth(2);
    canvas->setPen(pen);
    extraColor.setAlpha(192);
    canvas->setBrush(extraColor);

    canvas->drawLine(center, extraPos);
    canvas->drawEllipse(extraPos, RADIUS/2, RADIUS/2);

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
