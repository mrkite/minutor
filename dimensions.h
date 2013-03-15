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


#ifndef __DIMENSIONS_H__
#define __DIMENSIONS_H__

#include <QString>
#include <QList>
#include <QHash>
#include <QDir>

class QMenu;
class QAction;
class QActionGroup;
class JSONArray;

class Dimension
{
public:
	Dimension(QString path,int scale):path(path),scale(scale) {}
	QString path;
	int scale;
};

class DimensionDef;

class Dimensions : public QObject
{
	Q_OBJECT
public:
	Dimensions();
	~Dimensions();
	int addDefinitions(JSONArray *,int pack=-1);
	void enableDefinitions(int);
	void disableDefinitions(int);
	void getDimensions(QDir path,QMenu *menu,QObject *parent);
	void removeDimensions(QMenu *menu);
signals:
	void dimensionChanged(Dimension &dim);
private slots:
	void viewDimension();
private:
	void addDimension(QDir path,QString dir,QString name,int scale,QObject *parent);
	QList<QAction *> items;
	QList<Dimension> dimensions;
	QList<DimensionDef*> definitions;
	QList<QList<DimensionDef*> > packs;
	QActionGroup *group;

	QHash<QString,bool> foundDimensions;
};

#endif
