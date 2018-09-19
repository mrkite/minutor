/** Copyright 2014 Rian Shelley */
#include "./village.h"
#include "./nbt.h"

// parse structures in *.dat files
QList<QSharedPointer<GeneratedStructure>>
Village::tryParseDatFile(const Tag* tag, const QString& dimension) {
  // we will return a list of all found structures
  QList<QSharedPointer<GeneratedStructure> > ret;

  if (tag && tag != &NBT::Null) {
    auto villages = tag->at("Villages");
    if (villages && villages != &NBT::Null) {
      for (int i = 0; i < villages->length(); ++i) {
        Village* newVillage = new Village();
        auto village = villages->at(i);
        int radius = village->at("Radius")->toInt();
        int cx = village->at("CX")->toInt();
        int cy = village->at("CY")->toInt();
        int cz = village->at("CZ")->toInt();
        newVillage->setBounds(
            Point(cx - radius, cy - radius, cz - radius),
            Point(cx + radius, cy + radius, cz + radius));
        newVillage->setDisplay("Village");
        newVillage->setType("Structure.Village");

        // color is based on the hash
        quint32 hue = qHash("Village");
        QColor color;
        color.setHsv(hue % 360, 255, 255, 64);
        newVillage->setColor(color);

        newVillage->setProperties(village->getData());
        newVillage->setDimension(dimension);
        ret.append(QSharedPointer<GeneratedStructure>(newVillage));
      }
    }
  }
  return ret;
}
