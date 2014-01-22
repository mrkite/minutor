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

#ifndef __BLOCKIDENTIFIER_H__
#define __BLOCKIDENTIFIER_H__

#include <QString>
#include <QMap>
#include <QHash>
#include <QList>

class JSONArray;
class JSONObject;

// bit masks for the flags
#define BlockTransparent 1
#define BlockSolid 2
#define BlockLiquid 4

// mobs can't spawn on transparent, but need 2 blocks of transparent,
// non solid, non liquid above

class BlockInfo
{
public:
	BlockInfo() {}
	bool isOpaque();
	bool isLiquid();
	bool isTransparent();

	int     id;
	QString name;
	double  alpha;
	quint8  mask;
	bool    enabled;
	bool    transparent;
	bool    liquid;
	bool    cubesolid;
	quint32 colors[16];
};

class BlockIdentifier
{
public:
	BlockIdentifier();
	~BlockIdentifier();
	int  addDefinitions(JSONArray *,int pack=-1);
	void enableDefinitions(int);
	void disableDefinitions(int);
	BlockInfo &getBlock(int id,int data);
private:
	void clearCache();
	void parseDefinition(JSONObject *block,BlockInfo *parent,int pack);
	QMap<quint32,QList<BlockInfo *> > blocks;
	QList<QList<BlockInfo*> > packs;
	BlockInfo *cache[65536];
};

#endif
