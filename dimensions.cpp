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


#include "dimensions.h"
#include <QDirIterator>
#include <QtWidgets/QMenu>
#include "json.h"

class DimensionDef
{
public:
	DimensionDef() {}
	QString name;
	QString path;
	int scale;
	bool regex;
	bool enabled;
};

Dimensions::Dimensions()
{
	group=NULL;
}
Dimensions::~Dimensions()
{
}

void Dimensions::enableDefinitions(int pack)
{
	if (pack<0) return;
	int len=packs[pack].length();
	for (int i=0;i<len;i++)
		packs[pack][i]->enabled=true;
}
void Dimensions::disableDefinitions(int pack)
{
	if (pack<0) return;
	int len=packs[pack].length();
	for (int i=0;i<len;i++)
		packs[pack][i]->enabled=false;
}

int Dimensions::addDefinitions(JSONArray *defs,int pack)
{
	if (pack==-1)
	{
		pack=packs.length();
		packs.append(QList<DimensionDef*>());
	}

	int len=defs->length();
	for (int i=0;i<len;i++)
	{
		JSONObject *d=dynamic_cast<JSONObject *>(defs->at(i));
		DimensionDef *dim=new DimensionDef();
		dim->enabled=true;
		if (d->has("name"))
			dim->name=d->at("name")->asString();
		else
			dim->name="Unknown";
		if (d->has("path"))
			dim->path=d->at("path")->asString();
		else
			dim->path=".";
		if (d->has("scale"))
			dim->scale=d->at("scale")->asNumber();
		else
			dim->scale=1;
		if (d->has("regex"))
			dim->regex=d->at("regex")->asBool();
		else
			dim->regex=false;
		definitions.append(dim);
		packs[pack].append(dim);
	}
	return pack;
}

void Dimensions::removeDimensions(QMenu *menu)
{
	for (int i=0;i<items.count();i++)
	{
		menu->removeAction(items[i]);
		delete items[i];
	}
	items.clear();
	dimensions.clear();
	foundDimensions.clear();
	menu->setEnabled(false);
	if (group!=NULL)
	{
		delete group;
		group=NULL;
	}
}
void Dimensions::getDimensions(QDir path,QMenu *menu,QObject *parent)
{
	//first get the currently selected dimension so it doesn't change
	QString current;
	for (int i=0;i<items.length();i++)
		if (items[i]->isChecked())
			current=dimensions[i].path;
	removeDimensions(menu);
	group=new QActionGroup(parent);

	for (int i=0;i<definitions.length();i++)
	{
		if (definitions[i]->enabled)
		{
			//check path for regex
			if (definitions[i]->regex)
			{
				QDirIterator it(path.absolutePath(),QDir::Dirs);
				QRegExp rx(definitions[i]->path);
				while (it.hasNext())
				{
					it.next();
					if (rx.indexIn(it.fileName())!=-1)
					{
						QString name=definitions[i]->name;
						for (int c=0;c<rx.captureCount();c++)
							name=name.arg(rx.cap(c+1));
						addDimension(path,it.fileName(),name,definitions[i]->scale,parent);
					}
				}
			}
			else
				addDimension(path,definitions[i]->path,definitions[i]->name,definitions[i]->scale,parent);
		}
	}
	menu->addActions(items);
	if (items.count()>0)
	{
		bool changed=true;
		//locate our old selected item
		for (int i=0;i<items.length();i++)
			if (dimensions[items[i]->data().toInt()].path==current)
			{
				items[i]->setChecked(true);
				changed=false;
				break;
			}
		if (changed)
		{
			items.first()->setChecked(true);
			emit dimensionChanged(dimensions[items.first()->data().toInt()]);
		}
		menu->setEnabled(true);
	}
}

void Dimensions::addDimension(QDir path,QString dir,QString name,int scale,QObject *parent)
{
	if (!path.exists(dir))
		return;

	if (foundDimensions.contains(dir))
		return;

	path.cd(dir);
	if (path.exists("region")) //is it a used dimension?
	{
		QAction *d=new QAction(parent);
		d->setText(name);
		d->setData(dimensions.count());
		dimensions.append(Dimension(path.absolutePath(),scale));
		d->setCheckable(true);
		parent->connect(d, SIGNAL(triggered()),
						this, SLOT(viewDimension()));
		group->addAction(d);
		items.append(d);
		foundDimensions.insert(dir,true);
	}
	path.cdUp();
}

void Dimensions::viewDimension()
{
	QAction *action=qobject_cast<QAction*>(sender());
	if (action)
		emit dimensionChanged(dimensions[action->data().toInt()]);
}
