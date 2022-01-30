/** Copyright (c) 2013, Sean Kasun */
#ifndef ZIPREADER_H_
#define ZIPREADER_H_

#include <QHash>
#include <QFile>
#include <QString>
#include <QByteArray>

struct ZipFileHeader {
  qint32 compressed, uncompressed;
  qint16 compression;
  qint64 offset;
};

class ZipReader {
 public:
  explicit    ZipReader(const QString filename);
  bool        open();
  void        close();
  QByteArray  get(const QString filename);
  QStringList getFileList() const;
 private:
  QFile f;
  QHash<QString, struct ZipFileHeader> files;
};

#endif  // ZIPREADER_H_
