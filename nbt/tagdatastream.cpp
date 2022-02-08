/** Copyright (c) 2013, Sean Kasun */

#include "nbt/tagdatastream.h"


TagDataStream::TagDataStream(const char *data, int len) {
  this->data = (const quint8 *)data;
  this->len = len;
  pos = 0;
}

quint8 TagDataStream::r8() {
  if (pos+1 > this->len) return 0;  // safety check to prevent reading beyond the end
  return data[pos++];
}

quint16 TagDataStream::r16() {
  if (pos+2 > this->len) return 0;  // safety check to prevent reading beyond the end
  quint16 r = data[pos++] << 8;
  r |= data[pos++];
  return r;
}

quint32 TagDataStream::r32() {
  if (pos+4 > this->len) return 0;  // safety check to prevent reading beyond the end
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
  if (pos+len > this->len) return;  // safety check to prevent reading beyond the end
  // you need to free anything read with this
  data_out.resize(len);
  memcpy(&data_out[0], data + pos, len);
  pos += len;
}

QString TagDataStream::utf8(int len) {
  if (pos+len > this->len) return QString();  // safety check to prevent reading beyond the end
  int old = pos;
  pos += len;
  return QString::fromUtf8((const char *)data + old, len);
}

void TagDataStream::skip(int len) {
  pos += len;
}
