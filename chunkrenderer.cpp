/** Copyright (c) 2019, Mc_Etlam */

#include "./chunk.h"
#include "./chunkrenderer.h"
#include "./chunkcache.h"
#include "./mapview.h"
#include "./blockidentifier.h"
#include "./biomeidentifier.h"
#include "./clamp.h"

ChunkRenderer::ChunkRenderer(int cx, int cz, int y, int flags)
  : cx(cx)
  , cz(cz)
  , depth(y)
  , flags(flags)
  , cache(ChunkCache::Instance())
{}


void ChunkRenderer::run() {
  // get existing Chunk entry from Cache
  Chunk *chunk = cache.fetchCached(cx, cz);
  // render Chunk data
  if (chunk) {
    renderChunk(chunk);
  }
  emit rendered(cx, cz);
}

void ChunkRenderer::renderChunk(Chunk *chunk) {
  int offset = 0;
  uchar *bits = chunk->image;
  uchar *depthbits = chunk->depth;
  for (int z = 0; z < 16; z++) {  // n->s
    int lasty = -1;
    for (int x = 0; x < 16; x++, offset++) {  // e->w
      // initialize color
      uchar r = 0, g = 0, b = 0;
      double alpha = 0.0;
      // get Biome
      auto &biome = BiomeIdentifier::Instance().getBiome(chunk->biomes[offset]);
      int top = depth;
      if (top > chunk->highest)
        top = chunk->highest;
      int highest = 0;
      for (int y = top; y >= 0; y--) {  // top->down
        int sec = y >> 4;
        ChunkSection *section = chunk->sections[sec];
        if (!section) {
          y = (sec << 4) - 1;  // skip whole section
          continue;
        }

        // get data value
        //int data = section->getData(offset, y);

        // get BlockInfo from block value
        BlockInfo &block = BlockIdentifier::Instance().getBlockInfo(section->getPaletteEntry(offset, y).hid);
        if (block.alpha == 0.0) continue;

        // get light value from one block above
        int light = 0;
        ChunkSection *section1 = NULL;
        if (y < 255)
          section1 = chunk->sections[(y+1) >> 4];
        if (section1)
          light = section1->getBlockLight(offset, y+1);
        int light1 = light;
        if (!(flags & MapView::flgLighting))
          light = 13;
        if (alpha == 0.0 && lasty != -1) {
          if (lasty < y)
            light += 2;
          else if (lasty > y)
            light -= 2;
        }
//        if (light < 0) light = 0;
//        if (light > 15) light = 15;

        // get current block color
        QColor blockcolor = block.colors[15];  // get the color from Block definition
        if (block.biomeWater()) {
          blockcolor = biome.getBiomeWaterColor(blockcolor);
        }
        else if (block.biomeGrass()) {
          blockcolor = biome.getBiomeGrassColor(blockcolor, y-64);
        }
        else if (block.biomeFoliage()) {
          blockcolor = biome.getBiomeFoliageColor(blockcolor, y-64);
        }

        // shade color based on light value
        double light_factor = pow(0.90,15-light);
        quint32 colr = std::clamp( int(light_factor*blockcolor.red()),   0, 255 );
        quint32 colg = std::clamp( int(light_factor*blockcolor.green()), 0, 255 );
        quint32 colb = std::clamp( int(light_factor*blockcolor.blue()),  0, 255 );

        // process flags
        if (flags & MapView::flgDepthShading) {
          // Use a table to define depth-relative shade:
          static const quint32 shadeTable[] = {
            0, 12, 18, 22, 24, 26, 28, 29, 30, 31, 32};
          size_t idx = qMin(static_cast<size_t>(depth - y),
                            sizeof(shadeTable) / sizeof(*shadeTable) - 1);
          quint32 shade = shadeTable[idx];
          colr = colr - qMin(shade, colr);
          colg = colg - qMin(shade, colg);
          colb = colb - qMin(shade, colb);
        }
        if (flags & MapView::flgMobSpawn) {
          // get block info from 1 and 2 above and 1 below
          uint blid1(0), blid2(0), blidB(0);  // default to legacy air (todo: better handling of block above)
          ChunkSection *section2 = NULL;
          ChunkSection *sectionB = NULL;
          if (y < 254)
            section2 = chunk->sections[(y+2) >> 4];
          if (y > 0)
            sectionB = chunk->sections[(y-1) >> 4];
          if (section1) {
            blid1 = section1->getPaletteEntry(offset, y+1).hid;
          }
          if (section2) {
            blid2 = section2->getPaletteEntry(offset, y+2).hid;
          }
          if (sectionB) {
            blidB = sectionB->getPaletteEntry(offset, y-1).hid;
          }
          BlockInfo &block2 = BlockIdentifier::Instance().getBlockInfo(blid2);
          BlockInfo &block1 = BlockIdentifier::Instance().getBlockInfo(blid1);
          BlockInfo &block0 = block;
          BlockInfo &blockB = BlockIdentifier::Instance().getBlockInfo(blidB);
          int light0 = section->getBlockLight(offset, y);

           // spawn check #1: on top of solid block
           if (block0.doesBlockHaveSolidTopSurface() &&
               !block0.isBedrock() && light1 < 8 &&
               !block1.isBlockNormalCube() && block1.spawninside &&
               !block1.isLiquid() &&
               !block2.isBlockNormalCube() && block2.spawninside) {
             colr = (colr + 256) / 2;
             colg = (colg + 0) / 2;
             colb = (colb + 192) / 2;
           }
           // spawn check #2: current block is transparent,
           // but mob can spawn through (e.g. snow)
           if (blockB.doesBlockHaveSolidTopSurface() &&
               !blockB.isBedrock() && light0 < 8 &&
               !block0.isBlockNormalCube() && block0.spawninside &&
               !block0.isLiquid() &&
               !block1.isBlockNormalCube() && block1.spawninside) {
             colr = (colr + 192) / 2;
             colg = (colg + 0) / 2;
             colb = (colb + 256) / 2;
           }
        }
        if (flags & MapView::flgBiomeColors) {
          colr = biome.colors[light].red();
          colg = biome.colors[light].green();
          colb = biome.colors[light].blue();
          alpha = 0;
        }

        // combine current block to final color
        if (alpha == 0.0) {
          // first color sample
          alpha = block.alpha;
          r = colr;
          g = colg;
          b = colb;
          highest = y;
        } else {
          // combine further color samples with blending
          r = (quint8)(alpha * r + (1.0 - alpha) * colr);
          g = (quint8)(alpha * g + (1.0 - alpha) * colg);
          b = (quint8)(alpha * b + (1.0 - alpha) * colb);
          alpha += block.alpha * (1.0 - alpha);
        }

        // finish depth (Y) scanning when color is saturated enough
        if (block.alpha == 1.0 || alpha > 0.9)
          break;
      }
      if (flags & MapView::flgCaveMode) {
        float cave_factor = 1.0;
        int cave_test = 0;
        for (int y=highest-1; (y >= 0) && (cave_test < CaveShade::CAVE_DEPTH); y--, cave_test++) {  // top->down
          // get section
          ChunkSection *section = chunk->sections[y >> 4];
          if (!section) continue;
          // get data value
          // int data = section->getData(offset, y);
          // get BlockInfo from block value
          BlockInfo &block = BlockIdentifier::Instance().getBlockInfo(section->getPaletteEntry(offset, y).hid);
          if (block.transparent) {
            cave_factor -= CaveShade::getShade(cave_test);
          }
        }
        cave_factor = std::max(cave_factor,0.25f);
        // darken color by blending with cave shade factor
        r = (quint8)(cave_factor * r);
        g = (quint8)(cave_factor * g);
        b = (quint8)(cave_factor * b);
      }
      *depthbits++ = lasty = highest;
      *bits++ = b;
      *bits++ = g;
      *bits++ = r;
      *bits++ = 0xff;
    }
  }
  chunk->renderedAt = depth;
  chunk->renderedFlags = flags;
}


// define a shading curve for Cave Mode:

CaveShade::CaveShade()
{
  // calculate exponential function for cave shade
  float cavesum = 0.0;
  for (int i=0; i<CAVE_DEPTH; i++) {
   caveshade[i] = 1/exp(i/(CAVE_DEPTH/2.0));
    cavesum += caveshade[i];
  }
  for (int i=0; i<CAVE_DEPTH; i++) {
    caveshade[i] = 1.5 * caveshade[i] / cavesum;
  }
}

float CaveShade::getShade(int index) {
  static CaveShade singleton;
  return singleton.caveshade[index];
}
