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

#include "chunkloader.h"
#include "chunk.h"

ChunkLoader::ChunkLoader(QString path,int x,int z,Chunk *chunk) : path(path),x(x),z(z),chunk(chunk)
{
}
ChunkLoader::~ChunkLoader()
{
}

void ChunkLoader::run()
{
	int rx=x>>5;
	int rz=z>>5;

	QFile f(path+"/region/r."+QString::number(rx)+"."+QString::number(rz)+".mca");
	if (!f.open(QIODevice::ReadOnly)) //no chunks in this region
	{
		emit loaded(x,z);
		return;
	}
	//map header into memory
	uchar *header=f.map(0,4096);
	int offset=4*((x&31)+(z&31)*32);
	int coffset=(header[offset]<<16)|(header[offset+1]<<8)|header[offset+2];
	int numSectors=header[offset+3];
	f.unmap(header);

	if (coffset==0) // no chunk
	{
		f.close();
		emit loaded(x,z);
		return;
	}

	uchar *raw=f.map(coffset*4096,numSectors*4096);
	NBT nbt(raw);
	chunk->load(nbt);
	f.unmap(raw);
	f.close();

	emit loaded(x,z);
}
