/** Copyright 2014 Mc_Etlam */
#include <QPainter>
#include "./entity.h"
#include "./entityidentifier.h"
#include "./nbt.h"

QSharedPointer<OverlayItem> Entity::TryParse(const Tag* tag) {
  EntityIdentifier& ei = EntityIdentifier::Instance();

  QSharedPointer<OverlayItem> ret;
  auto pos = tag->at("Pos");
  if (pos && pos != &NBT::Null) {
    Entity* entity = new Entity();
    entity->pos.x = pos->at(0)->toDouble();
    entity->pos.y = pos->at(1)->toDouble();
    entity->pos.z = pos->at(2)->toDouble();
    auto id = tag->at("id");
    if (id && id != &NBT::Null) {
      QString type = id->toString();
      EntityInfo const & info = ei.getEntityInfo(type);

      QMap<QString, QVariant> props = tag->getData().toMap();

      // get something more descriptive if its an item
      if (type == "Item") {
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
      ret.reset(entity);
    }
  }
  return ret;
}


bool Entity::intersects(const Point& min, const Point& max) const {
  return min.x <= pos.x && max.x >= pos.x &&
      min.y <= pos.y && max.y >= pos.y &&
      min.z <= pos.z && max.z >= pos.z;
}

void Entity::draw(double offsetX, double offsetZ, double scale,
                  QPainter *canvas) const {
  QPoint center((pos.x - offsetX) * scale,
                (pos.z - offsetZ) * scale);

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
