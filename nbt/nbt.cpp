/** Copyright (c) 2013, Sean Kasun */

#include <zlib.h>
#include <QFile>

#include "./nbt.h"


// this handles decoding the gzipped level.dat
NBT::NBT(const QString level)
  : root(&NBT::Null)  // just in case we die
{
  QFile f(level);
  f.open(QIODevice::ReadOnly);
  QByteArray data = f.readAll();
  f.close();

  QByteArray nbt;
  z_stream strm;
  static const int CHUNK_SIZE = 8192;
  char out[CHUNK_SIZE];
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = data.size();
  strm.next_in = reinterpret_cast<Bytef *>(data.data());

  inflateInit2(&strm, 15 + 32);
  do {
    strm.avail_out = CHUNK_SIZE;
    strm.next_out = reinterpret_cast<Bytef *>(out);
    inflate(&strm, Z_NO_FLUSH);
    nbt.append(out, CHUNK_SIZE - strm.avail_out);
  } while (strm.avail_out == 0);
  inflateEnd(&strm);

  TagDataStream s(nbt.constData(), nbt.size());

  if (s.r8() == 10) {  // compound
    s.skip(s.r16());  // skip name
    root = new Tag_Compound(&s);
  }
}

// this handles decoding a compressed() section of a region file
NBT::NBT(const uchar *chunk)
  : root(&NBT::Null)  // just in case we die
{
  // find chunk size
  int length = (chunk[0] << 24) | (chunk[1] << 16) | (chunk[2] << 8) |
      chunk[3];
  if (chunk[4] != 2)  // rfc1950
    return;

  z_stream strm;
  static const int CHUNK_SIZE = 8192;
  char out[CHUNK_SIZE];
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = length - 1;
  strm.next_in = (Bytef *)chunk + 5;

  QByteArray nbt;

  inflateInit(&strm);
  do {
    strm.avail_out = CHUNK_SIZE;
    strm.next_out = reinterpret_cast<Bytef *>(out);
    inflate(&strm, Z_NO_FLUSH);
    nbt.append(out, CHUNK_SIZE - strm.avail_out);
  } while (strm.avail_out == 0);
  inflateEnd(&strm);

  TagDataStream s(nbt.constData(), nbt.size());

  if (s.r8() == 10) {  // compound
    s.skip(s.r16());  // skip name
    root = new Tag_Compound(&s);
  }
}

Tag NBT::Null;

bool NBT::has(const QString key) const {
  return root->has(key);
}

const Tag *NBT::at(const QString key) const {
  return root->at(key);
}

NBT::~NBT() {
  if (root != &NBT::Null)
    delete root;
}
