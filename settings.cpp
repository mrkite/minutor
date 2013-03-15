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

#include "settings.h"
#include <QDebug>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QSettings>
#include <QDir>

Settings::Settings(QWidget *parent) : QWidget(parent)
{
	setWindowFlags(Qt::Window);
	setWindowTitle(tr("%1 Settings").arg(qApp->applicationName()));

	QVBoxLayout *layout=new QVBoxLayout;

	QGroupBox *paths=new QGroupBox("Minecraft Location");
	QVBoxLayout *pathlayout=new QVBoxLayout;
	QCheckBox *def=new QCheckBox("Use Default Location");
	connect(def,SIGNAL(toggled(bool)),
			this,SLOT(chooseDefault(bool)));
	pathlayout->addWidget(def);

	QSettings info;

	QHBoxLayout *custlayout=new QHBoxLayout;
	QLineEdit *le=new QLineEdit();
	connect(this,SIGNAL(locationChanged(QString)),
			le,SLOT(setText(const QString &)));
	connect(le,SIGNAL(textChanged(const QString &)),
			this,SLOT(pathChanged(const QString &)));
	//after the connection to set defaults
	le->setText(info.value("mcdir","").toString());
	custlayout->addWidget(le);
	QPushButton *browse=new QPushButton("Browse");
	connect(browse,SIGNAL(clicked(bool)),
			this,SLOT(chooseLocation(bool)));
	custlayout->addWidget(browse);
	QWidget *custom=new QWidget;
	custom->setLayout(custlayout);
	pathlayout->addWidget(custom);

	connect(def,SIGNAL(toggled(bool)),
			custom,SLOT(setDisabled(bool)));
	//after connection to set defaults
	def->setChecked(info.value("usedefault",true).toBool());

	paths->setLayout(pathlayout);
	layout->addWidget(paths);

	QCheckBox *update=new QCheckBox("Auto-check for updates");
	connect(update,SIGNAL(toggled(bool)),
			this,SLOT(autoUpdate(bool)));
	//after connection to set defaults
	update->setChecked(info.value("autoupdate",true).toBool());
	layout->addWidget(update);

	setLayout(layout);
}

QSize Settings::minimumSizeHint() const
{
	return QSize(400,150);
}
QSize Settings::sizeHint() const
{
	return QSize(400,150);
}

void Settings::autoUpdate(bool up)
{
	update=up;
	QSettings info;
	info.setValue("autoupdate",up);
	emit settingsUpdated();
}
void Settings::chooseLocation(bool)
{
	QString dirName=QFileDialog::getExistingDirectory(this,tr("Find Minecraft"));
	if (!dirName.isEmpty())
		emit locationChanged(dirName);
}
void Settings::pathChanged(const QString &path)
{
	mcpath=path;
	QSettings info;
	info.setValue("mcdir",path);
	//save settings
	emit settingsUpdated();
}
void Settings::chooseDefault(bool def)
{
	QSettings info;
	info.setValue("usedefault",def);
	if (!def) //we unchecked default.. but we're still technically default
		return;

	QString mc;
#ifdef Q_OS_MAC
	mc=QDir::homePath()+QDir::toNativeSeparators("/Library/Application Support/minecraft");
#elif defined Q_OS_WIN32
	//pretend to be minecraft
	QSettings ini(QSettings::IniFormat,QSettings::UserScope,
				  ".minecraft","minecraft1");
	mc=QFileInfo(ini.fileName()).absolutePath();
#else
	mc=QDir::homePath()+QDir::toNativeSeparators("/.minecraft");
#endif

	emit locationChanged(mc);
}
