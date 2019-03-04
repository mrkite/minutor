/** Copyright (c) 2013, Sean Kasun */

#include "./chunk.h"

Chunk::Chunk() {
  loaded = false;
}

void Chunk::load(const NBT &nbt) {
  renderedAt = -1;  // impossible.
  renderedFlags = 0;  // no flags
  memset(this->biomes, 127, 256);  // init to unknown biome
  for (int i = 0; i < 16; i++) {
      this->sections[i] = NULL;
  }
  highest = 0;

  auto level = nbt.at("Level");
  chunkX = level->at("xPos")->toInt();
  chunkZ = level->at("zPos")->toInt();

  auto biomes = level->at("Biomes");
  //qDebug().nospace() << "biomes type is " << biomes->getType() ;

	//const void* pBiomesArray = biomes->toByteArray();
	/* debug */
	const void* pBiomesArray = NULL;
	if ( biomes->getType() == Tag::ByteArray)
	{
		pBiomesArray = biomes->toByteArray();
	} 
	else if ( biomes->getType() == Tag::IntArray)
	{
		pBiomesArray = biomes->toIntArray();
	}
	else
	{
		pBiomesArray = biomes->toByteArray();
	}
  //qDebug().nospace() << "this->biomes " << qPrintable(p1) ;
  //qDebug().nospace() << "biomes->toByteArray() " << QString("%1").arg((quint64)pBiomesArray, 0, 16)  ;
  const int nBiomesLength = biomes->length();
  

  //memcpy(this->biomes, biomes->toByteArray(), biomes->length());
    if( pBiomesArray != NULL ) {
        memcpy(this->biomes, pBiomesArray, nBiomesLength);  
    }

  auto sections = level->at("Sections");
  int numSections = sections->length();
  //qDebug().nospace() << "section count: " << QString("%1").arg((qint64)numSections, 0, 10);

  // for debug
  Tag_List* listSections = NULL;
  if( sections->getType() == Tag::List ) {
		listSections = (Tag_List*)sections;
  }

  for (int i = 0; i < numSections; i++) {
    auto section = sections->at(i);
    auto cs = new ChunkSection();

	// by hzw. the blocks is 16bit??
    //auto raw = section->at("Blocks")->toByteArray();
	//qDebug().nospace() << "section type is " << section->getType() ;
	if( section->getType() == Tag::List ) {
		//((Tag_List*)section)->PrintDebugInfo();
	}
	if( section->getType() == Tag::Compound ) {
		//((Tag_Compound*)section)->PrintDebugInfo();
	}
	auto section_blocks = section->at("Blocks");
    //qDebug().nospace() << "section_blocks length " << QString("%1").arg((quint64)section_blocks->length(), 0, 16)  ;
	// added by hzw
	if ( section_blocks == &NBT::Null)
	{
		//qDebug().nospace() << "null at " << QString("%1").arg((qint64)chunkX, 0, 10) << " " << QString("%1").arg((qint64)chunkZ, 0, 10);
		section_blocks = section->at("BlockStates");
		if ( section_blocks == &NBT::Null)
		{
			//qDebug().nospace() << "null 2 at " << QString("%1").arg((qint64)chunkX, 0, 10) << " " << QString("%1").arg((qint64)chunkZ, 0, 10);
			continue;
		}
	}
	//auto raw = section->at("Blocks")->toByteArray();
	const quint8 * raw = NULL;
	
	if ( section_blocks->getType() == Tag::ByteArray) 
	{
		//qDebug().nospace() << "byte array" ;
		raw = section_blocks->toByteArray();
		if( raw == NULL ) {
			qDebug().nospace() << "null raw " ;
			continue;
		}
		for (int i = 0; i < 4096; i++) {
		  cs->blocks[i] = raw[i];
		}
	}
	else if ( section_blocks->getType() == Tag::LongArray) 
	{
		//qDebug().nospace() << "long array" ;
		raw = (quint8*)(((Tag_Long_Array*)section_blocks)->toLongArray());
		int nRawLength = ((Tag_Long_Array*)section_blocks)->length();
		//qDebug().nospace() << "long array length " << nRawLength;
		if( raw == NULL ) {
			qDebug().nospace() << "null raw " ;
			continue;
		}

		int nBlockBitWidth = nRawLength/64;
		//qDebug().nospace() << "nBlockBitWidth " << nBlockBitWidth;

		unsigned char PaletteMap[256];
		auto section_palette = section->at("Palette");
		if( section_palette->getType() == Tag::List ) 
		{
			Tag_List* section_palette_real = (Tag_List*)section_palette;
			if( nRawLength != 256 ) {
				qDebug().nospace() << "non-256 palette length " << section_palette_real->length();
				//section_palette_real->PrintDebugInfo();
				//qDebug().nospace() << section_blocks->toString();
			}
			memset(PaletteMap, 14, 256);

			for( int i=0; i<256; ++i ) 
			{ 
				if( i< section_palette_real->length() ) 
				{
					QString qstrItemName = section_palette_real->at(i)->at("Name")->toString();
					//qDebug().nospace() << "palette item " << qstrItemName ;
					//qDebug().nospace() << "palette item to comp" << QString("minecraft:grass") ;
					//qDebug().nospace() << "palette item type " << section_palette_real->at(i)->at("Name")->getType();
					// seems value 2/5 make segment fault
					if(0 == qstrItemName.compare(QString("minecraft:air")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:stone")))
					{
						PaletteMap[i] = 1;
					}
					else if( 0 == qstrItemName.compare(QString("minecraft:grass")) ) 
					{
						PaletteMap[i] = 2;
					} 
					else if(0 == qstrItemName.compare(QString("minecraft:dirt")))
					{
						PaletteMap[i] = 3;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:cobble_stone")))
					{
						PaletteMap[i] = 4;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:oak_wood_plank")))
					{
						PaletteMap[i] = 5;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:oak_sapling")))
					{
						PaletteMap[i] = 6;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:bedrock")))
					{
						PaletteMap[i] = 7;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:water_flow"))) // flowing
					{
						PaletteMap[i] = 8;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:water")))
					{
						PaletteMap[i] = 9;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:lava_flow"))) // flowing
					{
						PaletteMap[i] = 10;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:lava")))
					{
						PaletteMap[i] = 11;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:sand")))
					{
						PaletteMap[i] = 12;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:gravel")))
					{
						PaletteMap[i] = 13;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:gold_ore")))
					{
						PaletteMap[i] = 14;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:iron_ore")))
					{
						PaletteMap[i] = 15;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:coal_ore")))
					{
						PaletteMap[i] = 16;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:oak_wood")))
					{
						PaletteMap[i] = 17;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:oak_leaves")))
					{
						PaletteMap[i] = 18;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:sponge")))
					{
						PaletteMap[i] = 19;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:glass")))
					{
						PaletteMap[i] = 20;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:lapis_ore")))
					{
						PaletteMap[i] = 21;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:snow")))
					{
						PaletteMap[i] = 78;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:ice")))
					{
						PaletteMap[i] = 79;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:birch_leaves")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:birch_fence")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:birch_log")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:diamond_ore")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:redstone_ore")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:grass_block")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:sand_stone")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:dandelion")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:torch")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:wall_torch")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:oak_door")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:stone_bricks")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:acacia_leaves")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:acacia_log")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:tall_grass")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:poppy")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:oak_log")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:oak_planks")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:oak_fence")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:rail")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:cave_air")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:cobweb")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:vine")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:jungle_leaves")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:jungle_log")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:andesite")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:granite")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:infested_stone")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:emerald_ore")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:clay")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:lily_pad")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:redstone_torch")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:powered_rail")))
					{
						PaletteMap[i] = 0;
					}
					else if(0 == qstrItemName.compare(QString("minecraft:brown_mushroom")))
					{
						PaletteMap[i] = 0;
					}
					else
					{
						//qDebug().nospace() << "unhandled palette item " << qstrItemName ;
					}
				}

			}
		} else {
			qDebug().nospace() << "Palette not list??" ;
		}
		if( raw == NULL ) {
			qDebug().nospace() << "null raw " ;
			continue;
		}
		for (int i = 0; i < 4096 ; i++) {
		//for (int i = 0; i < 4096 && i< nRawLength*16; i++) {
			//cs->blocks[i] = raw[i];
			if (nBlockBitWidth == 4)
			{
				if( i%2 == 0 ) {
					//cs->blocks[i] = PaletteMap[raw[i/2] % 4 ]; // wierd correct...
					unsigned char u8PaletteIndex = raw[i/2] % 16;
					//qDebug().nospace() << "PaletteIndex " << u8PaletteIndex ;
					cs->blocks[i] = PaletteMap[u8PaletteIndex];
				}
				else 
				{
					cs->blocks[i] = PaletteMap[raw[i/2] >> 4];
				}
				
			} 
			/*
			else if(nBlockBitWidth == 5)
			{
				if ( i*5%8 == 0 )
				{
					cs->blocks[i] = PaletteMap[ raw[i*5/8] & 31 ];
				}
				else if ( i*5%8 < 3 )
				{
					cs->blocks[i] = PaletteMap[ (raw[i*5/8] >> (i*5%8) ) & 31 ];
				}
				else
				{
					cs->blocks[i] = PaletteMap[ (raw[i*5/8] >> (i*5%8) + raw[i*5/8+1] << (8 - i*5%8) ) & 31 ];
				}
			}
			*/
			else if(nBlockBitWidth == 5)
			{
				/*
				if( i%2 == 0 ) {
					cs->blocks[i] = PaletteMap[raw[i/2] % 4 + raw[2048 + i/8] ];
				}
				else 
				{
					cs->blocks[i] = PaletteMap[raw[i/2] >> 4];
				}
				*/
				cs->blocks[i] = (i/4) % 256; // test purpose
			}
			else if(nBlockBitWidth == 6)
			{
				/*
				cs->blocks[i] = PaletteMap
					[
						//( raw[i/2] >> ((i%2) *4)) % 4 
						( raw[i/2] >> ((i%2) *4)) % 16 
							+ 
						(( raw[2048 + i/4] >> ((i%4) *2)) % 2) * 16
						
					];
				*/
				/*
				cs->blocks[i] = PaletteMap[ 
												(
													raw[(i*6)/8] >> ((i*6)%8) + 
													raw[(i*6)/8+1] << (8 - (i*6)%8) 
												) 
												& 63 
											];
				*/
				cs->blocks[i] = (i/8) % 256; // test purpose
			}
			else
			{
				/*
				if ( i*nBlockBitWidth%8 == 0 )
				{
					cs->blocks[i] = PaletteMap[ raw[i*nBlockBitWidth/8] & 31 ];
				}
				else if ( i*nBlockBitWidth%8 < 3 )
				{
					cs->blocks[i] = PaletteMap[ (raw[i*nBlockBitWidth/8] >> (i*nBlockBitWidth%8) ) & 31 ];
				}
				else
				{
					cs->blocks[i] = PaletteMap[ (raw[i*nBlockBitWidth/8] >> (i*nBlockBitWidth%8) + raw[i*nBlockBitWidth/8+1] << (8 - i*nBlockBitWidth%8) ) & 31 ];
				}
				*/
				cs->blocks[i] = i % 256; // test purpose
				qDebug().nospace() << "unknown bit width " << nBlockBitWidth;
			}
			//cs->blocks[i] = i % 256; // test purpose
			//cs->blocks[i] = PaletteMap[raw[i*4]];
		}
	} 
	else 
	{
		//qDebug().nospace() << "unknown array " << section_blocks->getType() ;;
		raw = section_blocks->toByteArray();
		if( raw == NULL ) {
			qDebug().nospace() << "null raw " ;
			continue;
		}
		for (int i = 0; i < 4096; i++) {
		  cs->blocks[i] = raw[i];
		}
	}

/*
    for (int i = 0; i < 4096; i++) {
      cs->blocks[i] = raw[i];
	}
*/

    if (section->has("Add")) {
      auto raw_add = section->at("Add")->toByteArray();
      for (int i = 0; i < 2048; i++) {
        cs->blocks[i * 2] |= (raw_add[i] & 0xf) << 8;
        cs->blocks[i * 2 + 1] |= (raw_add[i] & 0xf0) << 4;
      }
    }
	//
	auto section_data = section->at("Data");
	//qDebug().nospace() << "section_data type is " << section_data->getType() ;
	if( section_data == &NBT::Null ) {
		//qDebug().nospace() << "section_data is null" ;
	} else {
	    memcpy(cs->data, section_data->toByteArray(), 2048);
	}
    memcpy(cs->light, section->at("BlockLight")->toByteArray(), 2048);
    int idx = section->at("Y")->toInt();
    this->sections[idx] = cs;
  }
  loaded = true;


  auto entitylist = level->at("Entities");
  int numEntities = entitylist->length();
  for (int i = 0; i < numEntities; ++i) {
    auto e = Entity::TryParse(entitylist->at(i));
    if (e) {
      entities.insertMulti(e->type(), e);
	}
  }

  for (int i = 15; i >= 0; i--) {  // check for the highest block in this chunk
    if (this->sections[i]) {
      for (int j = 4095; j >= 0; j--) {
        if (this->sections[i]->blocks[j]) {
          highest = i * 16 + (j >> 8);
          return;
        }
      }
    }
  }
}

Chunk::~Chunk() {
  if (loaded) {
    for (int i = 0; i < 16; i++)
      if (sections[i]) {
        delete sections[i];
        sections[i] = NULL;
      }
  }
}

ChunkSection::ChunkSection() {
	memset(blocks, 0, sizeof(blocks));
	memset(data, 0, sizeof(data));
	memset(light, 0, sizeof(light));
}

quint16 ChunkSection::getBlock(int x, int y, int z) {
  int xoffset = x;
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  return blocks[xoffset + yoffset + zoffset];
}

quint16 ChunkSection::getBlock(int offset, int y) {
  int yoffset = (y & 0x0f) << 8;
  return blocks[offset + yoffset];
}

quint8 ChunkSection::getData(int x, int y, int z) {
  int xoffset = x;
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  int value = data[(xoffset + yoffset + zoffset) / 2];
  if (x & 1) value >>= 4;
  return value & 0x0f;
}

quint8 ChunkSection::getData(int offset, int y) {
  int yoffset = (y & 0x0f) << 8;
  int value = data[(offset + yoffset) / 2];
  if (offset & 1) value >>= 4;
  return value & 0x0f;
}

quint8 ChunkSection::getLight(int x, int y, int z) {
  int xoffset = x;
  int yoffset = (y & 0x0f) << 8;
  int zoffset = z << 4;
  int value = light[(xoffset + yoffset + zoffset) / 2];
  if (x & 1) value >>= 4;
  return value & 0x0f;
}

quint8 ChunkSection::getLight(int offset, int y) {
  int yoffset = (y & 0x0f) << 8;
  int value = light[(offset + yoffset) / 2];
  if (offset & 1) value >>= 4;
  return value & 0x0f;
}
