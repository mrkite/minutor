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
    void specialArea(double x1, double y1, double z1,
                     double x2, double y2, double z2,
                     QString type, QString display, QVariant properties);
	void showProperties(int x, int y, int z);

signals:
	void worldLoaded(bool isLoaded);

private:
	void createActions();
	void createMenus();
	void createStatusBar();
    void loadStructures(const QDir &dataPath);

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
        double x1, y1, z1, x2, y2, z2 ;
		QString type;
		QString display;
		QVariant properties;

        bool intersects(double x1, double y1, double z1,
                        double x2, double y2, double z2) const
        {
            return  x1 <= this->x2 &&
              this->x1 <= x2 &&
                    y1 <= this->y2 &&
              this->y1 <= y2 &&
                    z1 <= this->z2 &&
              this->z1 <= z2;
        }
    };
    //           type                 x    z
    typedef QMap<QString, QHash<QPair<int, int>, Entity> > EntityMap;
    EntityMap entities;
    int maxentitydistance;
	Properties * propView;
};

#endif
