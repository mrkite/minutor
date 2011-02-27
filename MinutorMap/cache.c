/*
Copyright (c) 2011, Ryan Hitchman
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* a simple cache based on a hashtable with separate chaining */

// these must be powers of two
#define HASH_XDIM 64
#define HASH_ZDIM 64
#define HASH_SIZE (HASH_XDIM * HASH_ZDIM)

// arbitrary, let users tune this?
// 6000 entries translates to minutor using ~300MB of RAM (on x64)
#define HASH_MAX_ENTRIES 6000

typedef struct block_entry {
    int x, z;
    struct block_entry *next;
    Block *data;
} block_entry;

typedef struct {
    int x, z;
} Point;

static block_entry **blockCache=NULL;

static Point *cacheHistory=NULL;
static int cacheN=0;

static int hash_coord(int x, int z) {
    return (x&(HASH_XDIM-1))*(HASH_ZDIM) + (z & (HASH_ZDIM - 1));
}

static block_entry* hash_new(int x, int z, void* data, block_entry* next) {
    block_entry* ret = malloc(sizeof(block_entry));
    ret->x = x;
    ret->z = z;
    ret->data = data;
    ret->next = next;
    return ret;
}

void Cache_Add(int bx, int bz, void *data)
{
    if (blockCache == NULL) {
        blockCache = malloc(sizeof(block_entry*) * HASH_SIZE);
        memset(blockCache, 0, sizeof(block_entry*) * HASH_SIZE);
        cacheHistory = malloc(sizeof(Point) * HASH_MAX_ENTRIES);
        cacheN = 0;
    }

    int hash = hash_coord(bx, bz);
    block_entry *to_del = NULL;

    if (cacheN >= HASH_MAX_ENTRIES) {
        // we need to remove an old entry
        Point coord = cacheHistory[cacheN % HASH_MAX_ENTRIES];
        int oldhash = hash_coord(coord.x, coord.z);

        block_entry **cur = &blockCache[oldhash];
        while (*cur != NULL) {
            if ((**cur).x == coord.x && (**cur).z == coord.z) {
                to_del = *cur;
                *cur = to_del->next;
                block_free(to_del->data);
                //free(to_del); // we will re-use this entry
                break;
            }
            cur = &((**cur).next);
        }
    }

    if (to_del != NULL) {
        // re-use the old entry for the new one
        to_del->next = blockCache[hash];
        to_del->x = bx;
        to_del->z = bz;
        to_del->data = data;
        blockCache[hash] = to_del;
    } else {
        blockCache[hash] = hash_new(bx, bz, data, blockCache[hash]);
    }

    cacheHistory[cacheN % HASH_MAX_ENTRIES].x = bx;
    cacheHistory[cacheN % HASH_MAX_ENTRIES].z = bz;
    cacheN++;
}

void *Cache_Find(int bx,int bz)
{
    if (blockCache == NULL)
        return NULL;

    block_entry *entry;

    for (entry = blockCache[hash_coord(bx, bz)]; entry != NULL; entry = entry->next)
        if (entry->x == bx && entry->z == bz)
            return entry->data;
    
    return NULL;
}

void Cache_Empty()
{
    if (blockCache == NULL)
        return;

    int hash;
    block_entry *entry, *next;
    for (hash = 0; hash < HASH_SIZE; hash++) {
        entry = blockCache[hash];
        while (entry != NULL) {
            next = entry->next;
            free(entry->data);
            free(entry);
            entry = next;
        }
    }
    
    free(blockCache);
    free(cacheHistory);
    blockCache = NULL;
}

/* a simple malloc wrapper, based on the observation that a common
 * behavior pattern for Minutor when the cache is at max capacity
 * is something like:
 *
 * newBlock = malloc(sizeof(Block));
 * cacheAdd(newBlock)
 *  free(oldBlock) // same size
 *
 * Repeatedly. Recycling the old block can prevent the need for 
 * malloc and free.
 */

static Block* last_block = NULL;

Block* block_alloc() 
{
    if (last_block != NULL)
    {
        Block* ret = last_block;
        last_block = NULL;
        return ret;
    }
    return malloc(sizeof(Block));
}

void block_free(Block* block)
{
    if (last_block != NULL)
        free(last_block);
    
    last_block = block;
}
