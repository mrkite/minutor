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

static void skipType(bfFile bf, int type);
static void skipList(bfFile bf);
static void skipCompound(bfFile bf);

void bfread(bfFile bf, void *target, int len)
{
    if (bf.type == BF_BUFFER) {
        memcpy(target, bf.buf + *bf.offset, len);
        *bf.offset += len;
    } else if (bf.type == BF_GZIP) {
        gzread(bf.gz, target, len);
    }
}

void bfseek(bfFile bf, int offset, int whence)
{
    if (bf.type == BF_BUFFER) {
        if (whence == SEEK_CUR)
            *bf.offset += offset;
        else if (whence == SEEK_SET)
            *bf.offset = offset;
    } else if (bf.type == BF_GZIP) {
        gzseek(bf.gz, offset, whence);
    }
} 

bfFile newNBT(const char *filename)
{
    bfFile ret;
    ret.type = BF_GZIP;
    ret.gz = gzopen(filename,"rb");
    ret._offset = 0;
    ret.offset = &ret._offset;
    return ret;
}

static unsigned short readWord(bfFile bf)
{
	unsigned char buf[2];
	bfread(bf,buf,2);
	return (buf[0]<<8)|buf[1];
}
static unsigned int readDword(bfFile bf)
{
	unsigned char buf[4];
	bfread(bf,buf,4);
	return (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
}
static double readDouble(bfFile bf)
{
	int i;
	union {
		double f;
		unsigned long long l;
	} fl;
	unsigned char buf[8];
	bfread(bf,buf,8);
	fl.l=0;
	for (i=0;i<8;i++)
	{
		fl.l<<=8;
		fl.l|=buf[i];
	}
	return fl.f;
}
static void skipType(bfFile bf,int type)
{
	int len;
	switch (type)
	{
		case 1: //byte
			bfseek(bf,1,SEEK_CUR);
			break;
		case 2: //short
			bfseek(bf,2,SEEK_CUR);
			break;
		case 3: //int
			bfseek(bf,4,SEEK_CUR);
			break;
		case 4: //long
			bfseek(bf,8,SEEK_CUR);
			break;
		case 5: //float
			bfseek(bf,4,SEEK_CUR);
			break;
		case 6: //double
			bfseek(bf,8,SEEK_CUR);
			break;
		case 7: //byte array
			len=readDword(bf);
			bfseek(bf,len,SEEK_CUR);
			break;
		case 8: //string
			len=readWord(bf);
			bfseek(bf,len,SEEK_CUR);
			break;
		case 9: //list
			skipList(bf);
			break;
		case 10: //compound
			skipCompound(bf);
			break;
	}
}
static void skipList(bfFile bf)
{
	int len,i;
	unsigned char type;
	bfread(bf,&type,1);
	len=readDword(bf);
	switch (type)
	{
		case 1: //byte
			bfseek(bf,len,SEEK_CUR);
			break;
		case 2: //short
			bfseek(bf,len*2,SEEK_CUR);
			break;
		case 3: //int
			bfseek(bf,len*4,SEEK_CUR);
			break;
		case 4: //long
			bfseek(bf,len*8,SEEK_CUR);
			break;
		case 5: //float
			bfseek(bf,len*4,SEEK_CUR);
			break;
		case 6: //double
			bfseek(bf,len*8,SEEK_CUR);
			break;
		case 7: //byte array
			for (i=0;i<len;i++)
			{
				int slen=readDword(bf);
				bfseek(bf,slen,SEEK_CUR);
			}
			break;
		case 8: //string
			for (i=0;i<len;i++)
			{
				int slen=readWord(bf);
				bfseek(bf,slen,SEEK_CUR);
			}
			break;
		case 9: //list
			for (i=0;i<len;i++)
				skipList(bf);
			break;
		case 10: //compound
			for (i=0;i<len;i++)
				skipCompound(bf);
			break;
	}
}
static void skipCompound(bfFile bf)
{
	int len;
	unsigned char type=0;
	do {
		bfread(bf,&type,1);
		if (type)
		{
			len=readWord(bf);
			bfseek(bf,len,SEEK_CUR);	//skip name
			skipType(bf,type);
		}
	} while (type);
}

static int compare(bfFile bf,char *name)
{
	int ret=0;
	int len=readWord(bf);
	char *thisName=malloc(len+1);
	bfread(bf,thisName,len);
	thisName[len]=0;
	if (strcmp(thisName,name)==0)
		ret=1;
	free(thisName);
	return ret;
}

// this finds an element in a composite list.
// it works progressively, so it only finds elements it hasn't come
// across yet.
static int nbtFindElement(bfFile bf,char *name)
{
	while (1)
	{
		unsigned char type=0;
		bfread(bf,&type,1);
		if (type==0) return 0;
		if (compare(bf,name))
			return type;
		skipType(bf,type);
	}
}

int nbtGetBlocks(bfFile bf, unsigned char *buff,unsigned char *blockLight)
{
	int len,found;
	//Level/Blocks
	bfseek(bf,1,SEEK_CUR); //skip type
	len=readWord(bf); //name length
	bfseek(bf,len,SEEK_CUR); //skip name ()
	if (nbtFindElement(bf,"Level")!=10)
		return 0;

	found=0;
	while (found!=2)
	{
		int ret=0;
		unsigned char type=0;
		char *thisName;
		bfread(bf,&type,1);
		if (type==0) 
            return 0;
		len=readWord(bf);
		thisName=malloc(len+1);
		bfread(bf,thisName,len);
		thisName[len]=0;
		if (strcmp(thisName,"BlockLight")==0)
		{
			found++;
			ret=1;
			len=readDword(bf); //array length
			bfread(bf,blockLight,len);
		}
		if (strcmp(thisName,"Blocks")==0)
		{
			found++;
			ret=1;
			len=readDword(bf); //array length
			bfread(bf,buff,len);
		}
		free(thisName);
		if (!ret)
			skipType(bf,type);
	}
	return 1;
}
void nbtGetSpawn(bfFile bf,int *x,int *y,int *z)
{
	int len;
	*x=*y=*z=0;
	//Data/SpawnX
	bfseek(bf,1,SEEK_CUR); //skip type
	len=readWord(bf); //name length
	bfseek(bf,len,SEEK_CUR); //skip name ()
	if (nbtFindElement(bf,"Data")!=10) return;
	if (nbtFindElement(bf,"SpawnX")!=3) return;
	*x=readDword(bf);
	if (nbtFindElement(bf,"SpawnY")!=3) return;
	*y=readDword(bf);
	if (nbtFindElement(bf,"SpawnZ")!=3) return;
	*z=readDword(bf);
}
void nbtGetPlayer(bfFile bf,int *px,int *py,int *pz)
{
	int len;
	*px=*py=*pz=0;
	//Data/Player/Pos
	bfseek(bf,1,SEEK_CUR); //skip type
	len=readWord(bf); //name length
	bfseek(bf,len,SEEK_CUR); //skip name ()
	if (nbtFindElement(bf,"Data")!=10) return;
	if (nbtFindElement(bf,"Player")!=10) return;
	if (nbtFindElement(bf,"Pos")!=9) return;
	bfseek(bf,5,SEEK_CUR); //skip subtype and num items
	*px=(int)readDouble(bf);
	*py=(int)readDouble(bf);
	*pz=(int)readDouble(bf);
}
void nbtClose(bfFile bf)
{
    if (bf.type == BF_GZIP)
    	gzclose(bf.gz);
}
