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

#ifndef __ENTITYIDENTIFIER_H__
#define __ENTITYIDENTIFIER_H__

#include <QString>
#include <QColor>
#include <QList>
#include <QMap>

class JSONArray;
class JSONObject;


class EntityInfo
{
public:
	EntityInfo(	QString name, QString category, QColor brushColor, QColor  penColor );
	QString name;
	QString category;
	QColor  brushColor;
	QColor  penColor;
};

class EntityIdentifier
{
public:
	EntityIdentifier();
	~EntityIdentifier();

	// singleton global usable instance
	static EntityIdentifier& Instance() {
		static EntityIdentifier singleton;
		return singleton;
	}

	int  addDefinitions(JSONArray *,int pack=-1);
	void enableDefinitions(int);
	void disableDefinitions(int);

	// interface to list of main categories
	typedef QList<QPair<QString,QColor>> TcatList;
	int              getNumCategories();
	TcatList const & getCategoryList();
	QColor           getCategoryColor(QString name);

	// interface to single EntityInfo objects
	EntityInfo const & getEntityInfo( QString id );
	QColor             getBrushColor( QString id );
	QColor             getPenColor  ( QString id );

private:
	void parseCategoryDefinition( JSONObject *data, int pack );
	void parseEntityDefinition  ( JSONObject *entity, QString const & category, QColor catcolor, int pack );

	TcatList categories;  // main categories for entities
	bool addCategory(QPair<QString,QColor> cat);

	typedef QMap<QString,EntityInfo> TentityMap; // key:id_name value:EntityInfo
	QMap< int, bool >       packenabled;
	QMap< int, TentityMap > packs;
};

#endif
