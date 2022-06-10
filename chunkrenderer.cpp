/** Copyright (c) 2019, EtlamGit */

#include "chunk.h"
#include "chunkrenderer.h"
#include "chunkcache.h"
#include "mapview.h"
#include "identifier/blockidentifier.h"
#include "identifier/biomeidentifier.h"
#include "clamp.h"
#include "worldinfo.h"
#include "java.h"

ChunkRenderer::ChunkRenderer(int cx, int cz, int y, int flags)
  : cx(cx)
  , cz(cz)
  , depth(y)
  , flags(flags)
  , cache(ChunkCache::Instance())
{}


void ChunkRenderer::run() {
  // get existing Chunk entry from Cache
  QSharedPointer<Chunk> chunk(cache.fetchCached(cx, cz));
  // render Chunk data
  if (chunk) {
    renderChunk(chunk);
  }
  emit rendered(cx, cz);
}

void ChunkRenderer::renderChunk(QSharedPointer<Chunk> chunk) {
  // threshold for mob spawn detection
  const int lightSpawnSave = (chunk->version >= 2800)? 1 : 8;

  int offset = 0;
  uchar *bits = chunk->image;
  short *depthbits = chunk->depth;

  // adapt y loop start/stop value to render depth and available data in Chunk
  int startY = std::min(chunk->highest, this->depth);
  int stopY  = chunk->lowest;
  if (this->flags & MapView::flgSingleLayer) {
    startY = this->depth;
    stopY  = this->depth;
  }

  bool isSlimeChunk = false;
  if (this->flags & MapView::flgSlimeChunks) {
    long long seed =
        ( WorldInfo::Instance().getSeed() +
          (int) (cx * cx * 0x4c1906) +
          (int) (cx * 0x5ac0db) +
          (int) (cz * cz) * 0x4307a7LL +
          (int) (cz * 0x5f24f) ^ 0x3ad8025fLL );
    if (Java::Random(seed).nextInt(10) == 0)
      isSlimeChunk = true;
  }

  float regionalDifficulty = 0.0;
  if (this->flags & MapView::flgInhabitedTime) {
    // regional difficulty is max-capped at 3600000 ticks
    long long inhabitedTime = std::min<long long>(chunk->inhabitedTime, 3600000);
    regionalDifficulty = 6.0 * static_cast<double>(inhabitedTime) / 3600000.0;
  }

  // render loop
  for (int z = 0; z < 16; z++) {  // n->s
    // we do not know the last y value from Chunk to the east, -> set special value
    int lasty = -9999;
    for (int x = 0; x < 16; x++, offset++) {  // e->w
      // initialize color
      uchar r = 0, g = 0, b = 0;
      double alpha = 0.0;

      int highest = -4096;  // highest block in current column
      for (int y = startY; y >= stopY; y--) {  // top->down
        // perform a one deep scan in SingleLayer mode
        int sec = y >> 4;
        const ChunkSection *section = chunk->getSectionByIdx(sec);
        if (!section) {
          y = (sec << 4) - 1;  // skip whole section
          continue;
        }

        // get BlockInfo from block value
        const BlockInfo &block = BlockIdentifier::Instance().getBlockInfo(section->getPaletteEntry(offset, y).hid);
        if (block.alpha == 0.0) continue;

        if (this->flags & MapView::flgSeaGround && block.isLiquid()) continue;

        // get light value from one block above
        int light;
        const ChunkSection *section1 = chunk->getSectionByY(y+1);
        if (section1)
          light = section1->getBlockLight(offset, y+1);
        else // just as fallback
          light = std::max(0, section->getBlockLight(offset, y)-1);
        int light1 = light;
        if (!(this->flags & MapView::flgLighting))
          light = 13;
        // y gradient detection / edge highlight
        if ((alpha == 0.0) && (lasty != -9999)) {
          if (lasty < y)
            light += 2;
          else if (lasty > y)
            light -= 2;
        }
//        if (light < 0) light = 0;
//        if (light > 15) light = 15;

        // get Biome
        const BiomeInfo &biome = (chunk->version >=2800) ?
            BiomeIdentifier::Instance().getBiome((quint8)chunk->getBiomeID(x,y,z)) :
            BiomeIdentifier::Instance().getBiome((qint32)chunk->getBiomeID(x,y,z));
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

        if (this->flags & MapView::flgDepthShading) {
          // Use a table to define depth-relative shade:
          static const quint32 shadeTable[] = {
            0, 12, 18, 22, 24, 26, 28, 29, 30, 31, 32};
          size_t idx = std::min(static_cast<size_t>(this->depth - y),
                            sizeof(shadeTable) / sizeof(*shadeTable) - 1);
          quint32 shade = shadeTable[idx];
          colr = colr - std::min(shade, colr);
          colg = colg - std::min(shade, colg);
          colb = colb - std::min(shade, colb);
        }

        if (this->flags & MapView::flgMobSpawn) {
          // get block info from 1 and 2 above and 1 below
          uint blid1(0), blid2(0), blidB(0);  // default to legacy air (todo: better handling of block above)
          const ChunkSection *section2 = chunk->getSectionByY(y+2);
          const ChunkSection *sectionB = chunk->getSectionByY(y-1);
          if (section1) {
            blid1 = section1->getPaletteEntry(offset, y+1).hid;
          }
          if (section2) {
            blid2 = section2->getPaletteEntry(offset, y+2).hid;
          }
          if (sectionB) {
            blidB = sectionB->getPaletteEntry(offset, y-1).hid;
          }
          const BlockInfo &block2 = BlockIdentifier::Instance().getBlockInfo(blid2);
          const BlockInfo &block1 = BlockIdentifier::Instance().getBlockInfo(blid1);
          const BlockInfo &block0 = block;
          const BlockInfo &blockB = BlockIdentifier::Instance().getBlockInfo(blidB);
          int light0 = section->getBlockLight(offset, y);

           // spawn check #1: on top of solid block
           if (block0.doesBlockHaveSolidTopSurface() &&
               !block0.isBedrock() && light1 < lightSpawnSave &&
               !block1.isBlockNormalCube() && block1.spawninside &&
               !block1.isLiquid() &&
               !block2.isBlockNormalCube() && block2.spawninside) {
             colr = (colr + 256) / 2;
             colg = (colg + 0) / 2;
             colb = (colb + 192) / 2;
           }
           // spawn check #2: current block is transparent,
           // but mob can spawn through from block below (e.g. snow)
           if (blockB.doesBlockHaveSolidTopSurface() &&
               !blockB.isBedrock() && light0 < lightSpawnSave &&
               !block0.isBlockNormalCube() && block0.spawninside &&
               !block0.isLiquid() &&
               !block1.isBlockNormalCube() && block1.spawninside) {
             colr = (colr + 192) / 2;
             colg = (colg + 0) / 2;
             colb = (colb + 256) / 2;
           }
           // water spawn check for Drowned, introduced with "Update Aquatic" (1.13)
           if ((chunk->version >= 1478) &&
               ((biome.isOceanBiome() && (y < 58)) || biome.isRiverBiome()) &&
               (light0 < lightSpawnSave) &&
               block0.biomeWater() &&
               block1.biomeWater() ) {
             colr = (colr + 256) / 2;
             colg = (colg + 0) / 2;
             colb = (colb + 128) / 2;
           }
        }

        if (this->flags & MapView::flgBiomeColors) {
          colr = biome.colors[light].red();
          colg = biome.colors[light].green();
          colb = biome.colors[light].blue();
        }

        if (isSlimeChunk) {
          colg = (colg + 255) / 2;
        }

        if (this->flags & MapView::flgInhabitedTime) {
          // first reduce brightness
          colr = colr / 2;
          colg = colg / 2;
          colb = colb / 2;
          // then add highlight
          int rdidx = static_cast<int>(regionalDifficulty);
          double rd = regionalDifficulty - rdidx;
          switch (rdidx) {
          case 0:  // transparent -> blue
            colb = (colb + 255*regionalDifficulty) / 2;
            break;
          case 1:  // blue -> cyan
            colg = (colg + 255*rd) / 2;
            colb = (colb + 255) / 2;
            break;
          case 2:  // cyan -> green
            colg = (colg + 255) / 2;
            colb = (colb + 255*(1.0-rd)) / 2;
            break;
          case 3:  // green -> yellow
            colr = (colr + 255*rd) / 2;
            colg = (colg + 255) / 2;
            break;
          case 4:  // yellow -> red
            colr = (colr + 255) / 2;
            colg = (colg + 255*(1.0-rd)) / 2;
            break;
          case 5:  // red -> purple
            colr = (colr + 255) / 2;
            colb = (colb + 255*rd) / 2;
            break;
          default:  // saturated at purple
            colr = (colr + 255) / 2;
            colb = (colb + 255) / 2;
          }
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

      } // top -> down

      // finished to find color for current column, only continue for cave mode
      if (this->flags & MapView::flgCaveMode) {
        float cave_factor = 1.0;
        int cave_test = 0;
        for (int y=highest-1; (y >= stopY) && (cave_test < CaveShade::CAVE_DEPTH); y--, cave_test++) {  // top->down
          // get section
          const ChunkSection *section = chunk->getSectionByY(y);
          if (!section) continue;
          // get BlockInfo from block value
          const BlockInfo &block = BlockIdentifier::Instance().getBlockInfo(section->getPaletteEntry(offset, y).hid);
          if (block.transparent) {
            cave_factor -= CaveShade::getShade(cave_test);
          }
        }
        cave_factor = std::max(cave_factor, 0.25f);
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
  chunk->renderedAt = this->depth;
  chunk->renderedFlags = this->flags;
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
