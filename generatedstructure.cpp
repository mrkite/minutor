/** Copyright 2014 Rian Shelley */
#include <QPainter>
#include "generatedstructure.h"
#include "nbt/nbt.h"


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

// parse structures stored in Chunks
QList<QSharedPointer<GeneratedStructure>>
GeneratedStructure::tryParseChunk(const Tag* structuresTag) {
  // we will return a list of all found structures
  QList<QSharedPointer<GeneratedStructure>> ret;

  // parse NBT data for "Starts" & "starts"
  if (structuresTag && structuresTag != &NBT::Null) {
    QString starts("Starts");
    if (!structuresTag->has(starts))
      starts = "starts";
    auto features = structuresTag->at(starts);
    if (features && features != &NBT::Null) {
      // convert the features to a qvariant here
      QVariant maybeFeatureMap = features->getData();
      ret.append( GeneratedStructure::tryParseFeatures(maybeFeatureMap) );
    }
  }
  return ret;
}

// static
QList<QSharedPointer<GeneratedStructure>>
GeneratedStructure::tryParseBlockEntites(const Tag* tagBlockEntites) {
  // we will return a list of all found block entities
  QList<QSharedPointer<GeneratedStructure>> ret;

  if (tagBlockEntites && tagBlockEntites != &NBT::Null) {

    // loop over all block entities in Chunk (typically chests and spawners)
    for (int idx = 0; idx < tagBlockEntites->length(); idx++) {
      const Tag * be = tagBlockEntites->at(idx);

      if (be->has("id") &&
          ( (be->at("id")->toString() == "minecraft:mob_spawner") ||
            (be->at("id")->toString() == "MobSpawner") ) ) {
        // mob spawner found
        // -> parse the spawner data to a GeneratedStructure pointer
        ret.append( GeneratedStructure::tryParseSpawner(be) );
      }

/*    if (be->has("id") &&
          ( (be->at("id")->toString() == "minecraft:shulker_box") ||
            (be->at("id")->toString() == "minecraft:chest") ||
            (be->at("id")->toString() == "Chest") ) ) {
        // chest found
        // -> parse the chest data to a GeneratedStructure pointer
        ret.append( GeneratedStructure::tryParseChest(be) );
      }
*/
    }
  }

  return ret;
}



// static
QSharedPointer<GeneratedStructure>
GeneratedStructure::tryParseChest(const Tag* tagChest) {
  return QSharedPointer<GeneratedStructure>();
}

// static
QSharedPointer<GeneratedStructure>
GeneratedStructure::tryParseSpawner(const Tag* tagSpawner) {
  // we will return a list of all found structures
  QSharedPointer<GeneratedStructure> spawner = QSharedPointer<GeneratedStructure>(new GeneratedStructure());

  QString mobType = "minecraft:unknown_mob";
  // before "The Flattening"
  if (tagSpawner->has("EntityId")) {
    mobType = tagSpawner->at("EntityId")->toString();
    // reformat CamelCase text to flattening
    // split at uppercase characters
    QStringList tokens = mobType.split(QRegExp("(?<=[a-z])(?=[A-Z])"), QString::SkipEmptyParts);
    mobType = tokens.join("_").toLower();
  }
  // after "The Flattening"
  if (tagSpawner->has("SpawnData") && tagSpawner->at("SpawnData")->has("id")) {
    mobType = tagSpawner->at("SpawnData")->at("id")->toString();
  }
  // after "Caves & Cliffs"
  if (tagSpawner->has("SpawnData") && tagSpawner->at("SpawnData")->has("entity") &&
      tagSpawner->at("SpawnData")->at("entity")->has("id")) {
    mobType = tagSpawner->at("SpawnData")->at("entity")->at("id")->toString();
  }
  mobType.replace("minecraft:", "");

  spawner->setType("Structure.minecraft:spawner");
  spawner->setDisplay("minecraft:spawner:"+mobType);
  spawner->setDimension("overworld");

  // base the color on a hash of its type
  int    hue = qHash(QString("minecraft:spawner"), 0) % 360;
  QColor color;
  color.setHsv(hue, 255, 255, 64);
  spawner->setColor(color);

  // fill properties
  const Tag_Compound * tc = static_cast<const Tag_Compound *>(tagSpawner);
  QMap<QString, QVariant> spawnerProperties = tc->getData().toMap();
  spawner->setProperties(spawnerProperties);

  // get covered area
  int x = tagSpawner->at("x")->toInt();
  int y = tagSpawner->at("y")->toInt();
  int z = tagSpawner->at("z")->toInt();
  int r = 4;
  if (tagSpawner->has("SpawnRange"))
    r = tagSpawner->at("SpawnRange")->toInt();
  spawner->setBounds( Point(x-r, y, z-r), Point(x+r, y, z+r));

  return spawner;
}

// static
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
        if (featureProperties.contains("id") && featureProperties["id"].toString() != "INVALID") {
          // parse id
          QString id = featureProperties["id"].toString();

          GeneratedStructure* structure = new GeneratedStructure();
          structure->setType("Structure." + id);
          structure->setDisplay(id);
          structure->setProperties(featureProperties);

          // base the color on a hash of its type
          int    hue = qHash(id, 0) % 360;
          QColor color;
          color.setHsv(hue, 255, 255, 64);
          structure->setColor(color);

          // this will have to be maintained if new structures are added
          // that are appearing only in some Dimensions
          if (structure->type() == "Structure.Fortress") {
            structure->setDimension("nether");
          } else if (structure->type() == "Structure.EndCity") {
            structure->setDimension("end");
          } else {
            structure->setDimension("overworld");
          }

          // get covered area
          int minX = INT_MAX;
          int minY = INT_MAX;
          int minZ = INT_MAX;
          int maxX = INT_MIN;
          int maxY = INT_MIN;
          int maxZ = INT_MIN;
          // covered area is given directly in feature (<1.17)
          if (featureProperties.contains("BB") &&
              (QMetaType::Type)featureProperties["BB"].type() == QMetaType::QVariantList ) {
            // parse Bounding Box
            QList<QVariant> bb = featureProperties["BB"].toList();
            if (bb.size() == 6) {
              minX = bb[0].toInt();
              minY = bb[1].toInt();
              minZ = bb[2].toInt();
              maxX = bb[3].toInt();
              maxY = bb[4].toInt();
              maxZ = bb[5].toInt();
            }
          } else {
            // covered area is given in Children only (starting with 1.17)
            if (featureProperties.contains("Children")) {
              // loop over all elements in Children
              auto children = featureProperties["Children"];
              if ((QMetaType::Type)children.type() == QMetaType::QVariantList)
                for (auto &child : children.toList()) {
                  if ( (QMetaType::Type)child.type() == QMetaType::QVariantMap ) {
                    QMap<QString, QVariant> childMap = child.toMap();
                    if (childMap.contains("BB") &&
                        ( QMetaType::Type)childMap["BB"].type() == QMetaType::QVariantList ) {
                      QList<QVariant> bb = childMap["BB"].toList();
                      if (bb.size() == 6) {
                        minX = std::min<int>(minX, bb[0].toInt());
                        minY = std::min<int>(minY, bb[1].toInt());
                        minZ = std::min<int>(minZ, bb[2].toInt());
                        maxX = std::max<int>(maxX, bb[3].toInt());
                        maxY = std::max<int>(maxY, bb[4].toInt());
                        maxZ = std::max<int>(maxZ, bb[5].toInt());
                      }
                    }
                  }
                }
            }
          }

          structure->setBounds( Point(minX, minY, minZ), Point(maxX, maxY, maxZ));
          ret.append( QSharedPointer<GeneratedStructure>(structure) );
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
