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


#ifndef __BLOCKINFO_H__
#define __BLOCKINFO_H__

#include <stdlib.h>

/*
IMPORTANT:
Change the colors all you want, but it's very important that all colors
are unqiue... otherwise the identifier might report a different block.
*/

static struct {
	const char *name;
	int canDraw;
	unsigned int color,dark,light;
	int transparent;
	const char *icon;
} blocks[]={
{"Air",			0,0x000000,0x000000,0x000000,0,NULL},	//00
{"Stone",		1,0x787878,0x3b3b3b,0xbbbbbb,0,NULL},	//01
{"Grass",		1,0x78b34d,0x2d6802,0xace781,0,NULL},	//02
{"Dirt",		1,0x8c6344,0x562d0d,0xd5ac8d,0,NULL},	//03
{"Cobblestone",		1,0x828282,0x404040,0xc0c0c0,0,NULL},	//04
{"Wood",		1,0x9f8150,0x5c3e0d,0xdcbe8d,0,NULL},	//05
{"Sapling",		0,0x000000,0x000000,0x000000,0,"sapling.png"},	//06
{"Bedrock",		1,0x565656,0x2a2a2a,0xaaaaaa,0,NULL},	//07
{"Water",		1,0x2a5eff,0x002dce,0x79adff,137,NULL},	//08
{"Stationary Water",	1,0x2a5efe,0x002dcd,0x79adfe,137,NULL},	//09
{"Lava",		1,0xf56d00,0xb02800,0xffa83a,0,NULL},	//0a
{"Stationary Lava",	1,0xf56d01,0xb02801,0xffa83b,0,NULL},	//0b
{"Sand",		1,0xe0d8a6,0x756d3b,0xf5edba,0,NULL},	//0c
{"Gravel",		1,0x857b7b,0x453c3b,0xc5bbbb,0,NULL},	//0d
{"Gold Ore",		1,0xfcee4b,0x8c7e00,0xfffd5a,0,NULL},	//0e
{"Iron Ore",		1,0xbc9980,0x6b482f,0xebc8ae,0,NULL},	//0f
{"Coal Ore",		1,0x343434,0x191919,0x999999,0,NULL},	//10
{"Log",			1,0xb1905a,0x67460f,0xe6c58f,0,NULL},	//11
{"Leaves",		1,0x39ab27,0x006e00,0x7bed69,0,NULL},	//12
{"Sponge",		1,0xc7c743,0x6a6b00,0xeaea66,0,NULL},	//13
{"Glass",		1,0xc0f5fe,0x4c828a,0xccffff,128,NULL},	//14
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//15
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//16
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//17
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//18
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//19
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//1a
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//1b
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//1c
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//1d
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//1e
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//1f
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//20
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//21
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//22
{"Cloth",		1,0xdcdcdc,0x6d6d6d,0xededed,0,NULL},	//23
{"",			0,0x000000,0x000000,0x000000,0,NULL},	//24
{"Yellow Flower",	0,0x000000,0x000000,0x000000,0,"yellowflower.png"},//25
{"Red Rose",		0,0x000000,0x000000,0x000000,0,"redrose.png"},	//26
{"Brown Mushroom",	0,0x000000,0x000000,0x000000,0,"brownmushroom.png"},//27
{"Red Mushroom",	0,0x000000,0x000000,0x000000,0,"redmushroom.png"},//28
{"Gold Block",		1,0xfef74e,0x8a8400,0xffff5a,0,NULL},	//29
{"Iron Block",		1,0xeeeeee,0x767676,0xf6f6f6,0,NULL},	//2a
{"Double Step",		1,0xa6a6a6,0x525252,0xd2d2d2,0,NULL},	//2b
{"Step",		1,0xa5a5a5,0x515151,0xd1d1d1,0,NULL},	//2c
{"Brick",		1,0xa0807b,0x5b3b36,0xdabbb5,0,NULL},	//2d
{"TNT",			1,0xdb441a,0xa40d00,0xff8d63,0,NULL},	//2e
{"Bookcase",		1,0x795a39,0x492a09,0xc8a988,0,NULL},	//2f
{"Mossy Cobblestone",	1,0x627162,0x2c3b2c,0xacbbab,0,NULL},	//30
{"Obsidian",		1,0x1b1729,0x0d091b,0x8d899b,0,NULL},	//31
{"Torch",		1,0xfcfc00,0x8c8c00,0xffff0f,0,NULL},	//32
{"Fire",		1,0xfca100,0xa64c00,0xffcb2a,0,NULL},	//33
{"Mob Spawner",		1,0x254254,0x072436,0x86a3b5,0,"mobspawner.png"},//34
{"Wooden Stairs",	1,0x9e804f,0x5b3d0c,0xdbbd8c,0,NULL},	//35
{"Chest",		1,0xa06f23,0x653400,0xe4b467,0,"chest.png"},	//36
{"Redstone Wire",	1,0xd60000,0xb50000,0xff5f5f,0,NULL},	//37
{"Diamond Ore",		1,0x5decf5,0x008a93,0x7bffff,0,NULL},	//38
{"Diamond Block",	1,0x7fe3df,0x1c807c,0x9cfffb,0,NULL},	//39
{"Workbench",		1,0x825432,0x522502,0xd2a482,0,"workbench.png"},//3a
{"Crops",		1,0x766615,0x453500,0xc4b463,0,NULL},	//3b
{"Soil",		1,0x40220b,0x2b0d00,0xab8d76,0,NULL},	//3c
{"Furnace",		1,0x767677,0x3a3a3a,0xbababa,0,"furnace.png"},	//3d
{"Burning Furnace",	1,0x777676,0x3b3a3a,0xbbbaba,0,"furnace.png"},	//3e
{"Sign Post",		1,0x9f814f,0x5c3e0c,0xdcbe8c,0,"sign.png"},//3f
{"Wooden Door",		0,0x000000,0x000000,0x000000,0,"wooddoor.png"},//40
{"Ladder",		0,0x000000,0x000000,0x000000,0,NULL},	//41
{"Minecart Tracks",	1,0x686868,0x333433,0xb3b3b3,0,NULL},	//42
{"Cobblestone Stairs",	1,0x818181,0x3f3f3f,0xbfbfbf,0,NULL},	//43
{"Wall Sign",		0,0x000000,0x000000,0x000000,0,"sign.png"},//44
{"Lever",		0,0x000000,0x000000,0x000000,0,NULL},	//45
{"Stone Pressure Plate",1,0xa4a4a4,0x505050,0xd0d0d0,0,NULL},	//46
{"Iron Door",		0,0x000000,0x000000,0x000000,0,"irondoor.png"},//47
{"Wooden Pressure Plate",1,0x9d7f4e,0x5a3c0b,0xdabc8b,0,NULL},	//48
{"Redstone Ore",	1,0x8f0303,0x780000,0xf76c6b,0,NULL},	//49
{"Glowing Redstone Ore",1,0x900303,0x790000,0xf86c6b,0,NULL},	//4a
{"Redstone Torch (off)",1,0x560000,0x490000,0xc87272,0,NULL},	//4b
{"Redstone Torch (on)",	1,0xfd0000,0xd50000,0xff5959,0,NULL},	//4c
{"Stone Button",	0,0x000000,0x000000,0x000000,0,NULL},	//4d
{"Snow",		1,0xf0fafa,0x747e7e,0xf3fefd,0,NULL},	//4e
{"Ice",			1,0x7dadff,0x2859aa,0xa8d8ff,157,NULL},	//4f
{"Snow Block",		1,0xf1fafa,0x757e7e,0xf4fefd,0,NULL},	//50
{"Cactus",		1,0x0f791d,0x005100,0x67d175,0,NULL},	//51
{"Clay",		1,0xa2a7b4,0x4e5360,0xcdd3df,0,NULL},	//52
{"Reed",		1,0x72944e,0x31530c,0xb0d28c,0,NULL},	//53
{"Jukebox",		1,0x8a5a40,0x57270d,0xd6a68c,0,"jukebox.png"},	//54
{"Fence",		1,0x9f814e,0x5c3e0b,0xdcbe8b,0,NULL}	//55
};

#define numBlocks 0x56

#endif
