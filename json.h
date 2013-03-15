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

#ifndef __JSON_H__
#define __JSON_H__

#include <QHash>
#include <QList>

class JSONHelper;

class JSONData
{
public:
	JSONData();
	virtual ~JSONData();
	virtual bool has(const QString key);
	virtual JSONData *at(const QString key);
	virtual JSONData *at(int index);
	virtual int length();
	virtual QString asString();
	virtual double asNumber();
	virtual bool asBool();
};

class JSONBool : public JSONData
{
public:
	JSONBool(bool val);
	bool asBool();
private:
	bool data;
};

class JSONString : public JSONData
{
public:
	JSONString(QString val);
	QString asString();
private:
	QString data;
};

class JSONNumber : public JSONData
{
public:
	JSONNumber(double val);
	double asNumber();
private:
	double data;
};

class JSONObject : public JSONData
{
public:
	JSONObject(JSONHelper &);
	~JSONObject();
	bool has(const QString key);
	JSONData *at(const QString key);
private:
	QHash<QString,JSONData *>children;
};
class JSONArray : public JSONData
{
public:
	JSONArray(JSONHelper &);
	~JSONArray();
	JSONData *at(int index);
	int length();
private:
	QList<JSONData *>data;
};

class JSONParseException
{
public:
	JSONParseException(QString reason,QString at) : reason(QString("%1 at %2").arg(reason).arg(at)) {}
	QString reason;
};

class JSON
{
public:
	static JSONData *parse(const QString data);
};

#endif
