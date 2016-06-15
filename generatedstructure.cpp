/** Copyright 2014 Rian Shelley */
#include <QPainter>
#include "./generatedstructure.h"
#include "./nbt.h"

QList<QSharedPointer<GeneratedStructure>>
GeneratedStructure::tryParse(const Tag* data) {
  QList<QSharedPointer<GeneratedStructure>> ret;
  if (data && data != &NBT::Null) {
    auto features = data->at("Features");
    if (features && features != &NBT::Null) {
      // convert the features to a qvariant here
      QVariant maybeFeatureMap = features->getData();
      if ((QMetaType::Type)maybeFeatureMap.type() == QMetaType::QVariantMap) {
        QMap<QString, QVariant> featureMap = maybeFeatureMap.toMap();
        for (auto &feature : featureMap) {
          if ((QMetaType::Type)feature.type() == QMetaType::QVariantMap) {
            QMap<QString, QVariant> featureProperties = feature.toMap();
            // check for required properties
            if (featureProperties.contains("BB") &&
                (QMetaType::Type)featureProperties["BB"].type() ==
                QMetaType::QVariantList && featureProperties.contains("id")) {
              QList<QVariant> bb = featureProperties["BB"].toList();
              if (bb.size() == 6) {
                GeneratedStructure* structure = new GeneratedStructure();
                structure->setBounds(
                    Point(bb[0].toInt(), bb[1].toInt(), bb[2].toInt()),
                    Point(bb[3].toInt(), bb[4].toInt(), bb[5].toInt()));
                structure->setType("Structure." +
                                   featureProperties["id"].toString());
                structure->setDisplay(featureProperties["id"].toString());
                structure->setProperties(featureProperties);

                // base the color on a hash of its type
                quint32 hue = qHash(featureProperties["id"].toString());
                QColor color;
                color.setHsv(hue % 360, 255, 255, 64);
                structure->setColor(color);

                // this will have to be maintained if new structures are added
                if (structure->type() == "Structure.Fortress") {
                  structure->setDimension("nether");
                } else if (structure->type() == "Structure.EndCity") {
                  structure->setDimension("end");
                } else {
                  structure->setDimension("overworld");
                }
                ret.append(QSharedPointer<GeneratedStructure>(structure));
              }
            }
          }
        }
      }
    }
  }
  return ret;
}

bool GeneratedStructure::intersects(const Point& min,
                                    const Point& max) const {
  return min.x <= p2.x && p1.x <= max.x && min.y <= p2.y &&
      p1.y <= max.y && min.z <= p2.z && p1.z <= max.z;
}

void GeneratedStructure::draw(double offsetX, double offsetZ, double scale,
                              QPainter *canvas) const {
  int left = static_cast<int>((p1.x - offsetX) * scale);
  int top = static_cast<int>((p1.z - offsetZ) * scale);
  int w = static_cast<int>((p2.x - p1.x + 1) * scale);
  int h = static_cast<int>((p2.z - p1.z + 1) * scale);

  canvas->setPen(Qt::transparent);
  canvas->setBrush(QBrush(color()));
  canvas->drawRoundedRect(left, top, w, h, RADIUS, RADIUS);
  QSize labelsize = canvas->fontMetrics().size(0, display());
  canvas->setPen(Qt::black);
  if (labelsize.height() < h && labelsize.width() < w) {
    canvas->drawText(left, top, w, h, Qt::AlignCenter, display());
  }
}

GeneratedStructure::Point GeneratedStructure::midpoint() const {
  return Point((p1.x + p2.x) / 2, (p1.y + p2.y) / 2, (p1.z + p2.z) / 2);
}
