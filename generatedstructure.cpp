/** Copyright 2014 Rian Shelley */
#include <QPainter>
#include "./generatedstructure.h"
#include "./nbt.h"


// parse structures in *.dat files
QList<QSharedPointer<GeneratedStructure>>
GeneratedStructure::tryParseDatFile(const Tag* data) {
  // we will return a list of all found structures
  QList<QSharedPointer<GeneratedStructure>> ret;

  // parse NBT data for "Features"
  if (data && data != &NBT::Null) {
    auto features = data->at("Features");
    if (features && features != &NBT::Null) {
      // convert the features to a qvariant here
      QVariant maybeFeatureMap = features->getData();
      ret.append( GeneratedStructure::tryParseFeatures(maybeFeatureMap) );
    }
  }
  return ret;
}

// parse structures in Chunks
QList<QSharedPointer<GeneratedStructure>>
GeneratedStructure::tryParseChunk(const Tag* data) {
  // we will return a list of all found structures
  QList<QSharedPointer<GeneratedStructure>> ret;

  // parse NBT data for "Starts"
  if (data && data != &NBT::Null) {
    auto features = data->at("Starts");
    if (features && features != &NBT::Null) {
      // convert the features to a qvariant here
      QVariant maybeFeatureMap = features->getData();
      ret.append( GeneratedStructure::tryParseFeatures(maybeFeatureMap) );
    }
  }
  return ret;
}

QList<QSharedPointer<GeneratedStructure>>
GeneratedStructure::tryParseFeatures(QVariant &maybeFeatureMap) {
  // we will return a list of all found structures
  QList<QSharedPointer<GeneratedStructure>> ret;

  // check if we got a map
  if ((QMetaType::Type)maybeFeatureMap.type() == QMetaType::QVariantMap) {
    // convert it to a real map
    QMap<QString, QVariant> featureMap = maybeFeatureMap.toMap();

    // loop over all elements in feature map
    for (auto &feature : featureMap) {
      // check if the element is also a map
      if ((QMetaType::Type)feature.type() == QMetaType::QVariantMap) {
        // convert it to a real map
        QMap<QString, QVariant> featureProperties = feature.toMap();
        // check for required properties
        if (featureProperties.contains("id") &&
            featureProperties.contains("BB") &&
            (QMetaType::Type)featureProperties["BB"].type() == QMetaType::QVariantList ) {
          // parse id
          QString id = featureProperties["id"].toString();
          // parse Bounding Box
          QList<QVariant> bb = featureProperties["BB"].toList();
          if (bb.size() == 6) {
            GeneratedStructure* structure = new GeneratedStructure();
            structure->setBounds(
              Point(bb[0].toInt(), bb[1].toInt(), bb[2].toInt()),
              Point(bb[3].toInt(), bb[4].toInt(), bb[5].toInt()));
            structure->setType("Structure." + id);
            structure->setDisplay(id);
            structure->setProperties(featureProperties);

            // base the color on a hash of its type
            int    hue = qHash(id) % 360;
            QColor color;
            color.setHsv(hue, 255, 255, 64);
            structure->setColor(color);

            // this will have to be maintained if new structures are added
            // that are appearing only a some Dimensions
            if (structure->type() == "Structure.Fortress") {
              structure->setDimension("nether");
            } else if (structure->type() == "Structure.EndCity") {
              structure->setDimension("end");
            } else {
              structure->setDimension("overworld");
            }
            ret.append( QSharedPointer<GeneratedStructure>(structure) );
          }
        }
      }
    }
  }
  return ret;
}

bool GeneratedStructure::intersects(const OverlayItem::Cuboid& cuboid) const {
  return cuboid.min.x <= p2.x && p1.x <= cuboid.max.x &&
         cuboid.min.y <= p2.y && p1.y <= cuboid.max.y &&
         cuboid.min.z <= p2.z && p1.z <= cuboid.max.z;
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
