#include "Game/Chunk.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Game/World.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"
#include "Engine/Core/Time.hpp"
#include "ThirdParty/Squirrel/RawNoise.hpp"
#include "Game/Player.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include <filesystem>

int GetChunkIndexFromCoords(IntVec3 chunkCoords)
{
	return (chunkCoords.x) | (chunkCoords.y << (XBITS)) | (chunkCoords.z << (XBITS + YBITS));
}

int GetChunkIndexFromCoords(int x, int y, int z)
{
	return x | (y << (XBITS)) | (z << (XBITS + YBITS));
}

IntVec3 GetCoordsFromChunkIndex(int chunkIndex)
{
	return IntVec3(chunkIndex & XMASK, (chunkIndex & YMASK) >> XBITS, (chunkIndex & ZMASK) >> (XBITS + YBITS));
}

Chunk::Chunk(IntVec2 coords)
	: m_chunkCoords(coords)
{
	m_blocks = new Block[CHUNK_SIZE];
	/*
	SetNeighboringChunks();
	
	if (!RetrieveBlockDataFromDisk())
	{
		GenerateBlockData();
		SpawnBlockTemplates();
	}
	InitializeLightingData();
	*/
}

bool Chunk::RetrieveBlockDataFromDisk()
{
	FILE* filePtr = nullptr;
	errno_t error = fopen_s(&filePtr, Stringf("Saves/World_%u/Chunk(%d,%d).chunk",g_theWorld->m_worldSeed, m_chunkCoords.x, m_chunkCoords.y).c_str(), "rb");
	if (error != 0)
	{
		return false;
	}
	std::vector<uint8_t> blockDataBuffer;
	blockDataBuffer.resize((size_t)CHUNK_SIZE + 12);
	FileReadToBuffer(blockDataBuffer, filePtr);

	//file format incorrect
	if (blockDataBuffer[0] != 'G' || blockDataBuffer[1] != 'C' || blockDataBuffer[2] != 'H' || blockDataBuffer[3] != 'K' || 
		  blockDataBuffer[5] != XBITS || blockDataBuffer[6] != YBITS || blockDataBuffer[7] != ZBITS)
	{
		ERROR_RECOVERABLE(Stringf("Incorrect file format from Saves/Chunk(%d,%d).chunk", m_chunkCoords.x, m_chunkCoords.y));
		return false;
	}

	if (blockDataBuffer[4] == 1)
	{
		blockDataBuffer.resize((size_t)CHUNK_SIZE + 8);
		return ParseBlockDataVersion1(blockDataBuffer);
	}
	if (blockDataBuffer[4] == 2)
	{
		return ParseBlockDataVersion2(blockDataBuffer);
	}

	return false;
}

bool Chunk::ParseBlockDataVersion1(std::vector<unsigned char> const& bufferData)
{
	int currBlockIdx = 0;
	for (size_t typeIdx = 8; typeIdx < bufferData.size() - 1; typeIdx += 2)
	{
		BlockType currBlockType = (BlockType)bufferData[typeIdx];
		uint8_t runLength = bufferData[typeIdx + 1];
		for (int blockNum = 0; blockNum < (int)runLength; blockNum++)
		{
			m_blocks[currBlockIdx].m_blockType = currBlockType;
			currBlockIdx++;
		}
	}
	if (currBlockIdx != CHUNK_SIZE)
	{
		ERROR_RECOVERABLE(Stringf("Incorrect number of blocks read in from Saves/Chunk(%d,%d).chunk", m_chunkCoords.x, m_chunkCoords.y));
		return false;
	}
	return true;
}

void Chunk::SpawnBlockTemplates()
{
	for (int i = 0; i < (int)m_blockTemplatesForChunk.size(); i++)
	{
		BlockTemplate& blockTemplate = m_blockTemplatesForChunk[i];
		for (int b = 0; b < blockTemplate.m_templateEntries.size(); b++)
		{
			BlockTemplateEntry& templateEntry = blockTemplate.m_templateEntries[b];
			IntVec3 blockPosInChunk = blockTemplate.m_templateOrigin + templateEntry.m_templateOffset;
			if (   blockPosInChunk.x >= 0 && blockPosInChunk.x < XSIZE
				&& blockPosInChunk.y >= 0 && blockPosInChunk.y < YSIZE
				&& blockPosInChunk.z >= 0 && blockPosInChunk.z < ZSIZE   )
			{
				m_blocks[GetChunkIndexFromCoords(blockPosInChunk)].m_blockType = templateEntry.m_blocktype;
			}
		}
	}
}

void Chunk::GenerateCaves()
{
	//find all cave origins within MAX_CAVE_RADIUS from current chunk
	int maxCaveWidth = (int)MAX_CAVE_RADIUS / XSIZE;
	int maxCaveHeight = (int)MAX_CAVE_RADIUS / YSIZE;

	std::map<IntVec3, std::vector<Vec3>> cavesToCheck;
	for (int y = m_chunkCoords.y - maxCaveHeight; y <= m_chunkCoords.y + maxCaveHeight; y++)
	{
		for (int x = m_chunkCoords.x - maxCaveWidth; x <= m_chunkCoords.x + maxCaveWidth; x++)
		{
			if (ShouldColumnHaveCaveOrigin(x, y))
			{
				IntVec3 worldCaveOrigin = GetCaveOriginFromChunkCoords(IntVec2(x, y));
				AABB3 chunkBounds;
				chunkBounds.m_mins = Vec3(m_chunkCoords.x * (float)XSIZE, m_chunkCoords.y * (float)YSIZE, 0.f);
				chunkBounds.m_maxs = chunkBounds.m_mins + Vec3((float)XSIZE, (float)YSIZE, (float)ZSIZE);
				if (DoSphereAndAABBOverlap3D(worldCaveOrigin.GetVec3(), MAX_CAVE_RADIUS, chunkBounds))
				{
					AddDataForCave(cavesToCheck, worldCaveOrigin);
				}
			}
		}
	}

	for (auto cave : cavesToCheck)
	{
		float capsuleRadius = MAX_CAVE_RADIUS / 2.f;
		std::vector<Vec3>& cavePoints = cave.second;
		Vec2 chunkCenterXY = Vec2( (float)(m_chunkCoords.x * XSIZE + XSIZE / 2), (float)(m_chunkCoords.y * YSIZE + YSIZE / 2));
		float halfXSize = (float)XSIZE * .5f;
		float halfYSize = (float)YSIZE * .5f;
		float chunkRadius = sqrtf(halfXSize * halfXSize + halfYSize * halfYSize);
		for (size_t i = 0; i < cavePoints.size() - 1; i++)
		{
			Vec2 capsuleCenterXY = (cavePoints[i] + cavePoints[i + 1]).GetXY() * .5f;
			if (GetDistanceSquared2D(chunkCenterXY, capsuleCenterXY) < (chunkRadius + capsuleRadius) * (chunkRadius + capsuleRadius))
			{
				CarveCaveInChunk(cavePoints[i], cavePoints[i+1]);
			}
		}
	}
}

float Chunk::GetTerrainHeightFromGlobalXY(int globalX, int globalY)
{
	constexpr float WATER_LEVEL = 64.f;
	constexpr float RIVER_DEPTH = 4.f;
	constexpr float RIVER_BOTTOM = WATER_LEVEL - RIVER_DEPTH;
	constexpr float MAX_MOUNTAIN_Z = 96;
	constexpr float MAX_OCEAN_DEPTH = 25;

	float terrainHeightNoise = Compute2dPerlinNoise((float)globalX, (float)globalY, 200.f, 7, .5f, 2.f, true, g_theWorld->m_worldSeed);
	terrainHeightNoise = fabsf(terrainHeightNoise);
	float hilliness = .5f + .5f * Compute2dPerlinNoise((float)globalX, (float)globalY, 300.f, 9, .5f, 2.f, true, g_theWorld->m_worldSeed + 1);
	hilliness = SmoothStep3(hilliness);
	float oceanness = .5f + .5f * Compute2dPerlinNoise((float)globalX, (float)globalY, 500.f, 4, .5f, 2.f, true, g_theWorld->m_worldSeed + 2);
	oceanness = SmoothStep3(SmoothStep3(oceanness));
	float terrainHeight = RangeMap(terrainHeightNoise, 0.f, 1.f, RIVER_BOTTOM, MAX_MOUNTAIN_Z);
	if (terrainHeight > WATER_LEVEL)
	{
		//apply hilliness only above water level so that terrain cannot be sucked down below bodies of water
		terrainHeight = ((terrainHeight - WATER_LEVEL) * hilliness) + WATER_LEVEL;
	}

	// apply oceanness
	if (oceanness > .75f)
	{
		terrainHeight -= MAX_OCEAN_DEPTH;
		if (terrainHeight < 0)
		{
			terrainHeight = 0;
		}
	}
	if (oceanness > .5f && oceanness < .75f)
	{
		float OceanLoweringStrength = GetFractionWithinRange(oceanness, .5f, .75f);
		terrainHeight -= MAX_OCEAN_DEPTH * OceanLoweringStrength;
		if (terrainHeight < 0)
		{
			terrainHeight = 0;
		}
	}
	return terrainHeight;
}

float Chunk::GetHumidityFromGlobalXY(int globalX, int globalY)
{
	float humidity = .5f + .5f * Compute2dPerlinNoise((float)globalX, (float)globalY, 300.f, 3, .5f, 2.f, true, g_theWorld->m_worldSeed + 3);
	return humidity;
}

float Chunk::GetTemperatureFromGlobalXY(int globalX, int globalY)
{
	float temperature = .5f + .5f * Compute2dPerlinNoise((float)globalX, (float)globalY, 300.f, 7, .5f, 2.f, true, g_theWorld->m_worldSeed + 4);
	return temperature;
}

bool Chunk::ShouldColumnHaveCaveOrigin(int columnX, int columnY)
{
	float thisColumnValue = Get2dNoiseZeroToOne(columnX, columnY);

	//determine local maxima chunk coord for cave start
	for (int x = columnX - CAVE_LOCAL_MAXIMA_DIMENSIONS / 2; x <= columnX + CAVE_LOCAL_MAXIMA_DIMENSIONS / 2; x++)
	{
		for (int y = columnY - CAVE_LOCAL_MAXIMA_DIMENSIONS / 2; y <= columnY + CAVE_LOCAL_MAXIMA_DIMENSIONS / 2; y++)
		{
			if (x == columnX && y == columnY)
			{
				continue;
			}
			float currCoordValue = Get2dNoiseZeroToOne(x, y);
			if (currCoordValue >= thisColumnValue)
			{
				return false;
			}
		}
	}
	return true;
}

IntVec3 Chunk::GetCaveOriginFromChunkCoords(IntVec2 const& chunkCoords)
{
	//std::string chunkValue = Stringf("%.2f", caveMaximaValue);
	//Vec3 textPos = Vec3((float)(caveMaximaCoords.x * XSIZE) + XSIZE / 2.f, (float)(caveMaximaCoords.y * YSIZE) + YSIZE / 2.f, 72.f);
	//DebugAddWorldBillboardText(chunkValue, textPos, 1.f, g_theGame->m_player->GetForwardNormal().GetXY(), 100.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::ALWAYS);

	Vec3 normalizedCaveOrigin;
	normalizedCaveOrigin.x = Get2dNoiseZeroToOne(chunkCoords.x, chunkCoords.y, 1);
	normalizedCaveOrigin.y = Get2dNoiseZeroToOne(chunkCoords.x, chunkCoords.y, 2);
	normalizedCaveOrigin.z = Get2dNoiseZeroToOne(chunkCoords.x, chunkCoords.y, 3);

	IntVec3 worldCaveOrigin;
	Vec2 caveOriginChunkCenterXY = Vec2((float)(chunkCoords.x * XSIZE) + ((float)XSIZE * .5f), (float)(chunkCoords.y * YSIZE) + ((float)YSIZE * .5f));
	worldCaveOrigin.x = RoundDownToInt(Lerp(caveOriginChunkCenterXY.x - (float)XSIZE * .5f, caveOriginChunkCenterXY.x + (float)XSIZE * .5f, normalizedCaveOrigin.x));
	worldCaveOrigin.y = RoundDownToInt(Lerp(caveOriginChunkCenterXY.y - (float)YSIZE * .5f, caveOriginChunkCenterXY.y + (float)YSIZE * .5f, normalizedCaveOrigin.y));
	worldCaveOrigin.z = RoundDownToInt( Lerp(0.f, GetTerrainHeightFromGlobalXY(worldCaveOrigin.x, worldCaveOrigin.y), normalizedCaveOrigin.z) );
	return worldCaveOrigin;
}


bool Chunk::ParseBlockDataVersion2(std::vector<unsigned char> const& bufferData)
{
	int currBlockIdx = 0;
	unsigned int byte1 = (unsigned int)bufferData[8];
	unsigned int byte2 = (unsigned int)bufferData[9];
	unsigned int byte3 = (unsigned int)bufferData[10];
	unsigned int byte4 = (unsigned int)bufferData[11];

	unsigned int worldSeed = byte1 | (byte2 << 8) | (byte3 << 16) | (byte4 << 24);
	if (worldSeed != g_theWorld->m_worldSeed)
	{
		return false;
	}

	for (size_t typeIdx = 12; typeIdx < bufferData.size() - 1; typeIdx += 2)
	{
		BlockType currBlockType = (BlockType)bufferData[typeIdx];
		uint8_t runLength = bufferData[typeIdx + 1];
		for (int blockNum = 0; blockNum < (int)runLength; blockNum++)
		{
			m_blocks[currBlockIdx].m_blockType = currBlockType;
			currBlockIdx++;
		}
	}
	if (currBlockIdx != CHUNK_SIZE)
	{
		ERROR_RECOVERABLE(Stringf("Incorrect number of blocks read in from Saves/Chunk(%d,%d).chunk", m_chunkCoords.x, m_chunkCoords.y));
		return false;
	}
	return true;
}


void Chunk::GenerateBlockData()
{
	std::vector<ColumnData> columnData;
	columnData.resize(XSIZE * YSIZE);

	//calculate columnHeights
	for (int y = 0; y < YSIZE; y++)
	{
		for (int x = 0; x < XSIZE; x++)
		{
			int globalX = XSIZE * m_chunkCoords.x + x;
			int globalY = YSIZE * m_chunkCoords.y + y;

			columnData[x + (y * XSIZE)].m_height = (int)GetTerrainHeightFromGlobalXY(globalX, globalY);
			columnData[x + (y * XSIZE)].m_humidity = GetHumidityFromGlobalXY(globalX, globalY);
			columnData[x + (y * XSIZE)].m_temperature = GetTemperatureFromGlobalXY(globalX, globalY);	
		}
	}

	for (int z = 0; z < ZSIZE; z++)
	{
		for (int y = 0; y < YSIZE; y++)
		{
			for (int x = 0; x < XSIZE; x++)
			{
				DetermineBlockType(x, y, z, columnData);
			}
		}
	}

	//place trees
	m_blockTemplatesForChunk.clear();
	constexpr int TREE_SPACE = 3;
	for (int y = -TREE_SPACE; y < YSIZE + TREE_SPACE; y++)
	{
		for (int x = -TREE_SPACE; x < XSIZE + TREE_SPACE; x++)
		{
			int globalX = x + m_chunkCoords.x * XSIZE;
			int globalY = y + m_chunkCoords.y * YSIZE;
			if (ShouldColumnHaveTree(globalX, globalY))
			{
				int blockZ = (int)GetTerrainHeightFromGlobalXY(globalX, globalY);
				//make sure tree is not under water
				if (blockZ < SEA_LEVEL)
				{
					break;
				}

				float humidity = GetHumidityFromGlobalXY(globalX, globalY);
				float temperature = GetTemperatureFromGlobalXY(globalX, globalY);

				if (temperature > .5 && humidity < DIRT_TO_SAND_THRESHOLD)
				{
					BlockTemplate currentTreeTemplate = g_theWorld->m_cactusTemplate;
					currentTreeTemplate.m_templateOrigin = IntVec3(x, y, blockZ + 1);
					m_blockTemplatesForChunk.push_back(currentTreeTemplate);
				}

				else if (temperature < WATER_TO_ICE_TEMP && humidity > DIRT_TO_SAND_THRESHOLD)
				{
					BlockTemplate currentTreeTemplate = g_theWorld->m_spruceTreeTemplate;
					currentTreeTemplate.m_templateOrigin = IntVec3(x, y, blockZ + 1);
					m_blockTemplatesForChunk.push_back(currentTreeTemplate);
				}
				else if (temperature > WATER_TO_ICE_TEMP && humidity > DIRT_TO_SAND_THRESHOLD)
				{
					BlockTemplate currentTreeTemplate = g_theWorld->m_oakTreeTemplate;
					currentTreeTemplate.m_templateOrigin = IntVec3(x, y, blockZ + 1);
					m_blockTemplatesForChunk.push_back(currentTreeTemplate);
				}
			}
		}
	}


}

void Chunk::DetermineBlockType(int x, int y, int z, std::vector<ColumnData> const& columnData)
{
	int columnIdx = x | (y << XBITS);
	int columnHeight = columnData[columnIdx].m_height;
	ColumnData const& columnEntry = columnData[columnIdx];
	int dirthDepth = g_randGen->RollRandomIntInRange(3, 4);
	float maxIceDepth = 5.f;
	if (z > columnHeight)
	{
		//water
		if (z <= SEA_LEVEL)
		{
			int iceDepth = RoundDownToInt(RangeMap(columnEntry.m_temperature, WATER_TO_ICE_TEMP, WATER_TO_ICE_TEMP * .33f, 0.f, (float)maxIceDepth + 1.f));
			if (z + iceDepth > SEA_LEVEL)
			{
				m_blocks[GetChunkIndexFromCoords(x, y, z)] = 12;
			}
			else
			{
				m_blocks[GetChunkIndexFromCoords(x, y, z)] = 8;
			}
		}
		//air
		else
		{
			m_blocks[GetChunkIndexFromCoords(x, y, z)] = 0;
		}
	}
	//surface
	else if (z == columnHeight)
	{
		//beach
		if (z == SEA_LEVEL)
		{
			if (columnEntry.m_humidity >= BEACH_HUMIDITY)
			{
				//grass
				m_blocks[GetChunkIndexFromCoords(x, y, z)] = 3;
			}
			else
			{
				//sand
				m_blocks[GetChunkIndexFromCoords(x, y, z)] = 10;
			}
		}
		else
		{
			if (columnEntry.m_humidity <= DIRT_TO_SAND_THRESHOLD && columnEntry.m_height <= MAX_SAND_HEIGHT)
			{
				//sand
				m_blocks[GetChunkIndexFromCoords(x, y, z)] = 10;
			}
			else
			{
				//grass
				m_blocks[GetChunkIndexFromCoords(x, y, z)] = 3;
			}
		}
	}
	else if (z + dirthDepth >= columnHeight)
	{
		int sandDepth = RoundDownToInt(RangeMap(columnEntry.m_humidity, DIRT_TO_SAND_THRESHOLD, DIRT_TO_SAND_THRESHOLD * .33f, 0.f, (float)dirthDepth + 1.f));
		if (columnEntry.m_humidity <= DIRT_TO_SAND_THRESHOLD && columnEntry.m_height <= MAX_SAND_HEIGHT && z + sandDepth >= columnHeight)
		{
			m_blocks[GetChunkIndexFromCoords(x, y, z)] = 10;
		}
		else
		{
			m_blocks[GetChunkIndexFromCoords(x, y, z)] = 2;
		}
	}
	//coal
	else if (g_randGen->RollRandomFloatZeroToOne() < .05)
	{
		m_blocks[GetChunkIndexFromCoords(x, y, z)] = 4;
	}
	//iron
	else if (g_randGen->RollRandomFloatZeroToOne() < .02)
	{
		m_blocks[GetChunkIndexFromCoords(x, y, z)] = 5;
	}
	//gold
	else if (g_randGen->RollRandomFloatZeroToOne() < .005)
	{
		m_blocks[GetChunkIndexFromCoords(x, y, z)] = 6;
	}
	//diamond
	else if (g_randGen->RollRandomFloatZeroToOne() < .001)
	{
		m_blocks[GetChunkIndexFromCoords(x, y, z)] = 7;
	}
	//stone
	else
	{
		m_blocks[GetChunkIndexFromCoords(x, y, z)] = 1;
	}
}

bool Chunk::ShouldColumnHaveTree(int globalX, int globalY)
{
	float treeDensity = .5f + .5f * Compute2dPerlinNoise((float)globalX, (float)globalY, 150.f, 5, .5f, 2.f, true, g_theWorld->m_worldSeed + 5);
	float treeNoiseMinimum = RangeMapClamped(treeDensity, .5f, 1.f, 1.f, .7f);
	treeNoiseMinimum = SmoothStop2(treeNoiseMinimum);
	float myNoise = Get2dNoiseZeroToOne(globalX, globalY, g_theWorld->m_worldSeed + 5);
	if (myNoise < treeNoiseMinimum)
	{
		return false;
	}
	for (int x = globalX - 2; x <= globalX + 2; x++)
	{
		for (int y = globalY - 2; y <= globalY + 2; y++)
		{
			float neighborNoise = Get2dNoiseZeroToOne(x, y, g_theWorld->m_worldSeed + 5);
			if (neighborNoise > myNoise)
			{
				return false;
			}
		}
	}
	return true;
}

void Chunk::InitializeLightingData()
{
	//mark non opaque boundary blocks connecting existing neighbors as sky
	
	//west
	if (m_westNeighbor != nullptr)
	{
		for (int z = 0; z < ZSIZE; z++)
		{
			for (int y = 0; y < YSIZE; y++)
			{
				Block& currentBlock = GetBlockFromCoords(0, y, z);
				if (!currentBlock.IsBlockOpaque())
				{
					//m_localDirtyLighting.push_back(BlockIterator(this, GetChunkIndexFromCoords(0, y, z)));
					g_theWorld->DirtyBlockLighting(BlockIterator(this, GetChunkIndexFromCoords(0, y, z)));
				}
			}
		}
	}

	//east
	if (m_eastNeighbor != nullptr)
	{
		for (int z = 0; z < ZSIZE; z++)
		{
			for (int y = 0; y < YSIZE; y++)
			{
				Block& currentBlock = GetBlockFromCoords(XSIZE - 1, y, z);
				if (!currentBlock.IsBlockOpaque())
				{
					//m_localDirtyLighting.push_back(BlockIterator(this, GetChunkIndexFromCoords(XSIZE - 1, y, z)));
					g_theWorld->DirtyBlockLighting(BlockIterator(this, GetChunkIndexFromCoords(XSIZE - 1, y, z)));
				}
			}
		}
	}

	//north
	if (m_northNeighbor != nullptr)
	{
		for (int z = 0; z < ZSIZE; z++)
		{
			for (int x = 0; x < XSIZE; x++)
			{
				Block& currentBlock = GetBlockFromCoords(x, YSIZE - 1, z);
				if (!currentBlock.IsBlockOpaque())
				{
					//m_localDirtyLighting.push_back(BlockIterator(this, GetChunkIndexFromCoords(x, YSIZE - 1, z)));
					g_theWorld->DirtyBlockLighting(BlockIterator(this, GetChunkIndexFromCoords(x, YSIZE - 1, z)));
				}
			}
		}
	}

	//south
	if (m_southNeighbor != nullptr)
	{
		for (int z = 0; z < ZSIZE; z++)
		{
			for (int x = 0; x < XSIZE; x++)
			{
				Block& currentBlock = GetBlockFromCoords(x, 0, z);
				if (!currentBlock.IsBlockOpaque())
				{
					//m_localDirtyLighting.push_back(BlockIterator(this, GetChunkIndexFromCoords(x, 0, z)));
					g_theWorld->DirtyBlockLighting(BlockIterator(this, GetChunkIndexFromCoords(x, 0, z)));
				}
			}
		}
	}
	
	// Descend each column downward from the top, flagging blocks as SKY, stopping at first opaque
	for (int y = 0; y < YSIZE; y++)
	{
		for (int x = 0; x < XSIZE; x++)
		{
			BlockIterator currSkyBlock(this, GetChunkIndexFromCoords(x, y, ZSIZE - 1));
			while (currSkyBlock.GetBlock() != nullptr && !currSkyBlock.GetBlock()->IsBlockOpaque())
			{
				currSkyBlock.GetBlock()->SetIsBlockSky(true);
				currSkyBlock = currSkyBlock.GetBottomNeighbor();
			}
		}
	}
	
	// Descend each column again until first opaque; set each sky block’s outdoor light influence to maximum (15), AND mark its non-opaque non-sky horizontal (NSEW only) neighbors dirty
	for (int y = 0; y < YSIZE; y++)
	{
		for (int x = 0; x < XSIZE; x++)
		{
			BlockIterator currSkyBlock(this, GetChunkIndexFromCoords(x, y, ZSIZE - 1));
			while (currSkyBlock.GetBlock() != nullptr && currSkyBlock.GetBlock()->IsBlockSky())
			{
				currSkyBlock.GetBlock()->SetOutdoorLightInfluence(15);

				BlockIterator eastNeighbor = currSkyBlock.GetEastNeighbor();
				if (eastNeighbor.IsValid() && !eastNeighbor.GetBlock()->IsBlockSky() && !eastNeighbor.GetBlock()->IsBlockOpaque())
				{
					//m_localDirtyLighting.push_back(eastNeighbor);
					g_theWorld->DirtyBlockLighting(eastNeighbor);
				}

				BlockIterator westNeighbor = currSkyBlock.GetWestNeighbor();
				if (westNeighbor.IsValid() && !westNeighbor.GetBlock()->IsBlockSky() && !westNeighbor.GetBlock()->IsBlockOpaque())
				{
					//m_localDirtyLighting.push_back(westNeighbor);
					g_theWorld->DirtyBlockLighting(westNeighbor);
				}

				BlockIterator southNeighbor = currSkyBlock.GetSouthNeighbor();
				if (southNeighbor.IsValid() && !southNeighbor.GetBlock()->IsBlockSky() && !southNeighbor.GetBlock()->IsBlockOpaque())
				{
					//m_localDirtyLighting.push_back(southNeighbor);
					g_theWorld->DirtyBlockLighting(southNeighbor);
				}

				BlockIterator northNeighbor = currSkyBlock.GetNorthNeighbor();
				if (northNeighbor.IsValid() && !northNeighbor.GetBlock()->IsBlockSky() && !northNeighbor.GetBlock()->IsBlockOpaque())
				{
					//m_localDirtyLighting.push_back(northNeighbor);
					g_theWorld->DirtyBlockLighting(northNeighbor);
				}
				
				currSkyBlock = currSkyBlock.GetBottomNeighbor();
			}
		}
	}
	
	// Loop through each block in the chunk; if it has a block type that emits light, mark it dirty
	for (int i = 0; i < CHUNK_SIZE; i++)
	{
		if (m_blocks[i].GetBlockDef()->m_indoorLightEmitted > 0)
		{
			//m_localDirtyLighting.push_back(BlockIterator(this, i));
			g_theWorld->DirtyBlockLighting(BlockIterator(this, i));
		}
	}
}

Rgba8 Chunk::GetColorForBlock(BlockIterator blockIter)
{
	Rgba8 color(0,0,127);
	Block* block = blockIter.GetBlock();
	if (block == nullptr)
	{
		return color;
	}
	float normalizedR = RangeMap((float)block->GetOutdoorLightInfluence(), 0, 15, 0, 1);
	color.r = DenormalizeByte(normalizedR);
	if (block->GetIndoorLightInfluence() > 0)
	{
		float normalizedG = RangeMap((float)block->GetIndoorLightInfluence(), 0, 15, 0, 1);
		color.g = DenormalizeByte(normalizedG);
	}
	return color;
}

Chunk::~Chunk()
{
	g_theWorld->UndirtyAllBlocksInChunk(this);
	RemoveNeighbors();
	/*
	if (m_needsSaving)
	{
		SaveChunkToDisk();
	}
	*/
	delete m_gpuVerts;
	delete[] m_blocks;
}

void Chunk::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (m_isMeshDirty && !g_theWorld->m_chunkMeshRebuiltThisFrame)
	{
		if (m_eastNeighbor != nullptr && m_westNeighbor != nullptr && m_northNeighbor != nullptr && m_southNeighbor != nullptr)
		{
			RebuildMesh();
			m_isMeshDirty = false;
			g_theWorld->m_chunkMeshRebuiltThisFrame = true;
		}
	}
}

void Chunk::Render()
{
	if (m_gpuVerts == nullptr)
	{
		return;
	}
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(SPRITE_SHEET_BASIC_SPRITES->GetTexture());
	g_theRenderer->DrawVertexBuffer(m_gpuVerts, (int)m_cpuVerts.size(), 0, VertexType::VERTEX_TYPE_PCU);
	if (g_theApp->m_debugMode)
	{
		DebugAddWorldLine(	Vec3(0.f + (float)(m_chunkCoords.x * XSIZE), 0.f + (float)(m_chunkCoords.y * YSIZE), (float)0.f),
							Vec3(0.f + (float)(m_chunkCoords.x * XSIZE), 0.f + (float)(m_chunkCoords.y * YSIZE), (float)ZSIZE), .05f, 0.f);
		DebugAddWorldLine(	Vec3((float)XSIZE + (float)(m_chunkCoords.x * XSIZE), 0.f + (float)(m_chunkCoords.y * YSIZE), (float)0.f),
							Vec3((float)XSIZE + (float)(m_chunkCoords.x * XSIZE), 0.f + (float)(m_chunkCoords.y * YSIZE), (float)ZSIZE), .05f, 0.f);
		DebugAddWorldLine(	Vec3(0.f + (float)(m_chunkCoords.x * XSIZE), (float)YSIZE + (float)(m_chunkCoords.y * YSIZE), (float)0.f),
							Vec3(0.f + (float)(m_chunkCoords.x * XSIZE), (float)YSIZE + (float)(m_chunkCoords.y * YSIZE), (float)ZSIZE), .05f, 0.f);
		DebugAddWorldLine(	Vec3((float)XSIZE + (float)(m_chunkCoords.x * XSIZE), (float)YSIZE + (float)(m_chunkCoords.y * YSIZE), (float)0.f),
							Vec3((float)XSIZE + (float)(m_chunkCoords.x * XSIZE), (float)YSIZE + (float)(m_chunkCoords.y * YSIZE), (float)ZSIZE), .05f, 0.f);

		DebugAddWorldLine(	Vec3(0.f + (float)(m_chunkCoords.x * XSIZE), 0.f + (float)(m_chunkCoords.y * YSIZE), 0.f),
							Vec3((float)XSIZE + (float)(m_chunkCoords.x * XSIZE), 0.f + (float)(m_chunkCoords.y * YSIZE), 0.f), .05f, 0.f);
		DebugAddWorldLine(	Vec3(0.f + (float)(m_chunkCoords.x * XSIZE), (float)YSIZE + (float)(m_chunkCoords.y * YSIZE), 0.f),
							Vec3((float)XSIZE + (float)(m_chunkCoords.x * XSIZE), (float)YSIZE + (float)(m_chunkCoords.y * YSIZE), 0.f), .05f, 0.f);
		DebugAddWorldLine(	Vec3(0.f + (float)(m_chunkCoords.x * XSIZE), 0.f + (float)(m_chunkCoords.y * YSIZE), 0.f),
							Vec3(0.f + (float)(m_chunkCoords.x * XSIZE), (float)YSIZE + (float)(m_chunkCoords.y * YSIZE), 0.f), .05f, 0.f);
		DebugAddWorldLine(	Vec3((float)XSIZE + (float)(m_chunkCoords.x * XSIZE), 0.f + (float)(m_chunkCoords.y * YSIZE), 0.f),
							Vec3((float)XSIZE + (float)(m_chunkCoords.x * XSIZE), (float)YSIZE + (float)(m_chunkCoords.y * YSIZE), 0.f), .05f, 0.f);
		
		DebugAddWorldLine(	Vec3(0.f + (float)(m_chunkCoords.x * XSIZE), 0.f + (float)(m_chunkCoords.y * YSIZE), (float)ZSIZE),
							Vec3((float)XSIZE + (float)(m_chunkCoords.x * XSIZE), 0.f + (float)(m_chunkCoords.y * YSIZE), (float)ZSIZE), .05f, 0.f);
		DebugAddWorldLine(	Vec3(0.f + (float)(m_chunkCoords.x * XSIZE), (float)YSIZE + (float)(m_chunkCoords.y * YSIZE), (float)ZSIZE),
							Vec3((float)XSIZE + (float)(m_chunkCoords.x * XSIZE), (float)YSIZE + (float)(m_chunkCoords.y * YSIZE), (float)ZSIZE), .05f, 0.f);
		DebugAddWorldLine(	Vec3(0.f + (float)(m_chunkCoords.x * XSIZE), 0.f + (float)(m_chunkCoords.y * YSIZE), (float)ZSIZE),
							Vec3(0.f + (float)(m_chunkCoords.x * XSIZE), (float)YSIZE + (float)(m_chunkCoords.y * YSIZE), (float)ZSIZE), .05f, 0.f);
		DebugAddWorldLine(	Vec3((float)XSIZE + (float)(m_chunkCoords.x * XSIZE), 0.f + (float)(m_chunkCoords.y * YSIZE), (float)ZSIZE),
							Vec3((float)XSIZE + (float)(m_chunkCoords.x * XSIZE), (float)YSIZE + (float)(m_chunkCoords.y * YSIZE), (float)ZSIZE), .05f, 0.f);
	}
}

Block& Chunk::GetBlockFromCoords(int x, int y, int z)
{
	return m_blocks[GetChunkIndexFromCoords(x, y, z)];
}


int Chunk::GetBlockIndexFromWorldPos(Vec3 const& worldPos)
{
	Vec2 minBlockXY((float)(XSIZE * m_chunkCoords.x), (float)(YSIZE * m_chunkCoords.y));
	Vec2 locaPos(worldPos.x - minBlockXY.x, worldPos.y - minBlockXY.y);
	locaPos.x = Clamp(locaPos.x, 0.f, 15.99f);
	locaPos.y = Clamp(locaPos.y, 0.f, 15.99f);
	IntVec2 blockCoordsXY((int)locaPos.x, (int)locaPos.y);
	IntVec3 blockCoords(blockCoordsXY.x, blockCoordsXY.y, (int)worldPos.z);
	return GetChunkIndexFromCoords(blockCoords);
}

void Chunk::RemoveNeighbors()
{
	if (m_eastNeighbor != nullptr)
	{
		m_eastNeighbor->m_westNeighbor = nullptr;
	}
	m_eastNeighbor = nullptr;
	if (m_westNeighbor != nullptr)
	{
		m_westNeighbor->m_eastNeighbor = nullptr;
	}
	m_westNeighbor = nullptr;
	if (m_northNeighbor != nullptr)
	{
		m_northNeighbor->m_southNeighbor = nullptr;
	}
	m_northNeighbor = nullptr;
	if (m_southNeighbor != nullptr)
	{
		m_southNeighbor->m_northNeighbor = nullptr;
	}
	m_southNeighbor = nullptr;
}

void Chunk::SaveChunkToDisk()
{
	std::vector<uint8_t> saveData;
	saveData.reserve(CHUNK_SIZE + 12);
	saveData.push_back('G');
	saveData.push_back('C');
	saveData.push_back('H');
	saveData.push_back('K');
	saveData.push_back(2);
	saveData.push_back(XBITS);
	saveData.push_back(YBITS);
	saveData.push_back(ZBITS);
	unsigned char bytes1 = (unsigned char)(g_theWorld->m_worldSeed & 0x000000ff);
	unsigned int byte2Masked = (g_theWorld->m_worldSeed & 0x0000ff00);
	unsigned char bytes2 = (unsigned char)(byte2Masked >> 8);
	unsigned int byte3Masked = (g_theWorld->m_worldSeed & 0x00ff0000);
	unsigned char bytes3 = (unsigned char)(byte3Masked >> 16);
	unsigned int byte4Masked = (g_theWorld->m_worldSeed & 0xff000000);
	unsigned char bytes4 = (unsigned char)(byte4Masked >> 24);
	saveData.push_back(bytes1);
	saveData.push_back(bytes2);
	saveData.push_back(bytes3);
	saveData.push_back(bytes4);
	BlockType currBlockType = 0;
	int numBlocksOfCurrType = 0;
	int blocksSaved = 0;
	for (int i = 0; i < CHUNK_SIZE; i++)
	{
		if (numBlocksOfCurrType == 0)
		{
			currBlockType = m_blocks[i].m_blockType;
		}
		bool runContinues = false;
		if (currBlockType == m_blocks[i].m_blockType)
		{
			numBlocksOfCurrType++;
			runContinues = true;
		}
		if (!runContinues)
		{
			blocksSaved += numBlocksOfCurrType;
			saveData.push_back((uint8_t)currBlockType);
			saveData.push_back((uint8_t)numBlocksOfCurrType);
			numBlocksOfCurrType = 1;
			currBlockType = m_blocks[i].m_blockType;
		}
		else if (numBlocksOfCurrType == 255)
		{
			blocksSaved += numBlocksOfCurrType;
			saveData.push_back((uint8_t)currBlockType);
			saveData.push_back((uint8_t)numBlocksOfCurrType);
			numBlocksOfCurrType = 0;
		}
	}
	//be sure to save last run of blocks
	if (numBlocksOfCurrType > 0)
	{
		blocksSaved += numBlocksOfCurrType;
		saveData.push_back((uint8_t)currBlockType);
		saveData.push_back((uint8_t)numBlocksOfCurrType);
	}

	if (blocksSaved != CHUNK_SIZE)
	{
		ERROR_AND_DIE("DID NOT SAVE CORRECT NUMBER OF BLOCKS TO DISK");
	}

	namespace fs = std::filesystem;
	// Check if src folder exists
	std::string folderName = Stringf("Saves/World_%u", g_theWorld->m_worldSeed);
	if (!fs::is_directory(folderName) || !fs::exists(folderName)) 
	{ 
		fs::create_directory(folderName); // create src folder
	}

	if ( !WriteBufferToFile(saveData, Stringf("Saves/World_%u/Chunk(%d,%d).chunk",g_theWorld->m_worldSeed, m_chunkCoords.x, m_chunkCoords.y)) )
	{
		ERROR_RECOVERABLE("Incorrect number of bytes written to file");
	}
}

void Chunk::SetNeighboringChunks()
{
	//NORTH
	m_northNeighbor = g_theWorld->GetChunkAtCoords(IntVec2(m_chunkCoords.x, m_chunkCoords.y + 1));
	if (m_northNeighbor != nullptr)
	{
		m_northNeighbor->m_southNeighbor = this;
	}
	//EAST
	m_eastNeighbor = g_theWorld->GetChunkAtCoords(IntVec2(m_chunkCoords.x + 1, m_chunkCoords.y));
	if (m_eastNeighbor != nullptr)
	{
		m_eastNeighbor->m_westNeighbor = this;
	}
	//SOUTH
	m_southNeighbor = g_theWorld->GetChunkAtCoords(IntVec2(m_chunkCoords.x, m_chunkCoords.y - 1));
	if (m_southNeighbor != nullptr)
	{
		m_southNeighbor->m_northNeighbor = this;
	}
	//WEST
	m_westNeighbor = g_theWorld->GetChunkAtCoords(IntVec2(m_chunkCoords.x - 1, m_chunkCoords.y));
	if (m_westNeighbor != nullptr)
	{
		m_westNeighbor->m_eastNeighbor = this;
	}
}

void Chunk::AddLoaclDirtyBlocksToGloablLightingQueue()
{
	while (m_localDirtyLighting.size() > 0)
	{
		g_theWorld->DirtyBlockLighting(m_localDirtyLighting.front());
		m_localDirtyLighting.pop_front();
	}
}

Vec2 Chunk::GetGlobalChunkCenterXY()
{
	return Vec2((float)(m_chunkCoords.x * XSIZE) + ((float)XSIZE * .5f), (float)(m_chunkCoords.y * YSIZE) + ((float)YSIZE * .5f));
}

Vec3 Chunk::GetWorldPosFromBlockCoords(int blockX, int blockY, int blockZ)
{
	return Vec3((float)(m_chunkCoords.x * XSIZE + blockX + .5f), (float)(m_chunkCoords.y * YSIZE + blockY + .5f), (float)blockZ + .5f);
}

void Chunk::AddVertsForBlock(BlockType blockType, IntVec3 const& blockCoords, std::vector<bool> const& visibleFaces)
{
	BlockDefinition* blockDef = BlockDefinition::GetBlockDefinitionByType(blockType);
	IntVec3 baseWorldCoords(m_chunkCoords.x * XSIZE, m_chunkCoords.y * YSIZE, 0);
	Vec3 worldPos = (baseWorldCoords + blockCoords).GetVec3();
	BlockIterator currBlockIter = BlockIterator(this, GetChunkIndexFromCoords(blockCoords));

	if (visibleFaces[(int)BlockFace::WEST])
	{
		AddVertsForQuad3D(m_cpuVerts, worldPos + Vec3(0.f, 1.f, 0.f), worldPos + Vec3(0.f, 0.f, 0.f), worldPos + Vec3(0.f, 0.f, 1.f), worldPos + Vec3(0.f, 1.f, 1.f), GetColorForBlock(currBlockIter.GetWestNeighbor()), blockDef->m_sideSpriteDef->GetUVs());
	}
	if (visibleFaces[(int)BlockFace::EAST])
	{
		AddVertsForQuad3D(m_cpuVerts, worldPos + Vec3(1.f, 0.f, 0.f), worldPos + Vec3(1.f, 1.f, 0.f), worldPos + Vec3(1.f, 1.f, 1.f), worldPos + Vec3(1.f, 0.f, 1.f), GetColorForBlock(currBlockIter.GetEastNeighbor()), blockDef->m_sideSpriteDef->GetUVs());
	}
	if (visibleFaces[(int)BlockFace::NORTH])
	{
		AddVertsForQuad3D(m_cpuVerts, worldPos + Vec3(1.f, 1.f, 0.f), worldPos + Vec3(0.f, 1.f, 0.f), worldPos + Vec3(0.f, 1.f, 1.f), worldPos + Vec3(1.f, 1.f, 1.f), GetColorForBlock(currBlockIter.GetNorthNeighbor()), blockDef->m_sideSpriteDef->GetUVs());
	}
	if (visibleFaces[(int)BlockFace::SOUTH])
	{
		AddVertsForQuad3D(m_cpuVerts, worldPos + Vec3(0.f, 0.f, 0.f), worldPos + Vec3(1.f, 0.f, 0.f), worldPos + Vec3(1.f, 0.f, 1.f), worldPos + Vec3(0.f, 0.f, 1.f), GetColorForBlock(currBlockIter.GetSouthNeighbor()), blockDef->m_sideSpriteDef->GetUVs());
	}
	if (visibleFaces[(int)BlockFace::TOP])
	{
		IntVec3 topCoords = IntVec3(blockCoords.x, blockCoords.y, blockCoords.z + 1);
		AddVertsForQuad3D(m_cpuVerts, worldPos + Vec3(0.f, 1.f, 1.f), worldPos + Vec3(0.f, 0.f, 1.f), worldPos + Vec3(1.f, 0.f, 1.f), worldPos + Vec3(1.f, 1.f, 1.f), GetColorForBlock(currBlockIter.GetTopNeighbor()), blockDef->m_topSpriteDef->GetUVs());
	}
	if (visibleFaces[(int)BlockFace::BOTTOM])
	{
		IntVec3 bottomCoords = IntVec3(blockCoords.x, blockCoords.y, blockCoords.z - 1);
		AddVertsForQuad3D(m_cpuVerts, worldPos + Vec3(1.f, 1.f, 0.f), worldPos + Vec3(1.f, 0.f, 0.f), worldPos + Vec3(0.f, 0.f, 0.f), worldPos + Vec3(0.f, 1.f, 0.f), GetColorForBlock(currBlockIter.GetBottomNeighbor()), blockDef->m_bottomSpriteDef->GetUVs());
	}
}

void Chunk::AddDataForCave(std::map<IntVec3, std::vector<Vec3>>& mapsToCheck, IntVec3 const& caveOrigin)
{
	unsigned int yawSeed = (unsigned int)(caveOrigin.x + caveOrigin.y + caveOrigin.z);
	unsigned int pitchSeed = yawSeed + 1;
	constexpr int MAX_CAVE_SEGMENTS = 50;
	constexpr float MIN_YAW_DELTA = -60.f;
	constexpr float MAX_YAW_DELTA = 60.f;
	constexpr float MIN_PITCH_DELTA = -15.f;
	constexpr float MAX_PITCH_DELTA = 15.f;
	float noisePosition = 0;
	float yawScale = 100.f;
	float pitchScale = 100.f;
	//Forawrd dir
	EulerAngles caveStartForward;
	EulerAngles currCaveForward;


	float yawNoise = Compute1dPerlinNoise((float)(caveOrigin.x + caveOrigin.y + caveOrigin.z), yawScale, 1U, .5f, 2.f, true, yawSeed);
	caveStartForward.m_yaw = RangeMapClamped(yawNoise, -1.f, 1.f, 0.f, 360.f);

	float pitchNoise = Compute1dPerlinNoise((float)(caveOrigin.x + caveOrigin.y + caveOrigin.z), pitchScale, 1U, .5f, 2.f, true, pitchSeed);
	caveStartForward.m_pitch = RangeMapClamped(pitchNoise, -1.f, 1.f, -20, 20.f);
	caveStartForward.m_roll = 0.f;
	currCaveForward = caveStartForward;

	int currFowardSegments = 0;
	Vec3 caveStart = caveOrigin.GetVec3();
	Vec3 segmentStartPos = caveStart;
	mapsToCheck[caveOrigin].push_back(segmentStartPos);
	while (currFowardSegments < MAX_CAVE_SEGMENTS && GetDistanceSquared3D(segmentStartPos, caveStart) < MAX_CAVE_RADIUS * MAX_CAVE_RADIUS)
	{
		Vec3 segmentEndPos = segmentStartPos + currCaveForward.GetIFwd() * CAVE_SEGMENT_LENGTH;

		mapsToCheck[caveOrigin].push_back(segmentEndPos);
		segmentStartPos = segmentEndPos;
		noisePosition += CAVE_SEGMENT_LENGTH;
		currCaveForward.m_yaw += RangeMapClamped(Compute1dPerlinNoise(noisePosition, yawScale, 
			1U, .5f, 2.f, true, yawSeed), -1.f, 1.f, MIN_YAW_DELTA, MAX_YAW_DELTA);
		currCaveForward.m_pitch += RangeMapClamped(Compute1dPerlinNoise(noisePosition, pitchScale,
			1U, .5f, 2.f, true, pitchSeed), -1.f, 1.f, MIN_PITCH_DELTA, MAX_PITCH_DELTA);
		currFowardSegments++;
	}
		
}

void Chunk::CarveCaveInChunk(Vec3 const& capsuleStart, Vec3 const& capsuleEnd)
{
	for (int z = 0; z < ZSIZE; z++)
	{
		for (int y = 0; y < YSIZE; y++)
		{
			for (int x = 0; x < XSIZE; x++)
			{
				Vec3 blockPos = GetWorldPosFromBlockCoords(x, y, z);
				if (IsPointInsideCapsule3D(blockPos, capsuleStart, capsuleEnd, CAVE_CAPSULE_RADIUS) && z > 1)
				{
					if (m_blocks[GetChunkIndexFromCoords(IntVec3(x, y, z))].m_blockType != 8)
					{
						m_blocks[GetChunkIndexFromCoords(IntVec3(x, y, z))].m_blockType = 0;
					}
				}
			}
		}
	}
}

void Chunk::AddVertsForBlock(BlockType blockType, IntVec3 const& blockCoords)
{
	BlockDefinition* blockDef = BlockDefinition::GetBlockDefinitionByType(blockType);
	IntVec3 baseWorldCoords(m_chunkCoords.x * XSIZE, m_chunkCoords.y * YSIZE, 0);
	Vec3 worldPos = (baseWorldCoords + blockCoords).GetVec3();

	//WEST
	AddVertsForQuad3D(m_cpuVerts, worldPos + Vec3(0.f, 1.f, 0.f), worldPos + Vec3(0.f, 0.f, 0.f), worldPos + Vec3(0.f, 0.f, 1.f), worldPos + Vec3(0.f, 1.f, 1.f), Rgba8(230, 230, 230), blockDef->m_sideSpriteDef->GetUVs());
	//EAST
	AddVertsForQuad3D(m_cpuVerts, worldPos + Vec3(1.f, 0.f, 0.f), worldPos + Vec3(1.f, 1.f, 0.f), worldPos + Vec3(1.f, 1.f, 1.f), worldPos + Vec3(1.f, 0.f, 1.f), Rgba8(230, 230, 230), blockDef->m_sideSpriteDef->GetUVs());
	//NORTH
	AddVertsForQuad3D(m_cpuVerts, worldPos + Vec3(1.f, 1.f, 0.f), worldPos + Vec3(0.f, 1.f, 0.f), worldPos + Vec3(0.f, 1.f, 1.f), worldPos + Vec3(1.f, 1.f, 1.f), Rgba8(200, 200, 200), blockDef->m_sideSpriteDef->GetUVs());
	//SOUTH
	AddVertsForQuad3D(m_cpuVerts, worldPos + Vec3(0.f, 0.f, 0.f), worldPos + Vec3(1.f, 0.f, 0.f), worldPos + Vec3(1.f, 0.f, 1.f), worldPos + Vec3(0.f, 0.f, 1.f), Rgba8(200, 200, 200), blockDef->m_sideSpriteDef->GetUVs());
	//TOP
	AddVertsForQuad3D(m_cpuVerts, worldPos + Vec3(0.f, 1.f, 1.f), worldPos + Vec3(0.f, 0.f, 1.f), worldPos + Vec3(1.f, 0.f, 1.f), worldPos + Vec3(1.f, 1.f, 1.f), Rgba8::WHITE, blockDef->m_topSpriteDef->GetUVs());
	//BOTTOM
	AddVertsForQuad3D(m_cpuVerts, worldPos + Vec3(1.f, 1.f, 0.f), worldPos + Vec3(1.f, 0.f, 0.f), worldPos + Vec3(0.f, 0.f, 0.f), worldPos + Vec3(0.f, 1.f, 0.f), Rgba8::WHITE, blockDef->m_bottomSpriteDef->GetUVs());

}

void Chunk::RebuildMesh()
{
	if (m_cpuVerts.size() > 0)
	{
		m_cpuVerts.clear();
	}
	m_cpuVerts.reserve(CHUNK_SIZE);
	if (m_gpuVerts != nullptr)
	{
		delete m_gpuVerts;
		m_gpuVerts = nullptr;
	}
	
	Vec3 WEST_SOUTH_BOTTOM(-.5f, -.5f, -.5f);
	Vec3 EAST_SOUTH_BOTTOM(.5f, -.5f, -.5f);
	Vec3 WEST_NORTH_BOTTOM(-.5f, .5f, -.5f);
	Vec3 EAST_NORTH_BOTTOM(.5f, .5f, -.5f);

	Vec3 WEST_SOUTH_TOP(-.5f, -.5f, .5f);
	Vec3 EAST_SOUTH_TOP(.5f, -.5f, .5f);
	Vec3 WEST_NORTH_TOP(-.5f, .5f, .5f);
	Vec3 EAST_NORTH_TOP(.5f, .5f, .5f);
	
	for (int i = 0; i < CHUNK_SIZE; i++)
	{
		if (m_blocks[i].m_blockType == 0)
		{
			continue;
		}
		BlockDefinition* currentBlockDef = BlockDefinition::GetBlockDefinitionByType(m_blocks[i].m_blockType);
		BlockIterator currentBlockIter(this, i);
		Vec3 worldCenter = currentBlockIter.GetWorldCenter();

		BlockIterator westNeighbor = currentBlockIter.GetWestNeighbor();
		if (westNeighbor.IsValid() && westNeighbor.GetBlock()->m_blockType == 0)
		{
			AddVertsForQuad3D(m_cpuVerts, worldCenter + WEST_NORTH_BOTTOM, worldCenter + WEST_SOUTH_BOTTOM, worldCenter + WEST_SOUTH_TOP, worldCenter + WEST_NORTH_TOP,
				GetColorForBlock(westNeighbor), currentBlockDef->m_sideSpriteDef->GetUVs());
		}
		BlockIterator eastNeighbor = currentBlockIter.GetEastNeighbor();
		if (eastNeighbor.IsValid() && eastNeighbor.GetBlock()->m_blockType == 0)
		{
			AddVertsForQuad3D(m_cpuVerts, worldCenter + EAST_SOUTH_BOTTOM, worldCenter + EAST_NORTH_BOTTOM, worldCenter + EAST_NORTH_TOP, worldCenter + EAST_SOUTH_TOP,
				GetColorForBlock(eastNeighbor), currentBlockDef->m_sideSpriteDef->GetUVs());
		}
		BlockIterator northNeighbor = currentBlockIter.GetNorthNeighbor();
		if (northNeighbor.IsValid() && northNeighbor.GetBlock()->m_blockType == 0)
		{
			AddVertsForQuad3D(m_cpuVerts, worldCenter + EAST_NORTH_BOTTOM, worldCenter + WEST_NORTH_BOTTOM, worldCenter + WEST_NORTH_TOP, worldCenter + EAST_NORTH_TOP,
				GetColorForBlock(northNeighbor), currentBlockDef->m_sideSpriteDef->GetUVs());
		}
		BlockIterator southNeighbor = currentBlockIter.GetSouthNeighbor();
		if (southNeighbor.IsValid() && southNeighbor.GetBlock()->m_blockType == 0)
		{
			AddVertsForQuad3D(m_cpuVerts, worldCenter + WEST_SOUTH_BOTTOM, worldCenter + EAST_SOUTH_BOTTOM, worldCenter + EAST_SOUTH_TOP, worldCenter + WEST_SOUTH_TOP, 
				GetColorForBlock(southNeighbor), currentBlockDef->m_sideSpriteDef->GetUVs());
		}
		BlockIterator topNeighbor = currentBlockIter.GetTopNeighbor();
		if (topNeighbor.IsValid() && topNeighbor.GetBlock()->m_blockType == 0)
		{
			AddVertsForQuad3D(m_cpuVerts, worldCenter + WEST_NORTH_TOP, worldCenter + WEST_SOUTH_TOP, worldCenter + EAST_SOUTH_TOP, worldCenter + EAST_NORTH_TOP, 
				GetColorForBlock(topNeighbor), currentBlockDef->m_topSpriteDef->GetUVs());
		}
		BlockIterator bottomNeighbor = currentBlockIter.GetBottomNeighbor();
		if (bottomNeighbor.IsValid() && bottomNeighbor.GetBlock()->m_blockType == 0)
		{
			AddVertsForQuad3D(m_cpuVerts, worldCenter + EAST_NORTH_BOTTOM, worldCenter + EAST_SOUTH_BOTTOM, worldCenter + WEST_SOUTH_BOTTOM, worldCenter + WEST_NORTH_BOTTOM, 
				GetColorForBlock(bottomNeighbor), currentBlockDef->m_bottomSpriteDef->GetUVs());
		}
	}
	
	if (m_cpuVerts.size() == 0)
	{
		return;
	}
	size_t vertexBufferSize = sizeof(Vertex_PCU) * m_cpuVerts.size();
	m_gpuVerts = g_theRenderer->CreateVertexBuffer(vertexBufferSize);
	g_theRenderer->CopyCPUToGPU(m_cpuVerts.data(), vertexBufferSize, m_gpuVerts);
}

void Chunk::AddBlockToMesh(int x, int y, int z, bool useHSR)
{
	int blockIdx = GetChunkIndexFromCoords(x, y, z);
	BlockType blockType = m_blocks[blockIdx].m_blockType;
	if (blockType == 0)
	{
		return;
	}

	if (useHSR)
	{
		std::vector<bool> visibleFaces;
		visibleFaces.resize((int)BlockFace::COUNT, false);
		BlockIterator currentBlockIter(this, blockIdx);

		BlockIterator westNeighbor = currentBlockIter.GetWestNeighbor();
		if (westNeighbor.IsValid() && !westNeighbor.GetBlock()->IsBlockOpaque())
		{
			visibleFaces[(int)BlockFace::WEST] = true;
		}
		BlockIterator eastNeighbor = currentBlockIter.GetEastNeighbor();
		if (eastNeighbor.IsValid() && !eastNeighbor.GetBlock()->IsBlockOpaque())
		{
			visibleFaces[(int)BlockFace::EAST] = true;
		}
		BlockIterator southNeighbor = currentBlockIter.GetSouthNeighbor();
		if (southNeighbor.IsValid() && !southNeighbor.GetBlock()->IsBlockOpaque())
		{
			visibleFaces[(int)BlockFace::SOUTH] = true;
		}
		BlockIterator northNeighbor = currentBlockIter.GetNorthNeighbor();
		if (northNeighbor.IsValid() && !northNeighbor.GetBlock()->IsBlockOpaque())
		{
			visibleFaces[(int)BlockFace::NORTH] = true;
		}
		BlockIterator bottomNeighbor = currentBlockIter.GetBottomNeighbor();
		if (bottomNeighbor.IsValid() && !bottomNeighbor.GetBlock()->IsBlockOpaque())
		{
			visibleFaces[(int)BlockFace::BOTTOM] = true;
		}
		BlockIterator topNeighbor = currentBlockIter.GetTopNeighbor();
		if (topNeighbor.IsValid() && !topNeighbor.GetBlock()->IsBlockOpaque())
		{
			visibleFaces[(int)BlockFace::TOP] = true;
		}
		AddVertsForBlock(blockType, IntVec3(x, y, z), visibleFaces);
	}
	else
	{
		AddVertsForBlock(blockType, IntVec3(x, y, z));
	}
}
