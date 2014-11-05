#ifndef ENTITY_H
#define ENTITY_H

#include "overlayitem.h"

#include <QSharedPointer>

class Tag;

class Entity: public OverlayItem
{
public:
	static QSharedPointer<OverlayItem> TryParse(Tag* tag);

	virtual bool intersects(const Point& min, const Point& max) const;
	virtual void draw(double offsetX, double offsetZ, double scale, QPainter& canvas) const;
	virtual Point midpoint() const;

	static const int RADIUS = 10;

protected:
	Entity() {}

private:


	Point pos;
	QString display;
};

#endif // ENTITY_H
