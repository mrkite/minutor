/** Copyright (c) 2013, Sean Kasun */

#include "./tagdatastream.h"


TagDataStream::TagDataStream(const char *data, int len) {
  this->data = (const quint8 *)data;
  this->len = len;
  pos = 0;
}

quint8 TagDataStream::r8() {
  return data[pos++];
}

quint16 TagDataStream::r16() {
  quint16 r = data[pos++] << 8;
  r |= data[pos++];
  return r;
}

quint32 TagDataStream::r32() {
  quint32 r = data[pos++] << 24;
  r |= data[pos++] << 16;
  r |= data[pos++] << 8;
  r |= data[pos++];
  return r;
}

quint64 TagDataStream::r64() {
  quint64 r = (quint64)r32() << 32;
  r |= r32();
  return r;
}

void TagDataStream::r(int len, std::vector<quint8>& data_out) {
  // you need to free anything read with this
  data_out.resize(len);
  memcpy(&data_out[0], data + pos, len);
  pos += len;
}

QString TagDataStream::utf8(int len) {
  int old = pos;
  pos += len;
  return QString::fromUtf8((const char *)data + old, len);
}

void TagDataStream::skip(int len) {
  pos += len;
}
