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

#include "biomeidentifier.h"
#include "json.h"

static BiomeInfo unknownBiome;
BiomeIdentifier::BiomeIdentifier()
{
	unknownBiome.name="Unknown";
}
BiomeIdentifier::~BiomeIdentifier()
{
	for (int i=0;i<packs.length();i++)
	{
		for (int j=0;j<packs[i].length();j++)
			delete packs[i][j];
	}
}

BiomeInfo &BiomeIdentifier::getBiome(int biome)
{
	QList<BiomeInfo*> &list=biomes[biome];
	//search backwards for priority sorting to work
	for (int i=list.length()-1;i>=0;i--)
		if (list[i]->enabled)
			return *list[i];
	return unknownBiome;
}

void BiomeIdentifier::enableDefinitions(int pack)
{
	if (pack<0) return;
	int len=packs[pack].length();
	for (int i=0;i<len;i++)
		packs[pack][i]->enabled=true;
}
void BiomeIdentifier::disableDefinitions(int pack)
{
	if (pack<0) return;
	int len=packs[pack].length();
	for (int i=0;i<len;i++)
		packs[pack][i]->enabled=false;
}

int BiomeIdentifier::addDefinitions(JSONArray *defs,int pack)
{
	if (pack==-1)
	{
		pack=packs.length();
		packs.append(QList<BiomeInfo *>());
	}

	int len=defs->length();
	for (int i=0;i<len;i++)
	{
		JSONObject *b=dynamic_cast<JSONObject *>(defs->at(i));
		int id=b->at("id")->asNumber();

		BiomeInfo *biome=new BiomeInfo();
		biome->enabled=true;
		if (b->has("name"))
			biome->name=b->at("name")->asString();
		else
			biome->name="Unknown";
		biomes[id].append(biome);
		packs[pack].append(biome);
	}
	return pack;
}
