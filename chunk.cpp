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


#include "chunk.h"

Chunk::Chunk()
{
	loaded=false;
}

void Chunk::load(NBT &nbt)
{
	renderedAt=-1; //impossible.
	renderedFlags=0; //no flags
	memset(this->biomes,127,256); //init to unknown biome
	for (int i=0;i<16;i++)
		this->sections[i]=NULL;
	highest=0;

	Tag *level=nbt.at("Level");
	Tag *biomes=level->at("Biomes");
	memcpy(this->biomes,biomes->toByteArray(),biomes->length());
	Tag *sections=level->at("Sections");
	int numSections=sections->length();
	for (int i=0;i<numSections;i++)
	{
		Tag *section=sections->at(i);
		ChunkSection *cs=new ChunkSection();
		const quint8 *raw=section->at("Blocks")->toByteArray();
		for (int i=0;i<4096;i++)
			cs->blocks[i]=raw[i];
		if (section->has("Add"))
		{
			raw=section->at("Add")->toByteArray();
			for (int i=0;i<2048;i++)
			{
				cs->blocks[i*2]|=(raw[i]&0xf)<<8;
				cs->blocks[i*2+1]|=(raw[i]&0xf0)<<4;
			}
		}
		memcpy(cs->data,section->at("Data")->toByteArray(),2048);
		memcpy(cs->light,section->at("BlockLight")->toByteArray(),2048);
		int idx=section->at("Y")->toInt();
		this->sections[idx]=cs;
	}
	loaded=true;
	for (int i=15;i>=0;i--) //check for the highest block in this chunk
	{
		if (this->sections[i])
			for (int j=4095;j>=0;j--)
				if (this->sections[i]->blocks[j])
				{
					highest=i*16+(j>>8);
					return;
				}
	}
}
Chunk::~Chunk()
{
	if (loaded)
		for (int i=0;i<16;i++)
			if (sections[i])
			{
				delete sections[i];
				sections[i]=NULL;
			}
}


quint16 ChunkSection::getBlock(int x, int y, int z)
{
    int xoffset = x;
    int yoffset = (y&0x0f)<<8;
    int zoffset = z<<4;
    return blocks[xoffset+yoffset+zoffset];
}

quint8 ChunkSection::getData(int x, int y, int z)
{
    int xoffset = x;
    int yoffset = (y&0x0f)<<8;
    int zoffset = z<<4;
    int value = data[(xoffset+yoffset+zoffset)/2];
    if (x&1) value>>=4;
    return value&0x0f;
}

quint8 ChunkSection::getLight(int x, int y, int z)
{
    int xoffset = x;
    int yoffset = (y&0x0f)<<8;
    int zoffset = z<<4;
    int value = light[(xoffset+yoffset+zoffset)/2];
    if (x&1) value>>=4;
    return value&0x0f;
}
