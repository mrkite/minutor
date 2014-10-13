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

class QAction;
class QActionGroup;
class QMenu;
class QProgressDialog;
class MapView;
class LabelledSlider;
class DefinitionManager;
class Settings;
class BiomeIdentifier;
class BlockIdentifier;
class Dimensions;
class Dimension;
class WorldSave;
class Properties;

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

	void loadWorld(QDir path);

	/// Jumps to the block coords provided
	void jumpToXZ(int blockX, int blockZ);

private slots:
	void openWorld();
	void open();
	void closeWorld();
	void reload();
	void save();

	void jumpToLocation();
	void viewDimension(Dimension &dim);
	void toggleFlags();

	void about();

	void updateDimensions();
	void rescanWorlds();
	void saveProgress(QString status,double value);
	void saveFinished();
    void specialBlock(int x, int y, int z, QString type, QString display, QVariant properties);
    void showProperties(int x, int y, int z);

signals:
	void worldLoaded(bool isLoaded);

private:
	void createActions();
	void createMenus();
	void createStatusBar();

	QString getWorldName(QDir path);
	void getWorldList();

	MapView *mapview;
	LabelledSlider *depth;
	QProgressDialog *progress;

	QMenu *fileMenu, *worldMenu;
	QMenu *viewMenu, *jumpMenu, *dimMenu;
	QMenu *helpMenu;
    QMenu *entitiesMenu;

	QList<QAction *>worlds;
	QAction *openAct, *reloadAct, *saveAct, *exitAct;
	QAction *jumpSpawnAct;
	QList<QAction *>players;
	QAction *lightingAct, *mobSpawnAct, *caveModeAct, *depthShadingAct;
	QAction *manageDefsAct;
	QAction *refreshAct;
	QAction *aboutAct;
	QAction *settingsAct;
	QAction *updatesAct;
    QList<QAction*> entityActions;

	//loaded world data
	QList<Location> locations;
	DefinitionManager *dm;
	Settings *settings;
	Dimensions *dimensions;
	QDir currentWorld;

    //special entities and objects with properties
    struct Entity
    {
        int x, y, z;
        QString type;
        QString display;
        QVariant properties;
    };
    //           type                 x     z
    typedef QMap<QString, QHash<QPair<int, int>, Entity> > EntityMap;
    EntityMap entities;
    Properties * propView;
};

#endif
