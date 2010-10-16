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


#include "stdafx.h"
#include <string.h>

static void skipList(gzFile gz);
static void skipCompound(gzFile gz);

gzFile newNBT(const char *filename)
{
	return gzopen(filename,"rb");
}

static unsigned short readWord(gzFile gz)
{
	unsigned char buf[2];
	gzread(gz,buf,2);
	return (buf[0]<<8)|buf[1];
}
static unsigned int readDword(gzFile gz)
{
	unsigned char buf[4];
	gzread(gz,buf,4);
	return (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
}
static double readDouble(gzFile gz)
{
	int i;
	union {
		double f;
		unsigned long long l;
	} fl;
	unsigned char buf[8];
	gzread(gz,buf,8);
	fl.l=0;
	for (i=0;i<8;i++)
	{
		fl.l<<=8;
		fl.l|=buf[i];
	}
	return fl.f;
}
static void skipType(gzFile gz,int type)
{
	int len;
	switch (type)
	{
		case 1: //byte
			gzseek(gz,1,SEEK_CUR);
			break;
		case 2: //short
			gzseek(gz,2,SEEK_CUR);
			break;
		case 3: //int
			gzseek(gz,4,SEEK_CUR);
			break;
		case 4: //long
			gzseek(gz,8,SEEK_CUR);
			break;
		case 5: //float
			gzseek(gz,4,SEEK_CUR);
			break;
		case 6: //double
			gzseek(gz,8,SEEK_CUR);
			break;
		case 7: //byte array
			len=readDword(gz);
			gzseek(gz,len,SEEK_CUR);
			break;
		case 8: //string
			len=readWord(gz);
			gzseek(gz,len,SEEK_CUR);
			break;
		case 9: //list
			skipList(gz);
			break;
		case 10: //compound
			skipCompound(gz);
			break;
	}
}
static void skipList(gzFile gz)
{
	int len,i;
	unsigned char type;
	gzread(gz,&type,1);
	len=readDword(gz);
	switch (type)
	{
		case 1: //byte
			gzseek(gz,len,SEEK_CUR);
			break;
		case 2: //short
			gzseek(gz,len*2,SEEK_CUR);
			break;
		case 3: //int
			gzseek(gz,len*4,SEEK_CUR);
			break;
		case 4: //long
			gzseek(gz,len*8,SEEK_CUR);
			break;
		case 5: //float
			gzseek(gz,len*4,SEEK_CUR);
			break;
		case 6: //double
			gzseek(gz,len*8,SEEK_CUR);
			break;
		case 7: //byte array
			for (i=0;i<len;i++)
			{
				int slen=readDword(gz);
				gzseek(gz,slen,SEEK_CUR);
			}
			break;
		case 8: //string
			for (i=0;i<len;i++)
			{
				int slen=readWord(gz);
				gzseek(gz,slen,SEEK_CUR);
			}
			break;
		case 9: //list
			for (i=0;i<len;i++)
				skipList(gz);
			break;
		case 10: //compound
			for (i=0;i<len;i++)
				skipCompound(gz);
			break;
	}
}
static void skipCompound(gzFile gz)
{
	int len;
	unsigned char type=0;
	do {
		gzread(gz,&type,1);
		if (type)
		{
			len=readWord(gz);
			gzseek(gz,len,SEEK_CUR);	//skip name
			skipType(gz,type);
		}
	} while (type);
}

static int compare(gzFile gz,char *name)
{
	int ret=0;
	int len=readWord(gz);
	char *thisName=malloc(len+1);
	gzread(gz,thisName,len);
	thisName[len]=0;
	if (strcmp(thisName,name)==0)
		ret=1;
	free(thisName);
	return ret;
}

// this finds an element in a composite list.
// it works progressively, so it only finds elements it hasn't come
// across yet.
static int findElement(gzFile gz,char *name)
{
	while (1)
	{
		unsigned char type=0;
		gzread(gz,&type,1);
		if (type==0) return 0;
		if (compare(gz,name))
			return type;
		skipType(gz,type);
	}
}

unsigned char *nbtGetBlocks(gzFile gz, unsigned char *buff)
{
	int len;
	//Level/Blocks
	gzseek(gz,1,SEEK_CUR); //skip type
	len=readWord(gz); //name length
	gzseek(gz,len,SEEK_CUR); //skip name ()
	if (findElement(gz,"Level")!=10)
		return NULL;
	if (findElement(gz,"Blocks")!=7)
		return NULL;
	len=readDword(gz); //array length
	gzread(gz,buff,len);
	return buff;
}
void nbtGetSpawn(gzFile gz,int *x,int *y,int *z)
{
	int len;
	*x=*y=*z=0;
	//Data/SpawnX
	gzseek(gz,1,SEEK_CUR); //skip type
	len=readWord(gz); //name length
	gzseek(gz,len,SEEK_CUR); //skip name ()
	if (findElement(gz,"Data")!=10) return;
	if (findElement(gz,"SpawnX")!=3) return;
	*x=readDword(gz);
	if (findElement(gz,"SpawnY")!=3) return;
	*y=readDword(gz);
	if (findElement(gz,"SpawnZ")!=3) return;
	*z=readDword(gz);
}
void nbtGetPlayer(gzFile gz,int *px,int *py,int *pz)
{
	int len;
	*px=*py=*pz=0;
	//Data/Player/Pos
	gzseek(gz,1,SEEK_CUR); //skip type
	len=readWord(gz); //name length
	gzseek(gz,len,SEEK_CUR); //skip name ()
	if (findElement(gz,"Data")!=10) return;
	if (findElement(gz,"Player")!=10) return;
	if (findElement(gz,"Pos")!=9) return;
	gzseek(gz,5,SEEK_CUR); //skip subtype and num items
	*px=(int)readDouble(gz);
	*py=(int)readDouble(gz);
	*pz=(int)readDouble(gz);
}
void nbtClose(gzFile gz)
{
	gzclose(gz);
}
