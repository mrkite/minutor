/*
   Copyright (c) 2014, Mc_Etlam
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

#include "entityidentifier.h"
#include "json.h"
#include <QDebug>
#include <assert.h>

// ---------- ---------- ---------- ----------

EntityInfo::EntityInfo(	QString name, QString category, QColor brushColor, QColor  penColor )
	: name(name)
	, category(category)
	, brushColor(brushColor)
	, penColor(penColor)
{}

static EntityInfo entityDummy("Name unknown","Entity.Unknown",Qt::black,Qt::black);

// ---------- ---------- ---------- ----------

EntityIdentifier::EntityIdentifier()
{}

EntityIdentifier::~EntityIdentifier()
{}

EntityIdentifier& EntityIdentifier::Instance()
{
	static EntityIdentifier singleton;
	return singleton;
}

void EntityIdentifier::enableDefinitions(int packID)
{
	if (packID<0) return;
	for (auto it = packs.begin(); it != packs.end(); ++it)
	{
		if (it->packID == packID)
		{
			it->enabled = true;
			return;
		}
	}
}

void EntityIdentifier::disableDefinitions(int packID)
{
	if (packID<0) return;
	for (auto it = packs.begin(); it != packs.end(); ++it)
	{
		if (it->packID == packID)
			it->enabled = false;
	}
}

int EntityIdentifier::addDefinitions(JSONArray *defs,int packID)
{
	if (packID==-1)
	{
		// find largest used packID
		for (auto it = packs.begin(); it != packs.end(); ++it)
		{
			if (it->packID > packID)
				packID = it->packID;
		}
		packID++; // use one higher than largest found
		packs.append(TpackInfo(packID));
	}
	int len=defs->length();
	for (int i=0;i<len;i++)
		parseCategoryDefinition(dynamic_cast<JSONObject *>(defs->at(i)),packID);
	return packID;
}

EntityIdentifier::TentityMap& EntityIdentifier::getMapForPackID(int packID)
{
	for (auto it = packs.begin(); it != packs.end(); ++it)
	{
		if (it->packID == packID)
			return it->map;
	}
	return dummyMap;
}

void EntityIdentifier::parseCategoryDefinition( JSONObject *data, int packID )
{
	QString category;
	if (data->has("category"))
		category = data->at("category")->asString();
	else
		category = "Unknown";

	QColor catcolor;
	if (data->has("catcolor"))
	{
		QString colorname = data->at("catcolor")->asString();
		catcolor.setNamedColor( colorname );
		assert(catcolor.isValid());
	} else {// use hashed by name instead
		quint32 hue = qHash(category);
		catcolor.setHsv(hue % 360, 255, 255);
	}
	addCategory(qMakePair(category,catcolor));

	if (data->has("entity")) {
		JSONArray *entities=dynamic_cast<JSONArray *>(data->at("entity"));
		int len=entities->length();

		for (int e=0;e<len;e++)
			parseEntityDefinition(dynamic_cast<JSONObject *>(entities->at(e)), category, catcolor, packID);
	}
}

void EntityIdentifier::parseEntityDefinition( JSONObject *entity, QString const & category, QColor catcolor, int packID )
{
	QString id;
	if (entity->has("id"))
		id = entity->at("id")->asString();
	else
		id = "Unknown";

	if (entity->has("catcolor"))
	{
		QString colorname = entity->at("catcolor")->asString();
		catcolor.setNamedColor( colorname );
		assert(catcolor.isValid());
	}

	QColor color;
	if (entity->has("color"))
	{
		QString colorname = entity->at("color")->asString();
		color.setNamedColor( colorname );
		assert(color.isValid());
	}
	else
	{// use hashed by name instead
		quint32 hue = qHash(id);
		color.setHsv(hue % 360, 255, 255);
	}

	// enter entity into manager
	TentityMap& map = getMapForPackID(packID);
	map.insert( id, EntityInfo(id,category,catcolor,color) );
}

bool EntityIdentifier::addCategory(QPair<QString,QColor> cat)
{
	TcatList::const_iterator it = categories.begin();
    for (; it != categories.end(); ++it)
	{
		if (it->first == cat.first)
			return false;
	}
	categories.append(cat);
	return true;
}

int EntityIdentifier::getNumCategories() const
{
	return categories.size();
}

EntityIdentifier::TcatList const & EntityIdentifier::getCategoryList() const
{
	return categories;
}

QColor EntityIdentifier::getCategoryColor(QString name) const
{
	for (TcatList::const_iterator it = categories.begin(); it != categories.end() ;++it)
	{
		if (it->first==name)
			return it->second;
	}
	return Qt::black; // dummy
}

EntityInfo const & EntityIdentifier::getEntityInfo( QString id ) const
{
	for (auto it = packs.begin(); it != packs.end(); ++it)
	{
		if (it->enabled)
		{
			TentityMap::const_iterator info = it->map.find(id);
			if (info != it->map.end())
			{ // found it
				return info.value();
			}
		}
	}
		
	return entityDummy;
}

QColor EntityIdentifier::getBrushColor( QString id ) const
{
	return getEntityInfo(id).brushColor;
}

QColor EntityIdentifier::getPenColor( QString id ) const
{
	return getEntityInfo(id).penColor;
}
