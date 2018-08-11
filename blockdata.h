/** Copyright (c) 2018, nnooney, EtlamGit */
#ifndef BLOCKDATA_H_
#define BLOCKDATA_H_

#include <QString>
#include <QMap>

class BlockData {
 public:
  uint    hid;   // we use hashed name as ID
  QString name;
  QMap<QString, QVariant> properties;
};

#endif  // BLOCKDATA_H_
