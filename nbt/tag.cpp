/** Copyright (c) 2013, Sean Kasun */
#include <QByteArray>
#include <QDebug>
#include <QStringList>

#include "nbt/tag.h"
#include "nbt/nbt.h"


Tag::Tag() {
}

Tag::~Tag() {
}

int Tag::length() const {
  qWarning() << "Tag::length unhandled in base class";
  return 0;
}

bool Tag::has(const QString) const {
  return false;
}

const Tag *Tag::at(const QString) const {
  return &NBT::Null;
}

const Tag *Tag::at(int /* idx */) const {
  return &NBT::Null;
}

const QString Tag::toString() const {
  qWarning() << "Tag::toString unhandled in base class";
  return "";
}

qint32 Tag::toInt() const {
  qWarning() << "Tag::toInt unhandled in base class";
  return 0;
}

double Tag::toDouble() const {
  qWarning() << "Tag::toDouble unhandled in base class";
  return 0.0;
}

const std::vector<quint8>& Tag::toByteArray() const {
  qWarning() << "Tag:: toByteArray unhandled in base class";
  static const std::vector<quint8> dummy;
  return dummy;
}

const std::vector<qint32>& Tag::toIntArray() const {
  qWarning() << "Tag::toIntArray unhandled in base class";
  static const std::vector<qint32> dummy;
  return dummy;
}

const std::vector<qint64>& Tag::toLongArray() const {
  qWarning() << "Tag::toLongArray unhandled in base class";
  static const std::vector<qint64> dummy;
  return dummy;
}

const QVariant Tag::getData() const {
  qWarning() << "tag::getData unhandled in base class";
  return QVariant();
}


// Tag_Byte

Tag_Byte::Tag_Byte(TagDataStream *s) {
  data = s->r8();
}

int Tag_Byte::toInt() const {
  return (signed char)data;
}

unsigned int Tag_Byte::toUInt() const {
  return (unsigned char)data;
}

const QString Tag_Byte::toString() const {
  return QString::number(data);
}

const QVariant Tag_Byte::getData() const {
  return data;
}


// Tag_Short

Tag_Short::Tag_Short(TagDataStream *s) {
  data = s->r16();
}

int Tag_Short::toInt() const {
  return (signed short)data;
}

unsigned int Tag_Short::toUInt() const {
  return (unsigned short)data;
}

const QString Tag_Short::toString() const {
  return QString::number(data);
}

const QVariant Tag_Short::getData() const {
  return data;
}


// Tag_Int

Tag_Int::Tag_Int(TagDataStream *s) {
  data = s->r32();
}

qint32 Tag_Int::toInt() const {
  return data;
}

unsigned int Tag_Int::toUInt() const {
  return (unsigned int)data;
}

const QString Tag_Int::toString() const {
  return QString::number(data);
}

const QVariant Tag_Int::getData() const {
  return data;
}

double Tag_Int::toDouble() const {
  return static_cast<double>(data);
}


// Tag_Long

Tag_Long::Tag_Long(TagDataStream *s) {
  data = s->r64();
}

double Tag_Long::toDouble() const {
  return static_cast<double>(data);
}

qint32 Tag_Long::toInt() const {
  return static_cast<qint32>(data);
}

const QString Tag_Long::toString() const {
  return QString::number(data);
}

const QVariant Tag_Long::getData() const {
  return data;
}


// Tag_Float

Tag_Float::Tag_Float(TagDataStream *s) {
  union {qint32 d; float f;} fl;
  fl.d = s->r32();
  data = fl.f;
}
double Tag_Float::toDouble() const {
  return data;
}

const QString Tag_Float::toString() const {
  return QString::number(data);
}

const QVariant Tag_Float::getData() const {
  return data;
}


// Tag_Double

Tag_Double::Tag_Double(TagDataStream *s) {
  union {qint64 d; double f;} fl;
  fl.d = s->r64();
  data = fl.f;
}

double Tag_Double::toDouble() const {
  return data;
}

const QVariant Tag_Double::getData() const {
  return data;
}

const QString Tag_Double::toString() const {
  return QString::number(data);
}


// Tag_Byte_Array

Tag_Byte_Array::Tag_Byte_Array(TagDataStream *s) {
  len = s->r32();
  if (len) {
    s->r(len, data);
  }
}

int Tag_Byte_Array::length() const {
  return len;
}

const std::vector<quint8> &Tag_Byte_Array::toByteArray() const {
  return data;
}

const QVariant Tag_Byte_Array::getData() const {
  return QByteArray(reinterpret_cast<const char*>(&data[0]), len);
}

const QString Tag_Byte_Array::toString() const {
  try {
    return QString::fromLatin1(reinterpret_cast<const char *>(&data[0]));
  } catch(...) {}

  return "<Binary data>";
}


// Tag_String

Tag_String::Tag_String(TagDataStream *s) {
  int len = s->r16();
  data = s->utf8(len);
}

const QString Tag_String::toString() const {
  return data;
}

const QVariant Tag_String::getData() const {
  return data;
}


// Tag_List

template <class T>
static void setListData(QList<Tag *> *data, int len,
                        TagDataStream *s) {
  for (int i = 0; i < len; i++)
    data->append(new T(s));
}

Tag_List::Tag_List(TagDataStream *s) {
  quint8 type = s->r8();
  int len = s->r32();
  if (len == 0)  // empty list, type is invalid
    return;

  switch (type) {
    case Tag::TAG_END:        /* should be sorted out as len==0 */ break;
    case Tag::TAG_BYTE:       setListData<Tag_Byte>(&data, len, s); break;
    case Tag::TAG_SHORT:      setListData<Tag_Short>(&data, len, s); break;
    case Tag::TAG_INT:        setListData<Tag_Int>(&data, len, s); break;
    case Tag::TAG_LONG:       setListData<Tag_Long>(&data, len, s); break;
    case Tag::TAG_FLOAT:      setListData<Tag_Float>(&data, len, s); break;
    case Tag::TAG_DOUBLE:     setListData<Tag_Double>(&data, len, s); break;
    case Tag::TAG_BYTE_ARRAY: setListData<Tag_Byte_Array>(&data, len, s); break;
    case Tag::TAG_STRING:     setListData<Tag_String>(&data, len, s); break;
    case Tag::TAG_LIST:       setListData<Tag_List>(&data, len, s); break;
    case Tag::TAG_COMPOUND:   setListData<Tag_Compound>(&data, len, s); break;
    case Tag::TAG_INT_ARRAY:  setListData<Tag_Int_Array>(&data, len, s); break;
    case Tag::TAG_LONG_ARRAY: setListData<Tag_Long_Array>(&data, len, s); break;
    default: throw "Unknown type";
  }
}

Tag_List::~Tag_List() {
  for (auto i = data.constBegin(); i != data.constEnd(); i++)
    delete *i;
}

int Tag_List::length() const {
  return data.count();
}

const Tag *Tag_List::at(int index) const {
  return data[index];
}

const QString Tag_List::toString() const {
  QStringList ret;
  ret << "[";
  for (auto i = data.constBegin(); i != data.constEnd(); i++) {
    ret << (*i)->toString();
    ret << ", ";
  }
  ret.last() = "]";
  return ret.join("");
}

const QVariant Tag_List::getData() const {
  QList<QVariant> lst;
  for (auto i = data.constBegin(); i != data.constEnd(); i++) {
    lst << (*i)->getData();
  }
  return lst;
}


// Tag_Compound

Tag_Compound::Tag_Compound(TagDataStream *s) {
  quint8 type;
  while ((type = s->r8()) != 0) {
    // until tag_end
    quint16 len = s->r16();
    QString key = s->utf8(len);
    Tag *child;
    switch (type) {
      case Tag::TAG_BYTE:       child = new Tag_Byte(s); break;
      case Tag::TAG_SHORT:      child = new Tag_Short(s); break;
      case Tag::TAG_INT:        child = new Tag_Int(s); break;
      case Tag::TAG_LONG:       child = new Tag_Long(s); break;
      case Tag::TAG_FLOAT:      child = new Tag_Float(s); break;
      case Tag::TAG_DOUBLE:     child = new Tag_Double(s); break;
      case Tag::TAG_BYTE_ARRAY: child = new Tag_Byte_Array(s); break;
      case Tag::TAG_STRING:     child = new Tag_String(s); break;
      case Tag::TAG_LIST:       child = new Tag_List(s); break;
      case Tag::TAG_COMPOUND:   child = new Tag_Compound(s); break;
      case Tag::TAG_INT_ARRAY:  child = new Tag_Int_Array(s); break;
      case Tag::TAG_LONG_ARRAY: child = new Tag_Long_Array(s); break;
      default: throw "Unknown tag";
    }
    children[key] = child;
  }
}

Tag_Compound::~Tag_Compound() {
  for (auto i = children.constBegin(); i != children.constEnd(); i++)
    delete i.value();
}

bool Tag_Compound::has(const QString key) const {
  return children.contains(key);
}

const Tag *Tag_Compound::at(const QString key) const {
  if (!children.contains(key))
    return &NBT::Null;
  return children[key];
}

int Tag_Compound::length() const {
  return children.size();
}

const QString Tag_Compound::toString() const {
  QStringList ret;
  ret << "{\n";
  for (auto i = children.constBegin(); i != children.constEnd(); i++) {
    ret << "\t" << i.key() << " = '" << i.value()->toString() << "',\n";
  }
  ret.last() = "}";
  return ret.join("");
}

const QVariant Tag_Compound::getData() const {
  QMap<QString, QVariant> map;
  for (auto i = children.constBegin(); i != children.constEnd(); i++) {
    map.insert(i.key(), i.value()->getData());
  }
  return map;
}


// Tag_Int_Array

Tag_Int_Array::Tag_Int_Array(TagDataStream *s) {
  len = s->r32();
  data.resize(len);
  for (int i = 0; i < len; i++)
    data[i] = s->r32();
}

const std::vector<qint32>& Tag_Int_Array::toIntArray() const {
  return data;
}

int Tag_Int_Array::length() const {
  return len;
}

const QString Tag_Int_Array::toString() const {
  QStringList ret;
  ret << "[";
  for (int i = 0; i < len; ++i) {
    ret << QString::number(data[i]) << ",";
  }
  ret.last() = "]";
  return ret.join("");
}

const QVariant Tag_Int_Array::getData() const {
  QList<QVariant> ret;
  for (int i = 0; i < len; ++i) {
    ret.push_back(data[i]);
  }

  return ret;
}


// Tag_Long_Array

Tag_Long_Array::Tag_Long_Array(TagDataStream *s) {
  len = s->r32();
  data.resize(len);
  for (int i = 0; i < len; i++)
    data[i] = s->r64();
}

const std::vector<qint64> &Tag_Long_Array::toLongArray() const {
  return data;
}

int Tag_Long_Array::length() const {
  return len;
}

const QString Tag_Long_Array::toString() const {
  QStringList ret;
  ret << "[";
  for (int i = 0; i < len; ++i) {
    ret << QString::number(data[i]) << ",";
  }
  ret.last() = "]";
  return ret.join("");
}

const QVariant Tag_Long_Array::getData() const {
  QList<QVariant> ret;
  for (int i = 0; i < len; ++i) {
    ret.push_back(data[i]);
  }

  return ret;
}
