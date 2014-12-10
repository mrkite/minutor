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


EntityInfo::EntityInfo(	QString category, QColor brushColor, QColor  penColor )
: category(category)
, brushColor(brushColor)
, penColor(penColor)
{}



EntityIdentifier::EntityIdentifier()
{
}

EntityIdentifier::~EntityIdentifier()
{
	// remove all packs
//	for (int i=0; i<packs.length(); i++)
//	{
//		for (int j=0; j<packs[i].length(); j++)
//			delete packs[i][j];
//	}
}


void EntityIdentifier::enableDefinitions(int pack)
{
	if (pack<0) return;
	packenabled[pack]=true;
}

void EntityIdentifier::disableDefinitions(int pack)
{
	if (pack<0) return;
	packenabled[pack]=false;
}

int EntityIdentifier::addDefinitions(JSONArray *defs,int pack)
{
	if (pack==-1)
	{
		pack=packenabled.size();
		packenabled[pack]=true;
	}
	int len=defs->length();
	for (int i=0;i<len;i++)
		parseCategoryDefinition(dynamic_cast<JSONObject *>(defs->at(i)),pack);
	return pack;
}


void EntityIdentifier::parseCategoryDefinition( JSONObject *data, int pack )
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
	categories[category] = catcolor;

	if (data->has("entity")) {
		JSONArray *entities=dynamic_cast<JSONArray *>(data->at("entity"));
		int len=entities->length();

		for (int e=0;e<len;e++)
			parseEntityDefinition(dynamic_cast<JSONObject *>(entities->at(e)), category, pack);
	}
}

void EntityIdentifier::parseEntityDefinition( JSONObject *entity, QString const & category, int pack )
{
	QString id;
	if (entity->has("id"))
		id = entity->at("id")->asString();
	else
		id = "Unknown";

	QColor catcolor(categories[category]);
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
	packs[pack][category].insert( id, EntityInfo(category,catcolor,color) );
}


int EntityIdentifier::getNumCategories()
{
	return categories.size();
}

QMap<QString,QColor> const & EntityIdentifier::getCategoryMap()
{
	return categories;
}

QColor EntityIdentifier::getCategoryColor(QString name)
{
	return categories[name];
}

