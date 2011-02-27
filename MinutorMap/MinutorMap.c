/*
Copyright (c) 2010, Sean Kasun
	Parts Copyright (c) 2010, Ryan Hitchman
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


// MinutorMap.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "blockInfo.h"
#include <string.h>

static void draw(const char *world,int bx,int bz,int y,int opts,unsigned char *bits,ProgressCallback callback,float percent);
static void blit(unsigned char *block,unsigned char *bits,int px,int py,
        double zoom,int w,int h);
static Block *LoadBlock(char *directory,int bx,int bz);
static void initColors();

static int colorsInited=0;
static unsigned int blockColors[256*16];

static unsigned short colormap=0;

#define clamp(v,a,b) ((v)<b?((v)>a?(v):a):b)


//world = path to world saves
//cx = center x world
//cz = center z world
//y = start depth
//w = output width
//h = output height
//zoom = zoom amount (1.0 = 100%)
//bits = byte array for output
//opts = bitmask of render options (see MinutorMap.h)
void DrawMap(const char *world,double cx,double cz,int y,int w,int h,double zoom,unsigned char *bits,int opts,ProgressCallback callback)
{
    /* We're converting between coordinate systems, so this gets kinda ugly 
     *
     * X     -world x N  -screen y
     * screen origin  |
     *                |
     *                |
     *                |
     *  +world z      |(cz,cx)   -world z
     * W--------------+----------------E
     *  -screen x     |          +screen x
     *                |
     *                | 
     *                |
     *      +world x  | +screen y
     *                S
     */

	unsigned char blockbits[16*16*4];
	int z,x,px,py;
	int blockScale=(int)(16*zoom);

	// number of blocks to fill the screen (plus 2 blocks for floating point inaccuracy)
	int hBlocks=(w+blockScale*2)/blockScale;
	int vBlocks=(h+blockScale*2)/blockScale;


	// cx/cz is the center, so find the upper left corner from that
	double startx=cx-(double)h/(2*zoom);
	double startz=cz+(double)w/(2*zoom);
	int startxblock=(int)(startx/16);
	int startzblock=(int)(startz/16);
	int shiftx=(int)(blockScale-(startz-startzblock*16)*zoom);
	int shifty=(int)((startx-startxblock*16)*zoom);

	if (shiftx<0)
	{
		startzblock++;
		shiftx+=blockScale;
	}
	if (shifty<0)
	{
		startxblock--;
		shifty+=blockScale;
	}

	if (!colorsInited)
		initColors();

    // x increases south, decreases north
    for (x=0,py=-shifty;x<=vBlocks;x++,py+=blockScale)
    {
        // z increases west, decreases east
        for (z=0,px=-shiftx;z<=hBlocks;z++,px+=blockScale)
        {
			draw(world,startxblock+x,startzblock-z,y,opts,blockbits,callback,(float)(x*hBlocks+z)/(float)(vBlocks*hBlocks));
			blit(blockbits,bits,px,py,zoom,w,h);
		}
	}
}

//bx = x coord of pixel
//by = y coord of pixel
//cx = center x world
//cz = center z world
//w = output width
//h = output height
//zoom = zoom amount (1.0 = 100%)
//ox = world x at mouse
//oz = world z at mouse
const char *IDBlock(int bx, int by, double cx, double cz, int w, int h, double zoom,int *ox,int *oz)
{
	//WARNING: keep this code in sync with draw()
	Block *block;
    int x,y,z,px,py,xoff,zoff;
	int blockScale=(int)(16*zoom);
    
	// cx/cz is the center, so find the upper left corner from that
	double startx=cx-(double)h/(2*zoom);
	double startz=cz+(double)w/(2*zoom);
	int startxblock=(int)(startx/16);
	int startzblock=(int)(startz/16);
	int shiftx=(int)(blockScale-(startz-startzblock*16)*zoom);
	int shifty=(int)((startx-startxblock*16)*zoom);

	if (shiftx<0)
	{
		startzblock++;
		shiftx+=blockScale;
	}
	if (shifty<0)
	{
		startxblock--;
		shifty+=blockScale;
	}

    x=(by+shifty)/blockScale;
    py=x*blockScale-shifty;
    z=(bx+shiftx)/blockScale;
    px=z*blockScale-shiftx;

    zoff=((int)((px - bx)/zoom) + 15) % 16;
    xoff=(int)((by - py)/zoom);

	*ox=(startxblock+x)*16+xoff;
	*oz=(startzblock-z)*16+zoff;

    block=(Block *)Cache_Find(startxblock+x, startzblock-z);

    if (block==NULL)
        return "Unknown";

    y=block->heightmap[xoff+zoff*16];

    if (y == (unsigned char)-1) 
        return "Empty";  // nothing was rendered here

    return blocks[block->grid[y+(zoff+xoff*16)*128]].name;
}


//copy block to bits at px,py at zoom.  bits is wxh
static void blit(unsigned char *block,unsigned char *bits,int px,int py,
	double zoom,int w,int h)
{
	int x,y,yofs,bitofs;
	int skipx=0,skipy=0;
	int bw=(int)(16*zoom);
	int bh=(int)(16*zoom);
	if (px<0) skipx=-px;
	if (px+bw>=w) bw=w-px;
	if (bw<=0) return;
	if (py<0) skipy=-py;
	if (py+bh>=h) bh=h-py;
	if (bh<=0) return;
	bits+=py*w*4;
	bits+=px*4;
	for (y=0;y<bh;y++,bits+=w<<2)
	{
		if (y<skipy) continue;
		yofs=((int)(y/zoom))<<6;
		bitofs=0;
		for (x=0;x<bw;x++,bitofs+=4)
		{
			if (x<skipx) continue;
			memcpy(bits+bitofs,block+yofs+(((int)(x/zoom))<<2),4);
		}
	}
}

void CloseAll()
{
	Cache_Empty();
}



// opts is a bitmask representing render options (see MinutorMap.h)
static void draw(const char *world,int bx,int bz,int y,int opts,unsigned char *bits,ProgressCallback callback,float percent)
{
    Block *block, *prevblock;
    int ofs=0,xOfs=0,prevy,zOfs,bofs;
    int x,z,i;
    unsigned int color;
    unsigned char pixel, r, g, b, seenempty;
    double alpha;

    char cavemode, showobscured, depthshading, lighting;

	cavemode=!!(opts&CAVEMODE);
    showobscured=!(opts&HIDEOBSCURED);
    depthshading=!!(opts&DEPTHSHADING);
    lighting=!!(opts&LIGHTING);

	block=(Block *)Cache_Find(bx,bz);

    if (block==NULL)
    {
        char directory[256];
        strncpy_s(directory,255,world,255);
        strncat_s(directory,255,"/",255);
        if (opts&HELL)
        {
            strncat_s(directory,255,"DIM-1/",255);
        }

        block=LoadBlock(directory,bx,bz);
        if (block==NULL) //blank tile
        {
            memset(bits,0xff,16*16*4);
            return;
        }
        //lets only update the progress bar if we're loading
        if (callback)
            callback(percent);

		Cache_Add(bx,bz,block);
	}

    if (block->rendery==y && block->renderopts==opts && block->colormap==colormap) // already rendered
    { 
        if (block->rendermissing // wait, the last render was incomplete
            && Cache_Find(bx, bz+block->rendermissing) != NULL) {
          ; // we can do a better render now that the missing block is loaded
        } else {
          // there's no need to re-render, use cache
          memcpy(bits, block->rendercache, sizeof(unsigned char)*16*16*4);
          return;
        }
    }

    block->rendery=y;
    block->renderopts=opts;
    block->rendermissing=0;
	block->colormap=colormap;

    // find the block to the west, so we can use its heightmap for shading
    prevblock=(Block *)Cache_Find(bx, bz + 1);

    if (prevblock==NULL)
        block->rendermissing=1; //note no loaded block to west
    else if (prevblock->rendery!=y || prevblock->renderopts!=opts) {
        block->rendermissing=1; //note improperly rendered block to west
        prevblock = NULL; //block was rendered at a different y level, ignore
    }
    // x increases south, decreases north
	for (x=0;x<16;x++,xOfs+=128*16)
	{
        if (prevblock!=NULL)
            prevy = prevblock->heightmap[x];
        else
    		prevy=-1;

		zOfs=xOfs+128*15;

        // z increases west, decreases east
		for (z=15;z>=0;z--,zOfs-=128)
		{
			bofs=zOfs+y;
			color=0;
			r=g=b=0;
            seenempty=0;
			alpha=0.0;
			for (i=y;i>=0;i--,bofs--)
			{
				pixel=block->grid[bofs];
                if (pixel==BLOCK_AIR)
                {
                    seenempty=1;
                    continue;
                }
                if (pixel==BLOCK_STATIONARY_WATER)
                    seenempty=1;
                if ((showobscured || seenempty) && pixel<numBlocks && blocks[pixel].alpha!=0.0)
				{
					int light=12;
					if (lighting && i < 127)
					{
						light=block->light[(bofs+1)/2];
						if (!(bofs&1)) light>>=4;
						light&=0xf;
					}
					if (prevy==-1) prevy=i;
					if (prevy<i)
						light+=2;
					else if (prevy>i)
						light-=5;
					light=clamp(light,1,15);
					color=blockColors[pixel*16+light];
					if (alpha==0.0)
					{
						alpha=blocks[pixel].alpha;
						r=color>>16;
						g=(color>>8)&0xff;
						b=color&0xff;
					}
					else
					{
							r+=(unsigned char)((1.0-alpha)*(color>>16));
							g+=(unsigned char)((1.0-alpha)*((color>>8)&0xff));
							b+=(unsigned char)((1.0-alpha)*(color&0xff));
							alpha+=blocks[pixel].alpha*(1.0-alpha);
					}
					if (blocks[pixel].alpha==1.0)
						break;
				}
			}

			prevy=i;

            if (depthshading) // darken deeper blocks
            {
                int num=prevy+50-(128-y)/5;
                int denom=y+50-(128-y)/5;

				r=r*num/denom;
                g=g*num/denom;
                b=b*num/denom;
            }

            if (cavemode)
			{
                seenempty=0;
                pixel=block->grid[bofs];

                if (pixel==BLOCK_LEAVES || pixel==BLOCK_WOOD) //special case surface trees
                    for (; i>=1; i--,pixel=block->grid[--bofs])
                        if (!(pixel==BLOCK_WOOD||pixel==BLOCK_LEAVES||pixel==BLOCK_AIR))
                            break; // skip leaves, wood, air

                for (;i>=1;i--,bofs--)
                {
                    pixel=block->grid[bofs];
                    if (pixel==BLOCK_AIR)
                    {
                        seenempty=1;
                        continue;
                    }
                    if (seenempty && pixel<numBlocks && blocks[pixel].alpha!=0.0)
                    {
                        r=r*(prevy-i+10)/138;
                        g=g*(prevy-i+10)/138;
                        b=b*(prevy-i+10)/138; 
                        break;
                    }
                }
            }

			bits[ofs++]=r;
			bits[ofs++]=g;
			bits[ofs++]=b;
			bits[ofs++]=0xff;

            block->heightmap[x+z*16] = prevy;
		}
	}
    memcpy(block->rendercache, bits, sizeof(unsigned char)*16*16*4);
}
Block *LoadBlock(char *directory, int cx, int cz)
{
    Block *block=malloc(sizeof(Block));
    block->rendery = -1; // force redraw

    if (regionGetBlocks(directory, cx, cz, block->grid, block->light)) {
        return block;
    }

    free(block);
    return NULL;
}

void GetSpawn(const char *world,int *x,int *y,int *z)
{
	bfFile bf;
	char filename[256];
	strncpy_s(filename,256,world,256);
	strncat_s(filename,256,"/level.dat",256);
	bf=newNBT(filename);
	nbtGetSpawn(bf,x,y,z);
	nbtClose(bf);
}
void GetPlayer(const char *world,int *px,int *py,int *pz)
{
	bfFile bf;
	char filename[256];
	strncpy_s(filename,256,world,256);
	strncat_s(filename,256,"/level.dat",256);
	bf=newNBT(filename);
	nbtGetPlayer(bf,px,py,pz);
	nbtClose(bf);
}

//palette should be in RGBA format
void SetMapPalette(unsigned int *palette,int num)
{
	unsigned char r,g,b;
	double a;
	int i;
	
	colormap++;
	for (i=0;i<num;i++)
	{
		r=palette[i]>>24;
		g=palette[i]>>16;
		b=palette[i]>>8;
		a=((double)(palette[i]&0xff))/255.0;
		r=(unsigned char)(r*a); //premultiply alpha
		g=(unsigned char)(g*a);
		b=(unsigned char)(b*a);
		blocks[i].color=(r<<16)|(g<<8)|b;
		blocks[i].alpha=a;
	}
	initColors();
}

// for each block color, calculate light levels 0-15
static void initColors()
{
	unsigned r,g,b,i,shade;
	double y,u,v,delta;
	unsigned int color;
	colorsInited=1;
	for (i=0;i<numBlocks;i++)
	{
		color=blocks[i].color;
		r=color>>16;
		g=(color>>8)&0xff;
		b=color&0xff;
		//we'll use YUV to darken the blocks.. gives a nice even
		//coloring
		y=0.299*r+0.587*g+0.114*b;
		u=(b-y)*0.565;
		v=(r-y)*0.713;
		delta=y/15;

		for (shade=0;shade<16;shade++)
		{
			y=shade*delta;
			r=(unsigned int)clamp(y+1.403*v,0,255);
			g=(unsigned int)clamp(y-0.344*u-0.714*v,0,255);
			b=(unsigned int)clamp(y+1.770*u,0,255);
			blockColors[i*16+shade]=(r<<16)|(g<<8)|b;
		}
	}
}
