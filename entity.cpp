#include "entity.h"

#include "nbt.h"

Entity::Entity(qint64 nx, qint64 ny, qint64 nz, QString nid):
	id(nid),
	x(nx),
	y(ny),
	z(nz)
{
}


void Entity::load(Tag *nbt)
{
	Tag* pos = nbt->at("Pos");
	x = (qint64)pos->at(0)->toDouble() - .5;
	y = (qint64)pos->at(1)->toDouble();
	z = (qint64)pos->at(2)->toDouble();
	Tag* id = nbt->at("id");
	if (id)
	{
		this->id = id->toString();
		properties = nbt->getData();
	}
}
