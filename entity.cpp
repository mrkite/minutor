#include "entity.h"

#include "nbt.h"

#include <QPainter>

QSharedPointer<OverlayItem> Entity::TryParse(Tag* tag)
{
	QSharedPointer<OverlayItem> ret;
	Tag* pos = tag->at("Pos");
	if (pos && pos != &NBT::Null)
	{
		Entity* entity = new Entity();
		entity->pos.x = pos->at(0)->toDouble();
		entity->pos.y = pos->at(1)->toDouble();
		entity->pos.z = pos->at(2)->toDouble();
		Tag* id = tag->at("id");
		if (id && id != &NBT::Null)
		{
			QString type = id->toString();

			QMap<QString, QVariant> props = tag->getData().toMap();
			QColor color;
			//get something more descriptive if its an item
			if (type == "Item")
			{

				Tag* itemId = tag->at("Item")->at("id");

				QString itemtype = itemId->toString();
				entity->setDisplay(itemtype.mid(itemtype.indexOf(':') + 1));

				entity->setType("Entity.Passive");
				color = Qt::white;
			}
			else
			{
				//attempt to automatically categorize this as hostile, neutral, or passive
				if (props.contains("InLove")  ||  //if its breedable, then its not an enemy?
					props.contains("Willing") ||
					props["id"].toString().contains("Golem") ||
					props["id"] == "SnowMan" ||
					props["id"] == "Squid" ||
					props["id"] == "Bat"
				)
				{
					entity->setType("Entity.Neutral");
					color = Qt::blue;
				}
				else if (props.size() < 20)
				{
					//simple objects are usually items
					entity->setType("Entity.Passive");
					color = Qt::white;
				}
				else
				{
					//otherwise, it will probably hurt you if you go near it
					entity->setType("Entity.Hostile");
					color = Qt::red;
				}
				entity->setDisplay(type);
			}
			color.setAlpha(128);
			entity->setColor(color);
			entity->setProperties(props);
			ret.reset(entity);
		}
	}
	return ret;
}


bool Entity::intersects(const Point& min, const Point& max) const
{
	return min.x <= pos.x && max.x >= pos.x &&
		   min.y <= pos.y && max.y >= pos.y &&
		   min.z <= pos.z && max.z >= pos.z;
}

void Entity::draw(double offsetX, double offsetZ, double scale, QPainter& canvas) const
{
	int left = (int)((pos.x - offsetX) * scale) - RADIUS / 2;
	int top = (int)((pos.z - offsetZ) * scale) - RADIUS / 2;

	canvas.setPen(Qt::transparent);
	canvas.setBrush(QBrush(color()));
	canvas.drawEllipse(left, top, RADIUS, RADIUS);
}

Entity::Point Entity::midpoint() const
{
	return pos;
}
