/** Copyright (c) 2018, nnooney, EtlamGit */
#ifndef PALETTEENTRY_H_
#define PALETTEENTRY_H_

#include <QString>
#include <QVariant>
#include <QMap>

class PaletteEntry {
 public:
  uint    hid;   // we use hashed name as ID
  QString name;
  QMap<QString, QVariant> properties;
};

#endif  // PALETTEENTRY_H_
