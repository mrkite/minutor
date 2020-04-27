/** Copyright (c) 2013, Sean Kasun */
#ifndef NBT_H_
#define NBT_H_

class QString;
class QByteArray;

#include <QHash>
#include <QString>
#include <QVariant>

class TagDataStream {
 public:
  TagDataStream(const char *data, int len);
  quint8 r8();
  quint16 r16();
  quint32 r32();
  quint64 r64();
  void r(int len, std::vector<quint8> &data_out);
  QString utf8(int len);
  void skip(int len);
 private:
  const quint8 *data;
  int pos, len;
};

class Tag {
 public:
  Tag();
  virtual ~Tag();
  virtual bool has(const QString key) const;
  virtual int length() const;
  virtual const Tag *at(const QString key) const;
  virtual const Tag *at(int index) const;
  virtual const QString toString() const;
  virtual qint32 toInt() const;
  virtual double toDouble() const;
  virtual const std::vector<quint8> &toByteArray() const;
  virtual const std::vector<qint32> &toIntArray() const;
  virtual const std::vector<qint64> &toLongArray() const;
  virtual const QVariant getData() const;
};

class NBT {
 public:
  explicit NBT(const QString level);
  explicit NBT(const uchar *chunk);
  ~NBT();

  bool has(const QString key) const;
  const Tag *at(const QString key) const;

  static Tag Null;
 private:
  Tag *root;
};

class Tag_Byte : public Tag {
 public:
  explicit Tag_Byte(TagDataStream *s);
  signed   int toInt() const;
  unsigned int toUInt() const;
  virtual const QString toString() const;
  virtual const QVariant getData() const;
 private:
  quint8 data;
};

class Tag_Short : public Tag {
 public:
  explicit Tag_Short(TagDataStream *s);
  signed   int toInt() const;
  unsigned int toUInt() const;
  virtual const QString toString() const;
  virtual const QVariant getData() const;
 private:
  qint16 data;
};

class Tag_Int : public Tag {
 public:
  explicit Tag_Int(TagDataStream *s);
  qint32 toInt() const;
  double toDouble() const;
  virtual const QString toString() const;
  virtual const QVariant getData() const;
 private:
  qint32 data;
};

class Tag_Long : public Tag {
 public:
  explicit Tag_Long(TagDataStream *s);
  qint32 toInt() const;
  double toDouble() const;
  virtual const QString toString() const;
  virtual const QVariant getData() const;
 private:
  qint64 data;
};

class Tag_Float : public Tag {
 public:
  explicit Tag_Float(TagDataStream *s);

  virtual double toDouble() const;
  virtual const QString toString() const;
  virtual const QVariant getData() const;

 private:
  float data;
};

class Tag_Double : public Tag {
 public:
  explicit Tag_Double(TagDataStream *s);
  double toDouble() const;
  virtual const QString toString() const;
  virtual const QVariant getData() const;
 private:
  double data;
};

class Tag_Byte_Array : public Tag {
 public:
  explicit Tag_Byte_Array(TagDataStream *s);
  ~Tag_Byte_Array();
  int length() const;
  const std::vector<quint8>& toByteArray() const override;
  virtual const QString toString() const;
  virtual const QVariant getData() const;
 private:
  std::vector<quint8> data;
  int len;
};

class Tag_String : public Tag {
 public:
  explicit Tag_String(TagDataStream *s);
  const QString toString() const;
  virtual const QVariant getData() const;
 private:
  QString data;
};

class Tag_List : public Tag {
 public:
  explicit Tag_List(TagDataStream *s);
  ~Tag_List();
  const Tag *at(int index) const;
  int length() const;
  virtual const QString toString() const;
  virtual const QVariant getData() const;
 private:
  QList<Tag *> data;
};

class Tag_Compound : public Tag {
 public:
  explicit Tag_Compound(TagDataStream *s);
  ~Tag_Compound();
  bool has(const QString key) const;
  const Tag *at(const QString key) const;
  virtual const QString toString() const;
  virtual const QVariant getData() const;
 private:
  QHash<QString, Tag *> children;
};

class Tag_Int_Array : public Tag {
 public:
  explicit Tag_Int_Array(TagDataStream *s);
  ~Tag_Int_Array();
  int length() const;
  const std::vector<qint32>& toIntArray() const override;
  virtual const QString toString() const;
  virtual const QVariant getData() const;
 private:
  int len;
  std::vector<qint32> data;
};

class Tag_Long_Array : public Tag {
 public:
  explicit Tag_Long_Array(TagDataStream *s);
  ~Tag_Long_Array();
  int length() const;
  const std::vector<qint64>& toLongArray() const override;
  virtual const QString toString() const;
  virtual const QVariant getData() const;
 private:
  int len;
  std::vector<qint64> data;
};

#endif  // NBT_H_
