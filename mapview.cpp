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
}

void MapView::setDepth(int depth)
{
	this->depth=depth;
	redraw();
}

void MapView::setFlags(int flags)
{
	this->flags=flags;
	redraw();
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
void MapView::wheelEvent(QWheelEvent *event)
{
	zoom+=floor(event->delta()/90.0);
	if (zoom<1.0) zoom=1.0;
	if (zoom>20.0) zoom=20.0;
	redraw();
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
	case Qt::Key_BracketLeft:
		break;
	case Qt::Key_End:
	case Qt::Key_BracketRight:
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

	for (int z=startz;z<startz+blockstall;z++)
		for (int x=startx;x<startx+blockswide;x++)
			drawChunk(x,z);
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
		renderChunk(chunk);

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
				int yoffset=(y&0xf)<<8;
				int data=section->data[(offset+yoffset)/2];
				if (x&1) data>>=4;
				BlockInfo &block=blocks->getBlock(section->blocks[offset+yoffset],data&0xf);
				if (block.alpha==0.0) continue;
				int light=12;
				if (flags&1) //use lighting
				{
					// light values are always the block above
					light=0;
					ChunkSection *lsec=chunk->sections[(y+1)>>4];
					if (lsec)
						light=lsec->light[(offset+(((y+1)&0xf)<<8))/2];
					if (x&1) light>>=4;
					light&=15;
				}
				if (alpha==0.0)
				{
					if (lasty!=-1 && lasty<y)
						light+=2;
					else if (lasty>y)
						light-=2;
				}
				if (light<0) light=0;
				if (light>15) light=15;
				quint32 color=block.colors[light];
				if (alpha==0.0)
				{
					alpha=block.alpha;
					r=color>>16;
					g=(color>>8)&0xff;
					b=color&0xff;
					highest=y;
				}
				else
				{
					r+=(quint8)((1.0-alpha)*(color>>16));
					g+=(quint8)((1.0-alpha)*((color>>8)&0xff));
					b+=(quint8)((1.0-alpha)*(color&0xff));
					alpha+=block.alpha*(1.0-alpha);
				}
                if (block.alpha==1.0 || alpha>0.9)
					break;
			}
			lasty=highest;
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

	if (chunk)
	{
		int top=depth;
		if (top>chunk->highest)
			top=chunk->highest;
		for (int y=top;y>=0;y--)
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
	}
	emit hoverTextChanged(tr("X:%1 Z:%2 - %3 - %4 (%5:%6)")
						  .arg(x)
						  .arg(z)
						  .arg(biome)
						  .arg(name)
						  .arg(id)
						  .arg(bd));
}
