/** Copyright (c) 2018, nnooney, EtlamGit */
#ifndef BLOCKDATA_H_
#define BLOCKDATA_H_

#include <QString>
#include <QMap>

class BlockData {
 public:
  QString name;
  QMap<QString, QVariant> properties;
};

#endif  // BLOCKDATA_H_
