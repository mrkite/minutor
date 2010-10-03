/*
Copyright (c) 2010, Sean Kasun
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

static void draw(const char *world,int bx,int bz,int y,unsigned char *bits);
static void blit(unsigned char *block,unsigned char *bits,int px,int pz,
	double zoom,int w,int h);
static Block *LoadBlock(char *filename);
static void b36(char *dest,int num);

//world = path to world saves
//cx = center x world
//cz = center z world
//y = start depth
//w = output width
//h = output height
//zoom = zoom amount (1.0 = 100%)
//bits = byte array for output
void DrawMap(const char *world,double cx,double cz,int y,int w,int h,double zoom,unsigned char *bits)
{
	int z,x,pz,px;
	int blockScale=(int)(16*zoom);

	// number of blocks to fill the screen
	int hBlocks=(w+(blockScale-1))/blockScale;
	int vBlocks=(h+(blockScale-1))/blockScale;

	unsigned char *blockbits=(unsigned char *)malloc(16*16*4);

	// cx/cz is the center, so find the upper left corner from that
	double startx=cx-(double)w/(2*zoom);
	double startz=cz-(double)h/(2*zoom);
	int startxblock=(int)(startx/16);
	int startzblock=(int)(startz/16);
	int shiftx=(int)((startx-startxblock*16)*zoom);
	int shiftz=(int)((startz-startzblock*16)*zoom);

	if (shiftx<0)
	{
		startxblock--;
		shiftx=blockScale+shiftx;
	}
	if (shiftz<0)
	{
		startzblock--;
		shiftz=blockScale+shiftz;
	}

	for (z=0,pz=-shiftz;z<=vBlocks;z++,pz+=blockScale)
	{
		for (x=0,px=-shiftx;x<=hBlocks;x++,px+=blockScale)
		{
			draw(world,startxblock+x,startzblock+z,y,blockbits);
			blit(blockbits,bits,px,pz,zoom,w,h);
		}
	}
	free(blockbits);
}

const char *IDBlock(unsigned int color)
{
	int i;
	for (i=0;i<numBlocks;i++)
	{
		if (!blocks[i].canDraw) continue;
		if (blocks[i].color==color)
			return blocks[i].name;
		if (blocks[i].dark==color)
			return blocks[i].name;
		if (blocks[i].light==color)
			return blocks[i].name;
	}
	return "Unknown";
}

//copy block to bits at px,pz at zoom.  bits is wxh
static void blit(unsigned char *block,unsigned char *bits,int px,int pz,
	double zoom,int w,int h)
{
	int x,y,yofs,bitofs;
	int skipx=0,skipy=0;
	int bw=(int)(16*zoom);
	int bh=(int)(16*zoom);
	if (px<0) skipx=-px;
	if (px+bw>=w) bw=w-px;
	if (bw<=0) return;
	if (pz<0) skipy=-pz;
	if (pz+bh>=h) bh=h-pz;
	if (bh<=0) return;
	bits+=pz*w*4;
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


static void draw(const char *world,int bx,int bz,int y,unsigned char *bits)
{
	int first,second;
	Block *block, *prevblock;
	char filename[256];
	int ofs=0,xOfs=0,prevy,zOfs,bofs;
	int x,z,i;
	unsigned int color, watercolor = blocks[BLOCK_WATER].color, water;
	unsigned char pixel, r, g, b;

	block=(Block *)Cache_Find(bx,bz);

	if (block==NULL)
	{
        strncpy_s(filename,256,world,256);
        strncat_s(filename,256,"/",256);
        first=bx%64;
        if (first<0) first+=64;
        b36(filename,first);
        strncat_s(filename,256,"/",256);
        second=bz%64;
        if (second<0) second+=64;
        b36(filename,second);
        strncat_s(filename,256,"/c.",256);
        b36(filename,bx);
        strncat_s(filename,256,".",256);
        b36(filename,bz);
        strncat_s(filename,256,".dat",256);

		block=LoadBlock(filename);
		if (block==NULL) //blank tile
		{
			memset(bits,0xff,16*16*4);
			return;
		}

		Cache_Add(bx,bz,block);
	}

    if (block->rendery == y) //block already rendered
    {
        memcpy(bits, block->rendercache, sizeof(unsigned char)*16*16*4);
        return;
    }

    // find the block to the left, so we can use its prevy
    prevblock=(Block *)Cache_Find(bx - 1, bz);
    if (prevblock!=NULL && prevblock->rendery != y)
        prevblock = NULL; //block was rendered at a different y level, ignore

	for (x=0;x<16;x++,xOfs+=128)
	{
        if (prevblock!=NULL)
            prevy = prevblock->prevy[x];
        else
    		prevy=-1;

		zOfs=xOfs;
		for (z=0;z<16;z++,zOfs+=128*16)
		{
			bofs=zOfs+y;
			color=0;
            water=0;
			for (i=y;i>=0;i--,bofs--)
			{
				pixel=block->grid[bofs];
                if (pixel==BLOCK_STATIONARY_WATER || pixel==BLOCK_WATER) 
                {
                    if (++water < 8)
                        continue;
                }
                if (pixel<numBlocks && blocks[pixel].canDraw)
				{
					if (prevy==-1) prevy=i;
					if (prevy<i)
						color=blocks[pixel].light;
					else if (prevy>i)
						color=blocks[pixel].dark;
					else
						color=blocks[pixel].color;

                    if (water != 0) {
                        r=(color>>16)/(water + 1) + (watercolor>>16)*water/(water + 1);
                        g=(color>>8&0xff)/(water + 1) + (watercolor>>8&0xff)*water/(water + 1);
                        b=(color&0xff)/(water + 1) + (watercolor&0xff)*water/(water + 1);
                        color = r<<16 | g<<8 | b;
                    }

					prevy=i;
					break;
				}
			}
			bits[ofs++]=color>>16;
			bits[ofs++]=color>>8;
			bits[ofs++]=color;
			bits[ofs++]=0xff;
		}

        block->prevy[x] = prevy;
	}

    block->rendery = y;
    memcpy(block->rendercache, bits, sizeof(unsigned char)*16*16*4);
}
Block *LoadBlock(char *filename)
{
	gzFile gz=newNBT(filename);
    Block *block=malloc(sizeof(Block));
    block->rendery = -1;
	if (nbtGetBlocks(gz, block->grid) == NULL) {
        free(block);
        block = NULL;
    }
	nbtClose(gz);
	return block;
}

void GetSpawn(const char *world,int *x,int *y,int *z)
{
	gzFile gz;
	char filename[256];
	strncpy_s(filename,256,world,256);
	strncat_s(filename,256,"/level.dat",256);
	gz=newNBT(filename);
	nbtGetSpawn(gz,x,y,z);
	nbtClose(gz);
}

static void b36(char *dest,int num)
{
	int lnum,i;
	int pos=strlen(dest);
	int len=0;
	for (lnum=num;lnum;len++)
		lnum/=36;
	if (num<0)
	{
		num=-num;
		dest[pos++]='-';
	}
	for (i=0;num;i++)
	{
		dest[pos+len-i-1]="0123456789abcdefghijklmnopqrstuvwxyz"[num%36];
		num/=36;
	}
	if (len==0)
		dest[pos++]='0';
	dest[pos+len]=0;
}
