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


#ifndef __DEFINITIONMANAGER_H__
#define __DEFINITIONMANAGER_H__

#include <QtWidgets/QWidget>
#include <QHash>
#include <QString>
#include <QList>
#include <QVariant>
#include <QDateTime>

class QTableWidget;
class QTableWidgetItem;
class QCheckBox;
class BiomeIdentifier;
class BlockIdentifier;
class DimensionIdentifier;
class EntityIdentifier;
class MapView;
class JSONData;
class DefinitionUpdater;

struct Definition
{
	QString name;
	QString version;
	QString path;
	QString update;
	enum {Block,Biome,Dimension,Entity,Pack} type;
	int id;
	bool enabled;
	// for packs only
	int blockid,biomeid,dimensionid,entityid;
};

class DefinitionManager : public QWidget
{
	Q_OBJECT
public:
	explicit DefinitionManager(QWidget *parent = 0);
    ~DefinitionManager();
	void attachMapView(MapView *mapview);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	BlockIdentifier     *blockIdentifier();
	BiomeIdentifier     *biomeIdentifier();
	DimensionIdentifier *dimensionIdentifer();
	EntityIdentifier    *entityIdentifier();

	void autoUpdate();

signals:
	void packSelected(bool);
	void packsChanged();
	void updateFinished();

public slots:
	void selectedPack(QTableWidgetItem *,QTableWidgetItem *);
	void toggledPack(bool);
	void addPack();
	void removePack();
	void exportPack();
	void checkForUpdates();
	void updatePack(DefinitionUpdater *updater,QString filename,QDateTime timestamp);

private:
	QTableWidget *table;
	QList<QCheckBox *>checks;
	void installJson(QString path,bool overwrite=true,bool install=true);
	void installZip(QString path,bool overwrite=true,bool install=true);
	void loadDefinition(QString path);
	void loadDefinition(JSONData *,int pack=-1);
	void removeDefinition(QString path);
	void refresh();
	QHash<QString,Definition> definitions;
	BiomeIdentifier     *biomes;
	BlockIdentifier     *blocks;
	DimensionIdentifier *dimensionList;
	EntityIdentifier    *entities;
	QString selected;
	QList<QVariant> sorted;

	bool isUpdating;
	QList<DefinitionUpdater *> updateQueue;
	QHash<QString,QVariant> lastUpdated;
};

#endif
