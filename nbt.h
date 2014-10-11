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

#ifndef __NBT_H__
#define __NBT_H__

class QString;
class QByteArray;

#include <QHash>
#include <QString>
#include <QVariant>

class TagDataStream
{
public:
	TagDataStream(const char *data,int len);
	quint8 r8();
	quint16 r16();
	quint32 r32();
	quint64 r64();
	quint8 *r(int len);
	QString utf8(int len);
	void skip(int len);
private:
	const quint8 *data;
	int pos,len;
};

class Tag
{
public:
	Tag();
	virtual ~Tag();
	virtual bool has(const QString key);
	virtual int length();
	virtual Tag *at(const QString key);
	virtual Tag *at(int index);
	virtual QString toString();
	virtual qint32 toInt();
	virtual double toDouble();
	virtual const quint8 *toByteArray();
	virtual const qint32 *toIntArray();
    virtual QVariant getData();
};

class NBT
{
public:
	NBT(const QString level);
	NBT(const uchar *chunk);
	~NBT();

	bool has(const QString key);
	Tag *at(const QString key);

	static Tag Null;
private:
	Tag *root;
};

class Tag_Byte : public Tag
{
public:
	Tag_Byte(TagDataStream &s);
	int toInt();
    virtual QString toString();
    virtual QVariant getData();
private:
	quint8 data;
};

class Tag_Short : public Tag
{
public:
	Tag_Short(TagDataStream &s);
    int toInt();
    virtual QString toString();
    virtual QVariant getData();
private:
	qint16 data;
};

class Tag_Int : public Tag
{
public:
	Tag_Int(TagDataStream &s);
	qint32 toInt();
	double toDouble();
    virtual QString toString();
    virtual QVariant getData();
private:
	qint32 data;
};

class Tag_Long : public Tag
{
public:
	Tag_Long(TagDataStream &s);
    qint32 toInt();
    double toDouble();
    virtual QString toString();
    virtual QVariant getData();
private:
	qint64 data;
};

class Tag_Float : public Tag
{
public:
    Tag_Float(TagDataStream &s);

    virtual double toDouble();
    virtual QString toString();
    virtual QVariant getData();

private:
	float data;
};

class Tag_Double : public Tag
{
public:
	Tag_Double(TagDataStream &s);
	double toDouble();
    virtual QString toString();
    virtual QVariant getData();
private:
	double data;
};

class Tag_Byte_Array : public Tag
{
public:
	Tag_Byte_Array(TagDataStream &s);
	~Tag_Byte_Array();
	int length();
	const quint8 *toByteArray();
    virtual QString toString();
    virtual QVariant getData();
private:
	const quint8 *data;
	int len;
};

class Tag_String : public Tag
{
public:
	Tag_String(TagDataStream &s);
	QString toString();
    virtual QVariant getData();
private:
	QString data;
};

class Tag_List : public Tag
{
public:
	Tag_List(TagDataStream &s);
	~Tag_List();
	Tag *at(int index);
	int length();
    virtual QString toString();
    virtual QVariant getData();
private:
	QList<Tag *> data;
};

class Tag_Compound : public Tag
{
public:
	Tag_Compound(TagDataStream &s);
	~Tag_Compound();
	bool has(const QString key);
	Tag *at(const QString key);
    virtual QString toString();
    virtual QVariant getData();
private:
	QHash<QString, Tag *> children;
};

class Tag_Int_Array : public Tag
{
public:
	Tag_Int_Array(TagDataStream &s);
	~Tag_Int_Array();
	int length();
	const qint32 *toIntArray();
    virtual QString toString();
    virtual QVariant getData();
private:
	int len;
	qint32 *data;
};

#endif
