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

#include "chunkcache.h"
#include "chunkloader.h"

ChunkID::ChunkID(int x,int z) : x(x),z(z)
{
}
bool ChunkID::operator==(const ChunkID &other) const
{
	return other.x==x && other.z==z;
}
uint qHash(const ChunkID &c)
{
//	return c.x^c.z;	//quick way to hash a pair of integers
	return qHash(c.x) ^ qHash(c.z); //safe way to hash a pair of integers
}

ChunkCache::ChunkCache()
{
	cache.setMaxCost(10000); // 10000 chunks, or a little bit more than 1920x1200 blocks
}

ChunkCache::~ChunkCache()
{
}

void ChunkCache::clear()
{
	QThreadPool::globalInstance()->waitForDone();
	mutex.lock();
	cache.clear();
	mutex.unlock();
}

void ChunkCache::setPath(QString path)
{
	this->path=path;
}
QString ChunkCache::getPath()
{
	return path;
}

Chunk *ChunkCache::fetch(int x, int z)
{
	ChunkID id(x,z);
	mutex.lock();
	Chunk *chunk=cache[id];
	mutex.unlock();
	if (chunk!=NULL)
	{
		if (chunk->loaded)
			return chunk;
		return NULL; //we're loading this chunk, or it's blank.
	}
	// launch background process to load this chunk
	chunk=new Chunk();
	mutex.lock();
	cache.insert(id,chunk);
	mutex.unlock();
	ChunkLoader *loader=new ChunkLoader(path,x,z,cache,mutex);
	connect(loader,SIGNAL(loaded(int,int)),
			this,SLOT(gotChunk(int,int)));
	QThreadPool::globalInstance()->start(loader);
	return NULL;
}
void ChunkCache::gotChunk(int x,int z)
{
	emit chunkLoaded(x,z);
}
