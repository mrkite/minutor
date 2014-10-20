#ifndef ENTITY_H
#define ENTITY_H

#include <QMap>
#include <QString>
#include <QVariant>
class Tag;

class Entity
{
public:
    Entity(qint64 nx = 0, qint64 ny = 0, qint64 nz = 0, QString nid = "");

    void load(Tag *t);

    const QString& getId() const { return id; }
    const QVariant& getProperties() const { return properties; }
    qint64 getX() const { return x; }
    qint64 getY() const { return y; }
    qint64 getZ() const { return z; }

private:
    QString id;
    QVariant properties;
    qint64 x;
    qint64 y;
    qint64 z;
};

#endif // ENTITY_H
