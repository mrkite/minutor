#ifndef CHUNKMATH_HPP
#define CHUNKMATH_HPP

#include <QVector2D>
#include <QVector3D>

enum
{
    CHUNK_SIDE_LENGTH = 16
};


inline QVector2D getChunkCoordinates(const QVector2D& worldCoordinates)
{
    QVector2D result;
    result.setX(worldCoordinates.x() / CHUNK_SIDE_LENGTH);
    result.setY(worldCoordinates.y() / CHUNK_SIDE_LENGTH);
    return result;
}

inline QVector2D getChunkCoordinates(const QVector3D& worldCoordinates)
{
    QVector2D result;
    result.setX(worldCoordinates.x() / CHUNK_SIDE_LENGTH);
    result.setY(worldCoordinates.z() / CHUNK_SIDE_LENGTH);
    return result;
}

#endif // CHUNKMATH_HPP
