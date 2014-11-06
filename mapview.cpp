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


#include "mapview.h"
#include "definitionmanager.h"
#include "blockidentifier.h"
#include "biomeidentifier.h"
#include <QPainter>
#include <QResizeEvent>
#include <QMessageBox>

#include <assert.h>

MapView::MapView(QWidget *parent) : QWidget(parent)
{
	depth=255;
	scale=1;
	zoom=1.0;
	connect(&cache, SIGNAL(chunkLoaded(int,int)),
			this, SLOT(chunkUpdated(int,int)));
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);

	int offset=0;
	for (int y=0;y<16;y++)
		for (int x=0;x<16;x++)
		{
			uchar color=((x&8)^(y&8))==0?0x44:0x88;
			placeholder[offset++]=color;
			placeholder[offset++]=color;
			placeholder[offset++]=color;
			placeholder[offset++]=0xff;
		}
}

QSize MapView::minimumSizeHint() const
{
	return QSize(300,300);
}
QSize MapView::sizeHint() const
{
	return QSize(400,400);
}

void MapView::attach(DefinitionManager *dm)
{
	this->dm=dm;
	connect(dm,SIGNAL(packsChanged()),
			this,SLOT(redraw()));
	this->blocks=dm->blockIdentifier();
	this->biomes=dm->biomeIdentifier();
}

void MapView::setLocation(double x, double z)
{
	this->x=x/scale;
	this->z=z/scale;

	redraw();
}

void MapView::setDimension(QString path, int scale)
{
	if (scale>0)
	{
		this->x*=this->scale;
		this->z*=this->scale; //undo current scale transform
		this->scale=scale;
		this->x/=scale; //and do the new scale transform
		this->z/=scale;
	}
	else
	{
		this->scale=1; //no scaling because no relation to overworld
		this->x=0; //and we jump to the center spawn automatically
		this->z=0;
	}
	cache.clear();
	cache.setPath(path);
	redraw();

	//we should eventually detect these instead of pre-loading them
	//for now, keep these in sync with the entities created in Entity.cpp,
	//or else we won't be able to select them.
	emit addOverlayItemType("Entity.Hostile", Qt::red);
	emit addOverlayItemType("Entity.Passive", Qt::white);
	emit addOverlayItemType("Entity.Neutral", Qt::blue);
}

void MapView::setDepth(int depth)
{
	this->depth=depth;
	redraw();
}

void MapView::setFlags(int flags)
{
	this->flags=flags;
}

void MapView::chunkUpdated(int x, int z)
{
	drawChunk(x,z);
	update();
}

QString MapView::getWorldPath()
{
	return cache.getPath();
}

void MapView::clearCache()
{
	cache.clear();
	//TODO: clear overlay items?
	redraw();
}

static int lastX=-1,lastY=-1;
static bool dragging=false;
void MapView::mousePressEvent(QMouseEvent *event)
{
	lastX=event->x();
	lastY=event->y();
	dragging=true;
}

void MapView::mouseMoveEvent(QMouseEvent *event)
{
	if (!dragging)
	{
		int centerblockx=floor(this->x);
		int centerblockz=floor(this->z);

		int centerx=image.width()/2;
		int centery=image.height()/2;

		centerx-=(this->x-centerblockx)*zoom;
		centery-=(this->z-centerblockz)*zoom;

		int mx=floor(centerblockx-(centerx-event->x())/zoom);
		int mz=floor(centerblockz-(centery-event->y())/zoom);

		getToolTip(mx,mz);
		return;
	}
	x+=(lastX-event->x())/zoom;
	z+=(lastY-event->y())/zoom;
	lastX=event->x();
	lastY=event->y();
	redraw();
}
void MapView::mouseReleaseEvent(QMouseEvent *)
{
	dragging=false;
}

void MapView::mouseDoubleClickEvent(QMouseEvent*event)
{
	int centerblockx=floor(this->x);
	int centerblockz=floor(this->z);

	int centerx=image.width()/2;
	int centery=image.height()/2;

	centerx-=(this->x-centerblockx)*zoom;
	centery-=(this->z-centerblockz)*zoom;

	int mx=floor(centerblockx-(centerx-event->x())/zoom);
	int mz=floor(centerblockz-(centery-event->y())/zoom);

	//get the y coordinate
	int my = getY(mx, mz);

	QList<QVariant> properties;
	foreach(const QSharedPointer<OverlayItem>& item, getItems(mx, my, mz))
	{
		properties.append(item->properties());
	}

	if (!properties.isEmpty())
	{
		emit showProperties(properties);
	}
}

void MapView::wheelEvent(QWheelEvent *event)
{
    if ((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier) // change depth
	{
		emit demandDepthChange(event->delta()/120);
	}
	else          // change zoom
	{
		zoom+=floor(event->delta()/90.0);
		if (zoom<1.0) zoom=1.0;
		if (zoom>20.0) zoom=20.0;
		redraw();
	}
}
void MapView::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_Up:
	case Qt::Key_W:
		z-=10.0/zoom;
		redraw();
		break;
	case Qt::Key_Down:
	case Qt::Key_S:
		z+=10.0/zoom;
		redraw();
		break;
	case Qt::Key_Left:
	case Qt::Key_A:
		x-=10.0/zoom;
		redraw();
		break;
	case Qt::Key_Right:
	case Qt::Key_D:
		x+=10.0/zoom;
		redraw();
		break;
	case Qt::Key_PageUp:
	case Qt::Key_Q:
		zoom++;
		if (zoom>20.0) zoom=20.0;
		redraw();
		break;
	case Qt::Key_PageDown:
	case Qt::Key_E:
		zoom--;
		if (zoom<1.0) zoom=1.0;
		redraw();
		break;
	case Qt::Key_Home:
	case Qt::Key_Plus:
	case Qt::Key_BracketLeft:
		emit demandDepthChange(+1);
		break;
	case Qt::Key_End:
	case Qt::Key_Minus:
	case Qt::Key_BracketRight:
		emit demandDepthChange(-1);
		break;
	}
}

void MapView::resizeEvent(QResizeEvent *event)
{
	image=QImage(event->size(),QImage::Format_RGB32);
	redraw();
}
void MapView::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.drawImage(QPoint(0,0),image);
	p.end();
}

void MapView::redraw()
{
	if (!this->isEnabled())
	{
		// blank
		uchar *bits=image.bits();
		int imgstride=image.bytesPerLine();
		int imgoffset=0;
		for (int y=0;y<image.height();y++,imgoffset+=imgstride)
			memset(bits+imgoffset,0xee,imgstride);
		update();
		return;
	}

	double chunksize=16*zoom;

	//first find the center block position
	int centerchunkx=floor(x/16);
	int centerchunkz=floor(z/16);
	// and the center of the screen
	int centerx=image.width()/2;
	int centery=image.height()/2;
	// and align for panning
	centerx-=(x-centerchunkx*16)*zoom;
	centery-=(z-centerchunkz*16)*zoom;
	// now calculate the topleft block on the screen
	int startx=centerchunkx-floor(centerx/chunksize)-1;
	int startz=centerchunkz-floor(centery/chunksize)-1;
	// and the dimensions of the screen in blocks
	int blockswide=image.width()/chunksize+3;
	int blockstall=image.height()/chunksize+3;

	for (int cz=startz;cz<startz+blockstall;cz++)
		for (int cx=startx;cx<startx+blockswide;cx++)
			drawChunk(cx,cz);

	//add on the entity layer
	QPainter canvas(&image);
	double halfviewwidth = image.width()/2/zoom;
	double halvviewheight = image.height()/2/zoom;
	double x1 = x - halfviewwidth;
	double z1 = z - halvviewheight;
	double x2 = x + halfviewwidth;
	double z2 = z + halvviewheight;

	//draw the entities
	for (int cz=startz;cz<startz+blockstall;cz++)
	{
		for (int cx=startx;cx<startx+blockswide;cx++)
		{
			foreach(const QString& type, overlayItemTypes)
			{
				Chunk *chunk=cache.fetch(cx,cz);
				if (chunk)
				{
					QPair<Chunk::EntityMap::const_iterator, Chunk::EntityMap::const_iterator> range = chunk->entities.equal_range(type);
					for (Chunk::EntityMap::const_iterator it = range.first; it != range.second; ++it)
					{
						//don't show entities above our depth
						if ((*it)->midpoint().y < depth)

						(*it)->draw(x1, z1, zoom, canvas);
					}
				}
			}
		}
	}


	//draw the generated structures
	foreach(const QString& type, overlayItemTypes)
	{
		foreach(const QSharedPointer<OverlayItem>& item, overlayItems[type])
		{
			if (item->intersects(
				OverlayItem::Point(x1-1, 0, z1-1),
				OverlayItem::Point(x2+1, depth, z2+1)))
			{
				item->draw(x1, z1, zoom, canvas);
			}
		}
	}

	update();
}
void MapView::drawChunk(int x, int z)
{
	if (!this->isEnabled())
		return;

	uchar *src=placeholder;
	//fetch the chunk
	Chunk *chunk=cache.fetch(x,z);

	if (chunk && (chunk->renderedAt!=depth || chunk->renderedFlags!=flags))
	{
		renderChunk(chunk);
	}

	//this figures out where on the screen this chunk should be drawn


	// first find the center chunk
	int centerchunkx=floor(this->x/16);
	int centerchunkz=floor(this->z/16);
	// and the center chunk screen coordinates
	int centerx=image.width()/2;
	int centery=image.height()/2;
	// which need to be shifted to account for panning inside that chunk
	centerx-=(this->x-centerchunkx*16)*zoom;
	centery-=(this->z-centerchunkz*16)*zoom;
	// centerx,y now points to the top left corner of the center chunk
	// so now calculate our x,y in relation
	double chunksize=16*zoom;
	centerx+=(x-centerchunkx)*chunksize;
	centery+=(z-centerchunkz)*chunksize;

	int srcoffset=0;
	uchar *bits=image.bits();
	int imgstride=image.bytesPerLine();

	int skipx=0,skipy=0;
	int blockwidth=chunksize,blockheight=chunksize;
	// now if we're off the screen we need to crop
	if (centerx<0)
	{
		skipx=-centerx;
		centerx=0;
	}
	if (centery<0)
	{
		skipy=-centery;
		centery=0;
	}
	// or the other side, we need to trim
	if (centerx+blockwidth>image.width())
		blockwidth=image.width()-centerx;
	if (centery+blockheight>image.height())
		blockheight=image.height()-centery;
	if (blockwidth<=0 || skipx>=blockwidth) return;
	int imgoffset=centerx*4+centery*imgstride;
	if (chunk)
		src=chunk->image;
	//blit (or scale blit)
	for (int z=skipy;z<blockheight;z++,imgoffset+=imgstride)
	{
		srcoffset=floor(z/zoom)*16*4;
		if (zoom==1.0)
			memcpy(bits+imgoffset,src+srcoffset+skipx*4,(blockwidth-skipx)*4);
		else
		{
			int xofs=0;
			for (int x=skipx;x<blockwidth;x++,xofs+=4)
				memcpy(bits+imgoffset+xofs,src+srcoffset+(int)floor(x/zoom)*4,4);
		}
	}
}

void MapView::renderChunk(Chunk *chunk)
{
	int offset=0;
	uchar *bits=chunk->image;
	uchar *depthbits=chunk->depth;
	for (int z=0;z<16;z++) //n->s
	{
		int lasty=-1;
		for (int x=0;x<16;x++,offset++) //e->w
		{
			uchar r=0,g=0,b=0;
			double alpha=0.0;
			int top=depth;
			if (top>chunk->highest)
				top=chunk->highest;
			int highest=0;
			for (int y=top;y>=0;y--)
			{
				int sec=y>>4;
				ChunkSection *section=chunk->sections[sec];
				if (!section)
				{
					y=(sec<<4)-1; //skip whole section
					continue;
				}

				// get data value
				int data = section->getData(x,y,z);

				// get BlockInfo from block value
				BlockInfo &block=blocks->getBlock(section->getBlock(x,y,z),data);
				if (block.alpha==0.0) continue;

				// get light value from one block above
				int light = 0;
				ChunkSection *section1=NULL;
				if (y<255)
					section1 = chunk->sections[(y+1)>>4];
				if (section1)
					light = section1->getLight(x,y+1,z);
				int light1 = light;
				if (!(flags & flgLighting))
					light = 13;
				if ((alpha==0.0)&&(lasty!=-1))
				{
					if (lasty<y)
						light+=2;
					else if (lasty>y)
						light-=2;
				}
				if (light<0) light=0;
				if (light>15) light=15;

				// define the color
				quint32 color=block.colors[light];
				quint32 colr = color >> 16;
				quint32 colg = (color >> 8) & 0xff;
				quint32 colb = color & 0xff;
				if (flags & flgDepthShading)
				{
					colr = colr * (256 - (depth - y)) / 256;
					colg = colg * (256 - (depth - y)) / 256;
					colb = colb * (256 - (depth - y)) / 256;
				}
				if (flags & flgMobSpawn)
				{
					// get block info from 1 and 2 above and 1 below
					quint16 blid1(0), blid2(0), blidB(0); // default to air
					int	 data1(0), data2(0), dataB(0); // default variant
					ChunkSection *section2=NULL;
					ChunkSection *sectionB=NULL;
					if (y<254)
						section2 = chunk->sections[(y+2)>>4];
					if (y>0)
						sectionB = chunk->sections[(y-1)>>4];
					if (section1)
					{
						blid1 = section1->getBlock(x,y+1,z);
						data1 = section1->getData(x,y+1,z);
					}
					if (section2)
					{
						blid2 = section2->getBlock(x,y+2,z);
						data2 = section2->getData(x,y+2,z);
					}
					if (sectionB)
					{
						blidB = sectionB->getBlock(x,y-1,z);
						dataB = sectionB->getData(x,y-1,z);
					}
					BlockInfo &block2=blocks->getBlock(blid2,data2);
					BlockInfo &block1=blocks->getBlock(blid1,data1);
					BlockInfo &block0=block;
					BlockInfo &blockB=blocks->getBlock(blidB,dataB);
					int light0 = section->getLight(x,y,z);

					// spawn check #1: on top of solid block
					if ( block0.doesBlockHaveSolidTopSurface(data) &&
						 !block0.name.contains("Bedrock") &&
						 (light1<8) &&
						 !block1.isBlockNormalCube() && block1.spawninside && !block1.isLiquid() &&
						 !block2.isBlockNormalCube() && block2.spawninside )
					{
						colr = (colr+256)/2;
						colg = (colg+0)/2;
						colb = (colb+192)/2;
					}
					// spawn check #2: current block is transparent, but mob can spawn through (e.g. snow)
					if ( blockB.doesBlockHaveSolidTopSurface(dataB) &&
						 !blockB.name.contains("Bedrock") &&
						 (light0<8) &&
						 !block0.isBlockNormalCube() && block0.spawninside && !block0.isLiquid() &&
						 !block1.isBlockNormalCube() && block1.spawninside )
					{
						colr = (colr+192)/2;
						colg = (colg+0)/2;
						colb = (colb+256)/2;
					}
				}
				if (alpha==0.0)
				{
					alpha = block.alpha;
					r = colr;
					g = colg;
					b = colb;
					highest = y;
				}
				else
				{
					r =(quint8)(alpha*r + (1.0-alpha)*(colr));
					g =(quint8)(alpha*g + (1.0-alpha)*(colg));
					b =(quint8)(alpha*b + (1.0-alpha)*(colb));
					alpha+=block.alpha*(1.0-alpha);
				}

				// finish deepth (Y) scanning when color is saturated enough
				if (block.alpha==1.0 || alpha>0.9)
					break;
			}
			*depthbits++=lasty=highest;
			*bits++=b;
			*bits++=g;
			*bits++=r;
			*bits++=0xff;
		}
	}
	chunk->renderedAt=depth;
	chunk->renderedFlags=flags;
}

void MapView::getToolTip(int x, int z)
{
	int cx=floor(x/16.0);
	int cz=floor(z/16.0);
	Chunk *chunk=cache.fetch(cx,cz);
	int offset=(x&0xf)+(z&0xf)*16;
	int id=0,bd=0;

	QString name="Unknown";
	QString biome="Unknown Biome";
	QMap<QString, int> entityIds;

	if (chunk)
	{
		int top=depth;
		if (top>chunk->highest)
			top=chunk->highest;
		int y = 0;
		for (y=top;y>=0;y--)
		{
			int sec=y>>4;
			ChunkSection *section=chunk->sections[sec];
			if (!section)
			{
				y=(sec<<4)-1; //skip entire section
				continue;
			}
			int yoffset=(y&0xf)<<8;
			int data=section->data[(offset+yoffset)/2];
			if (x&1) data>>=4;
			BlockInfo &block=blocks->getBlock(section->blocks[offset+yoffset],data&0xf);
			if (block.alpha==0.0) continue;
			//found block
			name=block.name;
			id=section->blocks[offset+yoffset];
			bd=data&0xf;
			break;
		}
		BiomeInfo &bi=biomes->getBiome(chunk->biomes[(x&0xf)+(z&0xf)*16]);
		biome=bi.name;

		foreach (const QSharedPointer<OverlayItem>& item, getItems(x, y, z))
		{
			entityIds[item->display()]++;
		}
	}

	QString entityStr;
	if (!entityIds.empty())
	{
		QStringList entities;
		QMap<QString, int>::iterator it, itEnd = entityIds.end();
		for (it = entityIds.begin(); it != itEnd; ++it)
		{
			if (it.value() > 1)
			{
				entities << it.key() + ":" + QString::number(it.value());
			}
			else
			{
				entities << it.key();
			}
		}
		entityStr = entities.join(", ");
	}

	emit hoverTextChanged(tr("X:%1 Z:%2 - %3 - %4 (%5:%6) %7")
						  .arg(x)
						  .arg(z)
						  .arg(biome)
						  .arg(name)
						  .arg(id)
						  .arg(bd)
						  .arg(entityStr));
}

void MapView::addOverlayItem(QSharedPointer<OverlayItem> item)
{
	overlayItems[item->type()].push_back(item);
}

void MapView::showOverlayItemTypes(const QSet<QString>& itemTypes)
{
	overlayItemTypes = itemTypes;
}

int MapView::getY(int x, int z)
{
	int cx=floor(x/16.0);
	int cz=floor(z/16.0);
	Chunk *chunk=cache.fetch(cx,cz);
	return chunk ? chunk->depth[(x&0xf)+(z&0xf)*16] : -1;
}

QList<QSharedPointer<OverlayItem> > MapView::getItems(int x, int y, int z)
{
	QList<QSharedPointer<OverlayItem> > ret;
	int cx=floor(x/16.0);
	int cz=floor(z/16.0);
	Chunk *chunk=cache.fetch(cx,cz);

	if (chunk)
	{
		double invzoom = 10.0/zoom;
		foreach (const QString& type, overlayItemTypes)
		{
			//generated structures
			foreach (const QSharedPointer<OverlayItem>& item, overlayItems[type])
			{
				double ymin = 0;
				double ymax = depth;
				//TODO: handle depth?
				if (item->intersects(
					OverlayItem::Point(x, ymin, z),
					OverlayItem::Point(x, ymax, z)))
				{
					ret.append(item);
				}
			}

			//entities
			QPair<Chunk::EntityMap::const_iterator, Chunk::EntityMap::const_iterator> itemRange = chunk->entities.equal_range(type);
			for(Chunk::EntityMap::const_iterator itItem = itemRange.first; itItem != itemRange.second; ++itItem)
			{
				double ymin = y - 4;
				double ymax = depth + 4;

				if ((*itItem)->intersects(
					OverlayItem::Point(x - invzoom/2, ymin, z - invzoom/2),
					OverlayItem::Point(x + 1 + invzoom/2, ymax, z + 1 + invzoom/2)))
				{
					ret.append(*itItem);
				}
			}
		}
	}
	return ret;
}
