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
We use pre-multiplied alpha.
That means that if the color is #ffffff, and alpha is 0.5, then the entry should be:
0x7f7f7f,0.5
*/

static struct {
	const char *name;
	unsigned int color;
	double alpha;
} blocks[256]={
{"Air",                    0x000000,0.0},	//00
{"Stone",                  0x787878,1.0},	//01
{"Grass",                  0x78b34d,1.0},	//02
{"Dirt",                   0x8c6344,1.0},	//03
{"Cobblestone",            0x828282,1.0},	//04
{"Wood",                   0x9f8150,1.0},	//05
{"Sapling",                0x000000,0.0},	//06
{"Bedrock",                0x565656,1.0},	//07
{"Water",                  0x163288,0.535},	//08
{"Stationary Water",       0x163288,0.535},	//09
{"Lava",                   0xf56d00,1.0},	//0a
{"Stationary Lava",        0xf56d00,1.0},	//0b
{"Sand",                   0xe0d8a6,1.0},	//0c
{"Gravel",                 0x857b7b,1.0},	//0d
{"Gold Ore",               0xfcee4b,1.0},	//0e
{"Iron Ore",               0xbc9980,1.0},	//0f
{"Coal Ore",               0x343434,1.0},	//10
{"Log",                    0xb1905a,1.0},	//11
{"Leaves",                 0x39ab27,1.0},	//12
{"Sponge",                 0xc7c743,1.0},	//13
{"Glass",                  0x607b7f,0.5},	//14
{"",                       0x000000,0.0},	//15
{"",                       0x000000,0.0},	//16
{"",                       0x000000,0.0},	//17
{"",                       0x000000,0.0},	//18
{"",                       0x000000,0.0},	//19
{"",                       0x000000,0.0},	//1a
{"",                       0x000000,0.0},	//1b
{"",                       0x000000,0.0},	//1c
{"",                       0x000000,0.0},	//1d
{"",                       0x000000,0.0},	//1e
{"",                       0x000000,0.0},	//1f
{"",                       0x000000,0.0},	//20
{"",                       0x000000,0.0},	//21
{"",                       0x000000,0.0},	//22
{"Cloth",                  0xdcdcdc,1.0},	//23
{"",                       0x000000,0.0},	//24
{"Yellow Flower",          0x000000,0.0},	//25
{"Red Rose",               0x000000,0.0},	//26
{"Brown Mushroom",         0x000000,0.0},	//27
{"Red Mushroom",           0x000000,0.0},	//28
{"Gold Block",             0xfef74e,1.0},	//29
{"Iron Block",             0xeeeeee,1.0},	//2a
{"Double Step",            0xa6a6a6,1.0},	//2b
{"Step",                   0xa5a5a5,1.0},	//2c
{"Brick",                  0xa0807b,1.0},	//2d
{"TNT",                    0xdb441a,1.0},	//2e
{"Bookcase",               0x795a39,1.0},	//2f
{"Mossy Cobblestone",      0x627162,1.0},	//30
{"Obsidian",               0x1b1729,1.0},	//31
{"Torch",                  0xfcfc00,1.0},	//32
{"Fire",                   0xfca100,1.0},	//33
{"Mob Spawner",            0x254254,1.0},	//34
{"Wooden Stairs",          0x9e804f,1.0},	//35
{"Chest",                  0xa06f23,1.0},	//36
{"Redstone Wire",          0xd60000,1.0},	//37
{"Diamond Ore",            0x5decf5,1.0},	//38
{"Diamond Block",          0x7fe3df,1.0},	//39
{"Workbench",              0x825432,1.0},	//3a
{"Crops",                  0x766615,1.0},	//3b
{"Soil",                   0x40220b,1.0},	//3c
{"Furnace",                0x767677,1.0},	//3d
{"Burning Furnace",        0x777676,1.0},	//3e
{"Sign Post",              0x9f814f,1.0},	//3f
{"Wooden Door",            0x000000,0.0},	//40
{"Ladder",                 0x000000,0.0},	//41
{"Minecart Tracks",        0x686868,1.0},	//42
{"Cobblestone Stairs",     0x818181,1.0},	//43
{"Wall Sign",              0x000000,0.0},	//44
{"Lever",                  0x000000,0.0},	//45
{"Stone Pressure Plate",   0xa4a4a4,1.0},	//46
{"Iron Door",              0x000000,0.0},	//47
{"Wooden Pressure Plate",  0x9d7f4e,1.0},	//48
{"Redstone Ore",           0x8f0303,1.0},	//49
{"Glowing Redstone Ore",   0x900303,1.0},	//4a
{"Redstone Torch (off)",   0x560000,1.0},	//4b
{"Redstone Torch (on)",    0xfd0000,1.0},	//4c
{"Stone Button",           0x000000,0.0},	//4d
{"Snow",                   0xf0fafa,1.0},	//4e
{"Ice",                    0x4d6a9c,0.613},	//4f
{"Snow Block",             0xf1fafa,1.0},	//50
{"Cactus",                 0x0f791d,1.0},	//51
{"Clay",                   0xa2a7b4,1.0},	//52
{"Reed",                   0x72944e,1.0},	//53
{"Jukebox",                0x8a5a40,1.0},	//54
{"Fence",                  0x9f814e,1.0}	//55
};

enum block_types {
    BLOCK_AIR = 0x00,
    BLOCK_STONE = 0x01,
    BLOCK_GRASS = 0x02,
    BLOCK_DIRT = 0x03,
    BLOCK_COBBLESTONE = 0x04,
    BLOCK_WOOD = 0x05,
    BLOCK_SAPLING = 0x06,
    BLOCK_BEDROCK = 0x07,
    BLOCK_WATER = 0x08,
    BLOCK_STATIONARY_WATER = 0x09,
    BLOCK_LAVA = 0x0A,
    BLOCK_STATIONARY_LAVA = 0x0B,
    BLOCK_SAND = 0x0C,
    BLOCK_GRAVEL = 0x0D,
    BLOCK_GOLD_ORE = 0x0E,
    BLOCK_IRON_ORE = 0x0F,
    BLOCK_COAL_ORE = 0x10,
    BLOCK_LOG = 0x11,
    BLOCK_LEAVES = 0x12,
    BLOCK_SPONGE = 0x13,
    BLOCK_GLASS = 0x14,
    BLOCK_CLOTH = 0x23,
    BLOCK_YELLOW_FLOWER = 0x25,
    BLOCK_RED_ROSE = 0x26,
    BLOCK_BROWN_MUSHROOM = 0x27,
    BLOCK_RED_MUSHROOM = 0x28,
    BLOCK_GOLD_BLOCK = 0x29,
    BLOCK_IRON_BLOCK = 0x2A,
    BLOCK_DOUBLE_STEP = 0x2B,
    BLOCK_STEP = 0x2C,
    BLOCK_BRICK = 0x2D,
    BLOCK_TNT = 0x2E,
    BLOCK_BOOKCASE = 0x2F,
    BLOCK_MOSSY_COBBLESTONE = 0x30,
    BLOCK_OBSIDIAN = 0x31,
    BLOCK_TORCH = 0x32,
    BLOCK_FIRE = 0x33,
    BLOCK_MOB_SPAWNER = 0x34,
    BLOCK_WOODEN_STAIRS = 0x35,
    BLOCK_CHEST = 0x36,
    BLOCK_REDSTONE_WIRE = 0x37,
    BLOCK_DIAMOND_ORE = 0x38,
    BLOCK_DIAMOND_BLOCK = 0x39,
    BLOCK_WORKBENCH = 0x3A,
    BLOCK_CROPS = 0x3B,
    BLOCK_SOIL = 0x3C,
    BLOCK_FURNACE = 0x3D,
    BLOCK_BURNING_FURNACE = 0x3E,
    BLOCK_SIGN_POST = 0x3F,
    BLOCK_WOODEN_DOOR = 0x40,
    BLOCK_LADDER = 0x41,
    BLOCK_MINECART_TRACKS = 0x42,
    BLOCK_COBBLESTONE_STAIRS = 0x43,
    BLOCK_WALL_SIGN = 0x44,
    BLOCK_LEVER = 0x45,
    BLOCK_STONE_PRESSURE_PLATE = 0x46,
    BLOCK_IRON_DOOR = 0x47,
    BLOCK_WOODEN_PRESSURE_PLATE = 0x48,
    BLOCK_REDSTONE_ORE = 0x49,
    BLOCK_GLOWING_REDSTONE_ORE = 0x4A,
    BLOCK_REDSTONE_TORCH_OFF = 0x4B,
    BLOCK_REDSTONE_TORCH_ON = 0x4C,
    BLOCK_STONE_BUTTON = 0x4D,
    BLOCK_SNOW = 0x4E,
    BLOCK_ICE = 0x4F,
    BLOCK_SNOW_BLOCK = 0x50,
    BLOCK_CACTUS = 0x51,
    BLOCK_CLAY = 0x52,
    BLOCK_REED = 0x53,
    BLOCK_JUKEBOX = 0x54,
    BLOCK_FENCE = 0x55,
};

#define numBlocks 0x56

#endif
