/*
Copyright (c) 2010, Sean Kasun
	Parts Copyright (c) 2010, Ryan Hitchman
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

/*
This is a simple tree cache.  The tree can become unbalanced... probably should do something
about that
*/

// we'll hold 5000 blocks at a time, that's 163 megs full up
// that sounds like a lot, but it's only enough space for a 1136x1136 grid

#define CLEANINTERVAL 2000

static BlockCache *root=NULL;
static long lastClean=0;
static long age=0;

static void cleannode(BlockCache **parentlink);
static void emptynode(BlockCache *node);

static BlockCache *newBC(int bx,int bz,void *data)
{
	BlockCache *bc=(BlockCache *)malloc(sizeof(BlockCache));
	bc->left=NULL;
	bc->right=NULL;
	bc->data=data;
	bc->bx=bx;
	bc->bz=bz;
	bc->age=age++;
	return bc;
}

void Cache_Add(int bx,int bz, void *data)
{
	BlockCache *node;
	if (age-lastClean>CLEANINTERVAL)
		Cache_Clean();

	if (root==NULL)
	{
		root=newBC(bx,bz,data);
		return;
	}
	node=root;
	while (1)
	{
		if (node->bx<bx)
		{
			if (node->left==NULL)
			{
				node->left=newBC(bx,bz,data);
				return;
			}
			node=node->left;
		}
		else if (node->bx>bx)
		{
			if (node->right==NULL)
			{
				node->right=newBC(bx,bz,data);
				return;
			}
			node=node->right;
		}
		else //found x slot, now find z slot
		{
			if (node->bz<bz)
			{
				if (node->left==NULL)
				{
					node->left=newBC(bx,bz,data);
					return;
				}
				node=node->left;
			}
			else if (node->bz>bz)
			{
				if (node->right==NULL)
				{
					node->right=newBC(bx,bz,data);
					return;
				}
				node=node->right;
			}
			else //replace
			{
				node->data=data;
				node->age=age;
				return;
			}
		}
	}
}

void *Cache_Find(int bx,int bz)
{
	BlockCache *node=root;
	while (node)
	{
		if (node->bx<bx)
			node=node->left;
		else if (node->bx>bx)
			node=node->right;
		else // found x slot, now find z
		{
			if (node->bz<bz)
				node=node->left;
			else if (node->bz>bz)
				node=node->right;
			else
			{
				node->age=age;
				return node->data;
			}
		}
	}
	return NULL;
}
void Cache_Clean()
{
	if (root==NULL) return; //not possible!
	cleannode(&root);
	lastClean=age;
}
static void cleannode(BlockCache **parentlink)
{
	BlockCache *node=*parentlink;
	// work from the bottom up... so we can prune dead leaves
	if (node->left)
		cleannode(&node->left);
	if (node->right)
		cleannode(&node->right);

	if (node->age<lastClean)
	{
		if (node->data)
			free(node->data);
		node->data=NULL;
		if (node->left && node->right) //both children are alive.. we have to leave a dead branch
			return;
		if (node->left) //only left is alive
			*parentlink=node->left;
		else if (node->right) //only right is alive
			*parentlink=node->right;
		else //no children alive
			*parentlink=NULL;
		free(node); //prune branch
	}
}

void Cache_Empty()
{
	if (root==NULL) return;
	emptynode(root);
	lastClean=age=0;
	root=NULL;
}
static void emptynode(BlockCache *node)
{
	if (node->left)
		emptynode(node->left);
	if (node->right)
		emptynode(node->right);
	if (node->data)
		free(node->data);
	free(node);
}
