/*
   Copyright (c) 2013, Sean Kasun
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QStringList>

#include "nbt.h"
#include <zlib.h>

// this handles decoding the gzipped level.dat
NBT::NBT(const QString level)
{
	root=&NBT::Null; //just in case we die

	QFile f(level);
	f.open(QIODevice::ReadOnly);
	QByteArray data=f.readAll();
	f.close();

	QByteArray nbt;
	z_stream strm;
	static const int CHUNK_SIZE = 8192;
	char out[CHUNK_SIZE];
	strm.zalloc=Z_NULL;
	strm.zfree=Z_NULL;
	strm.opaque=Z_NULL;
	strm.avail_in=data.size();
	strm.next_in=(Bytef*)data.data();

	inflateInit2(&strm,15+32);
	do {
		strm.avail_out=CHUNK_SIZE;
		strm.next_out=(Bytef*)out;
		inflate(&strm,Z_NO_FLUSH);
		nbt.append(out,CHUNK_SIZE-strm.avail_out);
	} while (strm.avail_out==0);
	inflateEnd(&strm);

	TagDataStream s(nbt.constData(),nbt.size());

	if (s.r8()==10) //compound
	{
		s.skip(s.r16()); //skip name
		root=new Tag_Compound(s);
	}
}


//this handles decoding a compressed() section of a region file
NBT::NBT(const uchar *chunk)
{
	root=&NBT::Null; //just in case

	// find chunk size
	int length=(chunk[0]<<24)|(chunk[1]<<16)|(chunk[2]<<8)|chunk[3];
	if (chunk[4]!=2) //rfc1950
		return;

	z_stream strm;
	static const int CHUNK_SIZE = 8192;
	char out[CHUNK_SIZE];
	strm.zalloc=Z_NULL;
	strm.zfree=Z_NULL;
	strm.opaque=Z_NULL;
	strm.avail_in=length-1;
	strm.next_in=(Bytef*)chunk+5;

	QByteArray nbt;

	inflateInit(&strm);
	do {
		strm.avail_out=CHUNK_SIZE;
		strm.next_out=(Bytef*)out;
		inflate(&strm,Z_NO_FLUSH);
		nbt.append(out,CHUNK_SIZE-strm.avail_out);
	} while (strm.avail_out==0);
	inflateEnd(&strm);

	TagDataStream s(nbt.constData(),nbt.size());

	if (s.r8()==10) //compound
	{
		s.skip(s.r16()); //skip name
		root=new Tag_Compound(s);
	}
}

Tag NBT::Null;

bool NBT::has(const QString key)
{
	return root->has(key);
}

Tag *NBT::at(const QString key)
{
	return root->at(key);
}

NBT::~NBT()
{
	if (root!=&NBT::Null)
		delete root;
}

/********** TAGS ****************/

Tag::Tag()
{
}
Tag::~Tag()
{
}
int Tag::length()
{
	qWarning()<<"Unhandled length";
	return 0;
}
bool Tag::has(const QString)
{
	return false;
}
Tag *Tag::at(const QString)
{
	return &NBT::Null;
}
Tag *Tag::at(int)
{
	return &NBT::Null;
}
QString Tag::toString()
{
	qWarning()<<"Unhandled toString";
	return "";
}
qint32 Tag::toInt()
{
	qWarning()<<"Unhandled toInt";
	return 0;
}
double Tag::toDouble()
{	
	qWarning()<<"Unhandled toDouble";
	return 0.0;
}
const quint8 *Tag::toByteArray()
{
	qWarning()<<"Unhandled toByteArray";
	return NULL;
}
const qint32 *Tag::toIntArray()
{
	qWarning()<<"Unhandled toIntArray";
	return NULL;
}
QVariant Tag::getData()
{
    qWarning()<<"Unhandled getData";
    return QVariant();
}

Tag_Byte::Tag_Byte(TagDataStream &s)
{
	data=s.r8();
}
int Tag_Byte::toInt()
{
	return data;
}

QString Tag_Byte::toString()
{
    return QString::number(data);
}

QVariant Tag_Byte::getData()
{
    return data;
}

Tag_Short::Tag_Short(TagDataStream &s)
{
	data=s.r16();
}

int Tag_Short::toInt()
{
    return data;
}


QString Tag_Short::toString()
{
    return QString::number(data);
}

QVariant Tag_Short::getData()
{
    return data;
}

Tag_Int::Tag_Int(TagDataStream &s)
{
	data=s.r32();
}
qint32 Tag_Int::toInt()
{
	return data;
}

QString Tag_Int::toString()
{
    return QString::number(data);
}

QVariant Tag_Int::getData()
{
    return data;
}

double Tag_Int::toDouble()
{
	return (double)data;
}


Tag_Long::Tag_Long(TagDataStream &s)
{
	data=s.r64();
}

double Tag_Long::toDouble()
{
    return (double)data;
}

qint32 Tag_Long::toInt()
{
    return (double)data;
}

QString Tag_Long::toString()
{
    return QString::number(data);
}

QVariant Tag_Long::getData()
{
    return data;
}

Tag_Float::Tag_Float(TagDataStream &s)
{
	union {qint32 d; float f;} fl;
	fl.d=s.r32();
	data=fl.f;
}
double Tag_Float::toDouble()
{
    return data;
}

QString Tag_Float::toString()
{
    return QString::number(data);
}

QVariant Tag_Float::getData()
{
    return data;
}

Tag_Double::Tag_Double(TagDataStream &s)
{
	union {qint64 d; double f;} fl;
	fl.d=s.r64();
	data=fl.f;
}
double Tag_Double::toDouble()
{
	return data;
}

QVariant Tag_Double::getData()
{
    return data;
}

QString Tag_Double::toString()
{
    return QString::number(data);
}

Tag_Byte_Array::Tag_Byte_Array(TagDataStream &s)
{
	len=s.r32();
	data=s.r(len);
}
Tag_Byte_Array::~Tag_Byte_Array()
{
	delete[] data;
}
int Tag_Byte_Array::length()
{
	return len;
}
const quint8 *Tag_Byte_Array::toByteArray()
{
	return data;
}

QVariant Tag_Byte_Array::getData()
{
    return QByteArray(reinterpret_cast<const char*>(data), len);
}

QString Tag_Byte_Array::toString()
{
    try
    {
        return QString::fromLatin1((char*)data);
    }
    catch(...)
    {

    }

    return "<Binary data>";
}

Tag_String::Tag_String(TagDataStream &s)
{
	int len=s.r16();
	data=s.utf8(len);
}
QString Tag_String::toString()
{
	return data;
}

QVariant Tag_String::getData()
{
    return data;
}

	template <class T>
static void setListData(QList<Tag *> &data,int len,TagDataStream &s)
{
	for (int i=0;i<len;i++)
		data.append(new T(s));
}

Tag_List::Tag_List(TagDataStream &s)
{
	quint8 type=s.r8();
	int len=s.r32();
    if (len==0) //empty list, type is invalid
        return;

	switch (type)
	{
	case 1: setListData<Tag_Byte>(data,len,s); break;
	case 2: setListData<Tag_Short>(data,len,s); break;
	case 3: setListData<Tag_Int>(data,len,s); break;
	case 4: setListData<Tag_Long>(data,len,s); break;
	case 5: setListData<Tag_Float>(data,len,s); break;
	case 6: setListData<Tag_Double>(data,len,s); break;
	case 7: setListData<Tag_Byte_Array>(data,len,s); break;
	case 8: setListData<Tag_String>(data,len,s); break;
	case 9: setListData<Tag_List>(data,len,s); break;
	case 10: setListData<Tag_Compound>(data,len,s); break;
	case 11: setListData<Tag_Int_Array>(data,len,s); break;
	default:
			 throw "Unknown type";
	}
}

Tag_List::~Tag_List()
{
	QList<Tag *>::const_iterator i;
	for (i=data.constBegin();i!=data.constEnd();i++)
		delete *i;
}
int Tag_List::length()
{
	return data.count();
}
Tag *Tag_List::at(int index)
{
	return data[index];
}

QString Tag_List::toString()
{
    QStringList ret;
    ret << "[";
    QList<Tag*>::iterator it, end = data.end();
    for (it = data.begin(); it != end; ++it)
    {
        ret << (*it)->toString();
        ret << ", ";
    }
    ret.last() = "]";
    return ret.join("");
}

QVariant Tag_List::getData()
{
    QList<QVariant> list;
    QList<Tag*>::iterator it, end = data.end();
    for (it = data.begin(); it != end; ++it)
    {
        list << (*it)->getData();
    }
    return list;
}

Tag_Compound::Tag_Compound(TagDataStream &s)
{
	quint8 type;
	while ((type=s.r8())!=0) //until tag_end
	{
		quint16 len=s.r16();
		QString key=s.utf8(len);
		Tag *child;
		switch (type)
		{
		case 1: child=new Tag_Byte(s); break;
		case 2: child=new Tag_Short(s); break;
		case 3: child=new Tag_Int(s); break;
		case 4: child=new Tag_Long(s); break;
		case 5: child=new Tag_Float(s); break;
		case 6: child=new Tag_Double(s); break;
		case 7: child=new Tag_Byte_Array(s); break;
		case 8: child=new Tag_String(s); break;
		case 9: child=new Tag_List(s); break;
		case 10: child=new Tag_Compound(s); break;
		case 11: child=new Tag_Int_Array(s); break;
		default: throw "Unknown tag";
		}
		children[key]=child;
	}
}
Tag_Compound::~Tag_Compound()
{
	QHash<QString, Tag*>::const_iterator i;
	for (i=children.constBegin();i!=children.constEnd();i++)
		delete i.value();
}
bool Tag_Compound::has(const QString key)
{
	return children.contains(key);
}
Tag *Tag_Compound::at(const QString key)
{
	if (!children.contains(key))
		return &NBT::Null;
	return children[key];
}

QString Tag_Compound::toString()
{
    QStringList ret;
    ret << "{\n";
    QHash<QString, Tag *>::iterator it, end = children.end();
    for (it = children.begin(); it != end; ++it)
    {
        ret << "\t" << it.key() << " = '" << it.value()->toString() << "',\n";
    }
    ret.last() = "}";
    return ret.join("");
}

QVariant Tag_Compound::getData()
{
    QMap<QString, QVariant> map;
    QHash<QString, Tag *>::iterator it, end = children.end();
    for (it = children.begin(); it != end; ++it)
    {
        map.insert(it.key(), it.value()->getData());
    }
    return map;
}

Tag_Int_Array::Tag_Int_Array(TagDataStream &s)
{
	len=s.r32();
	data=new qint32[len];
	for (int i=0;i<len;i++)
		data[i]=s.r32();
}
Tag_Int_Array::~Tag_Int_Array()
{
	delete[] data;
}
const qint32 *Tag_Int_Array::toIntArray()
{
	return data;
}
int Tag_Int_Array::length()
{
	return len;
}

QString Tag_Int_Array::toString()
{
    QStringList ret;
    ret << "[";
    for (int i = 0; i < len; ++i)
    {
        ret << QString::number(data[i]) << ",";
    }
    ret.last() = "]";
    return ret.join("");
}

QVariant Tag_Int_Array::getData()
{
    return toString();
}

TagDataStream::TagDataStream(const char *data,int len)
{
	this->data=(const quint8 *)data;
	this->len=len;
	pos=0;
}

quint8 TagDataStream::r8()
{
	return data[pos++];
}
quint16 TagDataStream::r16()
{
	quint16 r=data[pos++]<<8;
	r|=data[pos++];
	return r;
}
quint32 TagDataStream::r32()
{
	quint32 r=data[pos++]<<24;
	r|=data[pos++]<<16;
	r|=data[pos++]<<8;
	r|=data[pos++];
	return r;
}
quint64 TagDataStream::r64()
{
	quint64 r=(quint64)r32()<<32;
	r|=r32();
	return r;
}
quint8 *TagDataStream::r(int len) //you need to free anything read with this
{
	quint8 *r=new quint8[len];
	memcpy(r,data+pos,len);
	pos+=len;
	return r;
}
QString TagDataStream::utf8(int len)
{
	int old=pos;
	pos+=len;
	return QString::fromUtf8((const char *)data+old,len);
}
void TagDataStream::skip(int len)
{
	pos+=len;
}
