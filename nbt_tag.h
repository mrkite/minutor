#ifndef TAG_H
#define TAG_H

#include <vector>
#include <QString>
#include <QVariant>

#include "nbt_tagdatastream.h"


class Tag {
 public:
  Tag();
  virtual ~Tag();

  virtual bool                        has(const QString key) const;
  virtual int                         length() const;
  virtual const Tag *                 at(const QString key) const;
  virtual const Tag *                 at(int index) const;
  virtual const QString               toString() const;
  virtual qint32                      toInt() const;
  virtual double                      toDouble() const;
  virtual const std::vector<quint8> & toByteArray() const;
  virtual const std::vector<qint32> & toIntArray() const;
  virtual const std::vector<qint64> & toLongArray() const;
  virtual const QVariant              getData() const;

  enum TagType {
    TAG_END        = 0,
    TAG_BYTE       = 1,
    TAG_SHORT      = 2,
    TAG_INT        = 3,
    TAG_LONG       = 4,
    TAG_FLOAT      = 5,
    TAG_DOUBLE     = 6,
    TAG_BYTE_ARRAY = 7,
    TAG_STRING     = 8,
    TAG_LIST       = 9,
    TAG_COMPOUND   = 10,
    TAG_INT_ARRAY  = 11,
    TAG_LONG_ARRAY = 12
  };
};


// all sub variants of Tag_*

class Tag_Byte : public Tag {
 public:
  explicit Tag_Byte(TagDataStream *s);

  signed   int   toInt() const override;
  unsigned int   toUInt() const;
  const QString  toString() const override;
  const QVariant getData() const override;
 private:
  quint8 data;
};

class Tag_Short : public Tag {
 public:
  explicit Tag_Short(TagDataStream *s);

  signed   int   toInt() const override;
  unsigned int   toUInt() const;
  const QString  toString() const override;
  const QVariant getData() const override;
 private:
  qint16 data;
};

class Tag_Int : public Tag {
 public:
  explicit Tag_Int(TagDataStream *s);

  qint32         toInt() const override;
  unsigned int   toUInt() const;
  double         toDouble() const override;
  const QString  toString() const override;
  const QVariant getData() const override;
 private:
  qint32 data;
};

class Tag_Long : public Tag {
 public:
  explicit Tag_Long(TagDataStream *s);

  qint32         toInt() const override;
  double         toDouble() const override;
  const QString  toString() const override;
  const QVariant getData() const override;
 private:
  qint64 data;
};

class Tag_Float : public Tag {
 public:
  explicit Tag_Float(TagDataStream *s);

  double         toDouble() const override;
  const QString  toString() const override;
  const QVariant getData() const override;

 private:
  float data;
};

class Tag_Double : public Tag {
 public:
  explicit Tag_Double(TagDataStream *s);

  double         toDouble() const override;
  const QString  toString() const override;
  const QVariant getData() const override;
 private:
  double data;
};

class Tag_Byte_Array : public Tag {
 public:
  explicit Tag_Byte_Array(TagDataStream *s);

  int                        length() const override;
  const std::vector<quint8>& toByteArray() const override;
  const QString              toString() const override;
  const QVariant             getData() const override;
 private:
  std::vector<quint8> data;
  int len;
};

class Tag_String : public Tag {
 public:
  explicit Tag_String(TagDataStream *s);

  const QString  toString() const override;
  const QVariant getData() const override;
 private:
  QString data;
};

class Tag_List : public Tag {
 public:
  explicit Tag_List(TagDataStream *s);
  ~Tag_List();

  const Tag *    at(int index) const override;
  int            length() const override;
  const QString  toString() const override;
  const QVariant getData() const override;
 private:
  QList<Tag *> data;
};

class Tag_Compound : public Tag {
 public:
  explicit Tag_Compound(TagDataStream *s);
  ~Tag_Compound();

  bool           has(const QString key) const override;
  const Tag *    at(const QString key) const override;
  const QString  toString() const override;
  const QVariant getData() const override;
 private:
  QHash<QString, Tag *> children;
};

class Tag_Int_Array : public Tag {
 public:
  explicit Tag_Int_Array(TagDataStream *s);

  int                        length() const override;
  const std::vector<qint32>& toIntArray() const override;
  const QString              toString() const override;
  const QVariant             getData() const override;
 private:
  int len;
  std::vector<qint32> data;
};

class Tag_Long_Array : public Tag {
 public:
  explicit Tag_Long_Array(TagDataStream *s);

  int                        length() const override;
  const std::vector<qint64>& toLongArray() const override;
  const QString              toString() const override;
  const QVariant             getData() const override;
 private:
  int len;
  std::vector<qint64> data;
};

#endif // TAG_H
