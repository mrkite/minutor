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
#include "json.h"
#include <QtCore>

enum Token {
	TokenNULL,
	TokenTRUE,
	TokenFALSE,
	TokenString,
	TokenNumber,
	TokenObject,
	TokenArray,
	TokenObjectClose,
	TokenArrayClose,
	TokenKeySeparator,
	TokenValueSeparator
};

class JSONHelper
{
public:
	JSONHelper(QString data) : data(data) {pos=0;len=data.length();}
	~JSONHelper() {}
	Token nextToken()
	{
		while (pos<len && data.at(pos).isSpace()) pos++;
		if (pos==len) throw JSONParseException("Unexpected EOF",location());
		QChar c=data.at(pos++);
		if (c.isLetter()) //keyword like NULL,TRUE or FALSE
		{
			int start=pos-1;
			while (pos<len && data.at(pos).isLetter())
				pos++;
			QStringRef ref=data.midRef(start,pos-start);
			if (ref.compare("null",Qt::CaseInsensitive)==0)
				return TokenNULL;
			if (ref.compare("true",Qt::CaseInsensitive)==0)
				return TokenTRUE;
			if (ref.compare("false",Qt::CaseInsensitive)==0)
				return TokenFALSE;
			throw JSONParseException("Unquoted string",location());
		}
		if (c.isDigit() || c=='-') //double or hex
		{
			pos--;
			return TokenNumber;
		}
		switch (c.unicode())
		{
		case '"': return TokenString;
		case '{': return TokenObject;
		case '}': return TokenObjectClose;
		case '[': return TokenArray;
		case ']': return TokenArrayClose;
		case ':': return TokenKeySeparator;
		case ',': return TokenValueSeparator;
		default: throw JSONParseException(QString("Unexpected character: %1").arg(c),location());
		}
	};
	QString readString()
	{
		QString r;
		while (pos<len && data.at(pos)!='"')
		{
			if (data.at(pos)=='\\')
			{
				pos++; if (pos==len) throw JSONParseException("Unexpected EOF",location());
				switch (data.at(pos++).unicode())
				{
				case '"': r+='"'; break;
				case '\\': r+='\\'; break;
				case '/': r+='/'; break;
				case 'b': r+='\b'; break;
				case 'f': r+='\f'; break;
				case 'n': r+='\n'; break;
				case 'r': r+='\r'; break;
				case 't': r+='\t'; break;
				case 'u': //hex
						  {
							  int num=0;
							  for (int i=0;i<4;i++)
							  {
								  if (pos==len) throw JSONParseException("Unexpected EOF",location());
								  num<<=4;
								  char c=data.at(pos++).unicode();
								  if (c>='0' && c<='9')
									  num|=c-'0';
								  else if (c>='a' && c<='f')
									  num|=c-'a'+10;
								  else if (c>='A' && c<='F')
									  num|=c-'A'+10;
								  else throw JSONParseException("Invalid hex code",location());
							  }
							  r+=QChar(num);
						  }
						  break;
				default: throw JSONParseException("Unknown escape sequence",location());
				}
			}
			else
				r+=data.at(pos++);
		}
		pos++;
		return r;
	}
	double readDouble()
	{
		double sign=1.0;
		if (data.at(pos)=='-')
		{
			sign=-1.0;
			pos++;
		}
		else if (data.at(pos)=='+')
			pos++;
		double value=0.0;
		while (pos<len && data.at(pos).isDigit())
		{
			value*=10.0;
			value+=data.at(pos++).unicode()-'0';
		}
		if (pos==len) throw JSONParseException("Unexpected EOF",location());
		if (data.at(pos)=='.')
		{
			double pow10=10.0;
			pos++;
			while (pos<len && data.at(pos).isDigit())
			{
				value+=(data.at(pos++).unicode()-'0')/pow10;
				pow10*=10.0;
			}
		}
		if (pos==len) throw JSONParseException("Unexpected EOF",location());
		double scale=1.0;
		bool frac=false;
		if (data.at(pos)=='e' || data.at(pos)=='E')
		{
			pos++;
			if (pos==len) throw JSONParseException("Unexpected EOF",location());
			if (data.at(pos)=='-')
			{
				frac=true;
				pos++;
			}
			else if (data.at(pos)=='+')
				pos++;
			unsigned int expon=0;
			while (pos<len && data.at(pos).isDigit())
			{
				expon*=10.0;
				expon+=data.at(pos++).unicode()-'0';
			}
			if (expon>308) expon=308;
			while (expon>=50) {scale *= 1E50; expon-=50; }
			while (expon>=8) {scale *=1E8; expon-=8; }
			while (expon>0) {scale*=10.0; expon-=1;}
		}
		return sign*(frac?(value/scale):(value*scale));
	}
	QString location()
	{
		int line=1;
		int col=0;
		int cpos=pos;
		bool doneCol=false;
		while (cpos>=0)
		{
			if (data.at(cpos)=='\n')
			{
				doneCol=true;
				line++;
			}
			if (!doneCol) col++;
			cpos--;
		}
		return QString("Line: %1, Offset: %2").arg(line).arg(col);
	}
private:
	int pos,len;
	QString data;
};

JSONData *JSON::parse(const QString data)
{
	JSONHelper reader(data);
	Token type=reader.nextToken();
	switch (type)
	{
	case TokenObject: //hash
		return new JSONObject(reader);
	case TokenArray: //array
		return new JSONArray(reader);
	default:
		throw JSONParseException("Doesn't start with object or array",reader.location());
		break;
	}
	return NULL;
}
static JSONData Null;
JSONData::JSONData()
{
}
JSONData::~JSONData()
{
}
bool JSONData::has(const QString)
{
	return false;
}
JSONData *JSONData::at(const QString)
{
	return &Null;
}
JSONData *JSONData::at(int)
{
	return &Null;
}
int JSONData::length()
{
	return 0;
}
QString JSONData::asString()
{
	return "";
}
double JSONData::asNumber()
{
	return 0.0;
}
bool JSONData::asBool()
{
	return false;
}

JSONBool::JSONBool(bool val)
{
	data=val;
}
bool JSONBool::asBool()
{
	return data;
}

JSONString::JSONString(QString val)
{
	data=val;
}
QString JSONString::asString()
{
	return data;
}

JSONNumber::JSONNumber(double val)
{
	data=val;
}
double JSONNumber::asNumber()
{
	return data;
}

JSONObject::JSONObject(JSONHelper &reader)
{
	while (true)
	{
		Token type=reader.nextToken();
		if (type==TokenObjectClose)
			break;
		if (type!=TokenString)
			throw JSONParseException("Expected quoted string",reader.location());
		QString key=reader.readString();
		if (key.length()==0) throw JSONParseException("Empty object key",reader.location());
		if (reader.nextToken()!=TokenKeySeparator) throw JSONParseException("Expected ':'",reader.location());
		JSONData *value;
		switch (reader.nextToken())
		{
		case TokenNULL: value=NULL; break;
		case TokenTRUE: value=new JSONBool(true); break;
		case TokenFALSE: value=new JSONBool(false); break;
		case TokenString: value=new JSONString(reader.readString()); break;
		case TokenNumber: value=new JSONNumber(reader.readDouble()); break;
		case TokenObject: value=new JSONObject(reader); break;
		case TokenArray: value=new JSONArray(reader); break;
		default: throw JSONParseException("Expected value",reader.location());
		}
		children[key]=value;
		type=reader.nextToken(); //comma or end
		if (type==TokenObjectClose)
			break;
		if (type!=TokenValueSeparator)
			throw JSONParseException("Expected ',' or '}'",reader.location());
	}
}
JSONObject::~JSONObject()
{
	QHash<QString, JSONData*>::const_iterator i;
	for (i=children.constBegin();i!=children.constEnd();i++)
		delete i.value();
}
bool JSONObject::has(QString key)
{
	return children.contains(key);
}
JSONData *JSONObject::at(QString key)
{
	if (children.contains(key))
		return children[key];
	return &Null;
}

JSONArray::JSONArray(JSONHelper &reader)
{
	while (true)
	{
		Token type=reader.nextToken();
		if (type==TokenArrayClose)
			break;
		JSONData *value;
		switch (type)
		{
		case TokenNULL: value=NULL; break;
		case TokenTRUE: value=new JSONBool(true); break;
		case TokenFALSE: value=new JSONBool(false); break;
		case TokenString: value=new JSONString(reader.readString()); break;
		case TokenNumber: value=new JSONNumber(reader.readDouble()); break;
		case TokenObject: value=new JSONObject(reader); break;
		case TokenArray: value=new JSONArray(reader); break;
		default: throw JSONParseException("Expected Value",reader.location());
		}
		data.append(value);
		type=reader.nextToken(); //comma or end
		if (type==TokenArrayClose)
			break;
		if (type!=TokenValueSeparator)
			throw JSONParseException("Expected ',' or ']'",reader.location());
	}
}
JSONArray::~JSONArray()
{
	QList<JSONData *>::const_iterator i;
	for (i=data.constBegin();i!=data.constEnd();i++)
		delete *i;
}
int JSONArray::length()
{
	return data.length();
}
JSONData *JSONArray::at(int index)
{
	if (index<data.length())
		return data[index];
	return &Null;
}
