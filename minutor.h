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


#ifndef __MINUTOR_H__
#define __MINUTOR_H__

#include <QtWidgets/QMainWindow>
#include <QDir>
#include <QVariant>
#include <QSharedPointer>
#include <QSet>

class QAction;
class QActionGroup;
class QMenu;
class QProgressDialog;
class MapView;
class LabelledSlider;
class DefinitionManager;
class DimensionIdentifier;
class Settings;
class DimensionInfo;
class WorldSave;
class Properties;
class OverlayItem;

class Location
{
public:
	Location(double x,double z):x(x),z(z) {}
	double x,z;
};

class Minutor : public QMainWindow
{
	Q_OBJECT
public:
	Minutor();

	void loadWorld( QDir path );

	void savePNG( QString filename, bool autoclose=false, bool regionChecker=false, bool chunkChecker=false );

	void jumpToXZ( int blockX, int blockZ );  // jumps to the block coords provided

	void setViewLighting( bool value );       // set View->Ligthing
	void setViewMobspawning( bool value );    // set View->Mob_Spawning
	void setViewCavemode( bool value );       // set View->Cave_Mode
	void setViewDepthshading( bool value );   // set View->Depth_Shading
	void setSingleLayer( bool value );        // set View->Single_Layer

	void setDepth( int value );               // set Depth-Slider

private slots:
	void openWorld();
	void open();
	void closeWorld();
	void reload();
	void save();

	void jumpToLocation();
	void viewDimension(DimensionInfo &dim);
	void toggleFlags();

	void about();

	void updateDimensions();
	void rescanWorlds();
	void saveProgress(QString status,double value);
	void saveFinished();
	void addOverlayItem(QSharedPointer<OverlayItem> item);
	void addOverlayItemType(QString type, QColor color, QString dimension = "");
	void showProperties(QVariant props);

signals:
	void worldLoaded(bool isLoaded);

private:
	void createActions();
	void createMenus();
	void createStatusBar();
	void loadStructures(const QDir &dataPath);
    void populateEntityOverlayMenu();
    QKeySequence generateUniqueKeyboardShortcut( QString & actionName );
	
	QString getWorldName(QDir path);
	void getWorldList();

	MapView *mapview;
	LabelledSlider *depth;
	QProgressDialog *progress;
	bool progressAutoclose;

	QMenu *fileMenu, *worldMenu;
	QMenu *viewMenu, *jumpMenu, *dimMenu;
	QMenu *helpMenu;
	QMenu *structureOverlayMenu, *entityOverlayMenu;

	QList<QAction *>worlds;
	QAction *openAct, *reloadAct, *saveAct, *exitAct;
	QAction *jumpSpawnAct;
	QList<QAction *>players;
	QAction *lightingAct, *mobSpawnAct, *caveModeAct, *depthShadingAct, *singleLayerAct;
	QAction *manageDefsAct;
	QAction *refreshAct;
	QAction *aboutAct;
	QAction *settingsAct;
	QAction *updatesAct;
	QList<QAction*> structureActions;
	QList<QAction*> entityActions;

	//loaded world data
	QList<Location> locations;
	DefinitionManager *dm;
	Settings *settings;
	DimensionIdentifier *dimensions;
	QDir currentWorld;

	//           type                 x    z
	typedef QMap<QString, QHash<QPair<int, int>, QSharedPointer<OverlayItem> > > OverlayMap;
	OverlayMap overlayItems;
	QSet<QString> overlayItemTypes;
	int maxentitydistance;
	Properties * propView;
};

#endif
