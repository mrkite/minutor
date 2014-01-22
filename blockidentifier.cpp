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

#include "blockidentifier.h"
#include "json.h"
#include <QDebug>

static BlockInfo unknownBlock;

bool BlockInfo::isOpaque()
{
    return !(this->liquid || this->transparent);
}

bool BlockInfo::isLiquid()
{
    return this->liquid;
}

bool BlockInfo::isTransparent()
{
    return this->transparent;
}

BlockIdentifier::BlockIdentifier()
{
	// clear cache pointers
	for (int i=0;i<65536;i++)
		cache[i]=NULL;
	for (int i=0;i<16;i++)
		unknownBlock.colors[i]=0xff00ff;
	unknownBlock.alpha=1.0;
	unknownBlock.transparent=false;
	unknownBlock.cubesolid=false;
	unknownBlock.name="Unknown";
}
BlockIdentifier::~BlockIdentifier()
{
	clearCache();
	for (int i=0;i<packs.length();i++)
	{
		for (int j=0;j<packs[i].length();j++)
			delete packs[i][j];
	}
}

// this routine is ridiculously slow
BlockInfo &BlockIdentifier::getBlock(int id, int data)
{
	quint32 bid=id|(data<<12);
	//first check the cache
	if (cache[bid]!=NULL)
		return *cache[bid];

	//first get the mask
	if (blocks.contains(id))
		data&=blocks[id].first()->mask;
	//now find the variant
	if (blocks.contains(bid))
	{
		QList<BlockInfo*> &list=blocks[bid];
		//run backwards for priority sorting
		for (int i=list.length()-1;i>=0;i--)
		{
			if (list[i]->enabled)
			{
				cache[bid]=list[i];
				return *list[i];
			}
		}
	}
	//no enabled variant found
	if (blocks.contains(id))
	{
		QList<BlockInfo*> &list=blocks[id];
		for (int i=list.length()-1;i>=0;i--)
		{
			if (list[i]->enabled)
			{
				cache[bid]=list[i];
				return *list[i];
			}
		}
	}
	//no blocks at all found.. dammit
	return unknownBlock;
}

void BlockIdentifier::enableDefinitions(int pack)
{
	if (pack<0) return;
	int len=packs[pack].length();
	for (int i=0;i<len;i++)
		packs[pack][i]->enabled=true;
	//clear cache
	clearCache();
}

void BlockIdentifier::disableDefinitions(int pack)
{
	if (pack<0) return;
	int len=packs[pack].length();
	for (int i=0;i<len;i++)
		packs[pack][i]->enabled=false;
	//clear cache
	clearCache();
}

int BlockIdentifier::addDefinitions(JSONArray *defs,int pack)
{
	if (pack==-1)
	{
		pack=packs.length();
		packs.append(QList<BlockInfo*>());
	}
	int len=defs->length();
	for (int i=0;i<len;i++)
		parseDefinition(dynamic_cast<JSONObject *>(defs->at(i)),NULL,pack);
	//clear cache
	clearCache();
	return pack;
}

static int clamp(int v,int min,int max)
{
	return (v<max?(v>min?v:min):max);
}

void BlockIdentifier::clearCache()
{
	for (int i=0;i<65536;i++)
	{
        cache[i]=NULL;
	}
}

void BlockIdentifier::parseDefinition(JSONObject *b, BlockInfo *parent, int pack)
{
	int id;
	if (parent==NULL)
		id=b->at("id")->asNumber();
	else
	{
		id=parent->id;
		int data=b->at("data")->asNumber();
		id|=data<<12;
	}
	BlockInfo *block=new BlockInfo();
	block->id=id;

	if (b->has("name"))
		block->name=b->at("name")->asString();
	else if (parent!=NULL)
		block->name=parent->name;
	else
		block->name="Unknown";
	block->enabled=true;

	if (b->has("transparent"))
		block->transparent=b->at("transparent")->asBool();
	else if (parent!=NULL)
		block->transparent=parent->transparent;
	else
		block->transparent=false;

	if (b->has("liquid"))
		block->liquid=b->at("liquid")->asBool();
	else if (parent!=NULL)
		block->liquid=parent->liquid;
	else
		block->liquid=false;

	if (b->has("cubesolid"))
		block->cubesolid=b->at("cubesolid")->asBool();
	else if (parent!=NULL)
		block->cubesolid=parent->cubesolid;
	else
		block->cubesolid=false;

	if (b->has("color"))
	{
		QString color=b->at("color")->asString();
		quint32 col=0;
		for (int h=0;h<color.length();h++)
		{
			ushort c=color.at(h).unicode();
			col<<=4;
			if (c>='0' && c<='9')
				col|=c-'0';
			else if (c>='A' && c<='F')
				col|=c-'A'+10;
			else if (c>='a' && c<='f')
				col|=c-'a'+10;
		}
		int rd=col>>16;
		int gn=(col>>8)&0xff;
		int bl=col&0xff;

		if (b->has("alpha"))
			block->alpha=b->at("alpha")->asNumber();
		else if (parent!=NULL)
			block->alpha=parent->alpha;
		else
			block->alpha=1.0;

		//pre multiply alphas
		rd*=block->alpha;
		gn*=block->alpha;
		bl*=block->alpha;

		//pre-calculate light spectrum
		double y=0.299*rd+0.587*gn+0.114*bl;
		double u=(bl-y)*0.565;
		double v=(rd-y)*0.713;
		double delta=y/15;
		for (int i=0;i<16;i++)
		{
			y=i*delta;
			rd=(unsigned int)clamp(y+1.403*v,0,255);
			gn=(unsigned int)clamp(y-0.344*u-0.714*v,0,255);
			bl=(unsigned int)clamp(y+1.770*u,0,255);
			block->colors[i]=(rd<<16)|(gn<<8)|bl;
		}

	}
	else if (parent!=NULL)
	{
		for (int i=0;i<16;i++)
			block->colors[i]=parent->colors[i];
		block->alpha=parent->alpha;
	}
	else
		block->alpha=0.0;
	if (b->has("mask"))
		block->mask=b->at("mask")->asNumber();
	else
		block->mask=0xff;
	if (b->has("variants"))
	{
		JSONArray *variants=dynamic_cast<JSONArray *>(b->at("variants"));
		int vlen=variants->length();
		for (int j=0;j<vlen;j++)
			parseDefinition(dynamic_cast<JSONObject *>(variants->at(j)),block,pack);
	}
	blocks[id].append(block);
	packs[pack].append(block);
}
