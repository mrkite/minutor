#ifndef TAGDATASTREAM_H
#define TAGDATASTREAM_H

#include <vector>
#include <QString>


class TagDataStream {
 public:
  TagDataStream(const char *data, int len);
  quint8  r8();                                       // read 8 bit
  quint16 r16();                                      // read 16 bit
  quint32 r32();                                      // read 32 bit
  quint64 r64();                                      // read 64 bit
  void    r(int len, std::vector<quint8> &data_out);  // read <len> bytes
  QString utf8(int len);                              // read UTF8 encoded string
  void    skip(int len);                              // skip <len> bytes of data
 private:
  const quint8 *data;
  int pos, len;
};

#endif // TAGDATASTREAM_H
