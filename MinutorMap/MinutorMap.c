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
static unsigned char *LoadBlock(char *filename);
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
	unsigned char *grid;
	char filename[256];
	int ofs=0,xOfs=0,prevy,zOfs,bofs;
	int x,z,i;
	unsigned int color;
	unsigned char pixel;



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

	grid=(unsigned char *)Cache_Find(bx,bz);
	if (grid==NULL)
	{
		grid=LoadBlock(filename);
		if (grid==NULL) //blank tile
		{
			memset(bits,0xff,16*16*4);
			return;
		}
		Cache_Add(bx,bz,grid);
	}
	for (x=0;x<16;x++,xOfs+=128)
	{
		prevy=-1;
		zOfs=xOfs;
		for (z=0;z<16;z++,zOfs+=128*16)
		{
			bofs=zOfs+y;
			color=0;
			for (i=y;i>=0;i--,bofs--)
			{
				pixel=grid[bofs];
				if (pixel<numBlocks && blocks[pixel].canDraw)
				{
					if (prevy==-1) prevy=i;
					if (prevy<i)
						color=blocks[pixel].light;
					else if (prevy>i)
						color=blocks[pixel].dark;
					else
						color=blocks[pixel].color;
					prevy=i;
					break;
				}
			}
			bits[ofs++]=color>>16;
			bits[ofs++]=color>>8;
			bits[ofs++]=color;
			bits[ofs++]=0xff;
		}
	}
}
static unsigned char *LoadBlock(char *filename)
{
	gzFile gz=newNBT(filename);
	unsigned char *data=nbtGetBlocks(gz);
	nbtClose(gz);
	return data;
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
