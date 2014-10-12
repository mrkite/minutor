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


#ifndef __CHUNK_H__
#define __CHUNK_H__

#include <QtCore>
#include <QVector>

#include "nbt.h"
#include "entity.h"
class BlockIdentifier;

class ChunkSection
{
public:
	quint16 getBlock(int x, int y, int z);
	quint8  getData(int x, int y, int z);
	quint8  getLight(int x, int y, int z);

	quint16 blocks[4096];
	quint8  data[2048];
	quint8  light[2048];
};

class Chunk
{
public:
	Chunk();
	void load(NBT &nbt);
	~Chunk();
protected:
	quint8 biomes[256];
	int highest;
	ChunkSection *sections[16];
	int renderedAt;
	int renderedFlags;
	QSet<QString> renderedSpecialBlockTypes;
	bool loaded;
	uchar image[16*16*4];	//cached render
	QMap<QString, Entity> entities;
	int chunkX;
	int chunkZ;
	friend class MapView;
	friend class ChunkCache;
	friend class WorldSave;
};

#endif
