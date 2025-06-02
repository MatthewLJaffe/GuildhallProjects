#include "Game/World.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"
#include "Game/ChunkJobGenerate.hpp"
#include "Game/ChunkJobReadFromDisk.hpp"
#include "Game/ChunkJobWriteToDisk.hpp"
#include <ctime>


static const int k_simpleMinerConstantsSlot = 5;

void World::StartUp()
{
	m_simpleMinerCBO = g_theRenderer->CreateConstantBuffer(sizeof(SimpleMinerConstants));
	//m_activeChunks[IntVec2(0, 0)] = new Chunk(IntVec2(0, 0));
	//m_activeChunks[IntVec2(2, 0)] = new Chunk(IntVec2(2, 0));
	//m_activeChunks[IntVec2(2, 1)] = new Chunk(IntVec2(2, 1));
	//m_activeChunks[IntVec2(2, -1)] = new Chunk(IntVec2(2, -1));
	m_debugTimer.Start();
	m_worldSeed = g_gameConfigBlackboard.GetValue("worldSeed", (unsigned int)0);
	if (m_worldSeed == 0)
	{
		srand((unsigned int)std::time(0));
		m_worldSeed = (unsigned int)g_randGen->RollRandomIntInRange(-INT_MIN/2, INT_MAX/2);
	}
	InitializeBlockTemplates();
}

World::World()
	: m_debugTimer(m_debugMessageTime)
{
	m_debugMessage = Stringf("FPS:%6.0f MS%4.0f Chunks: %d Blocks: %d Verts: %d", 1.f / g_theApp->GetGameClock()->GetDeltaSeconds(), g_theApp->GetTimeLastFrameMS(), 0, 0, 0);
}

void World::Update(float deltaSeconds)
{
	if (g_theInput->WasKeyJustReleased('R'))
	{
		m_isRaycastLocked = !m_isRaycastLocked;
	}
	if (!m_isRaycastLocked)
	{
		m_raycastDir = g_theGame->m_player->GetForwardNormal();
		m_raycastStarPos = g_theGame->m_player->m_position;
	}
	RaycastVsWorld(m_selectedBlock, m_raycastStarPos, m_raycastDir, 8.f);
	if (m_isRaycastLocked)
	{
		DebugAddWorldArrow(m_raycastStarPos, m_selectedBlock.m_impactPos, .01f, 0.f, Rgba8::MAGENTA, Rgba8::MAGENTA, DebugRenderMode::X_RAY);
		DebugAddWorldArrow(m_selectedBlock.m_impactPos, m_raycastStarPos + m_raycastDir * 8.f, .01f, 0.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
	}

	if (g_theInput->WasKeyJustPressed('1'))
	{
		m_blockTypeToPlace = 9;
	}
	if (g_theInput->WasKeyJustPressed('2'))
	{
		m_blockTypeToPlace = 11;
	}
	if (m_blockTypeToPlace == 9)
	{
		Vec2 screenDimensions = GetScreenDimensions();
		DebugAddScreenText("Block Place Type: Brick", Vec2(1200.f, 750.f), 20.f, Vec2(.5f, .5f), 0.f);
	}
	else if (m_blockTypeToPlace == 11)
	{
		DebugAddScreenText("Block Place Type: Glowstone", Vec2(1200.f, 750.f), 20.f, Vec2(.5f, .5f), 0.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		MineBlock();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
	{
		PlaceBlock(m_blockTypeToPlace);
	}


	float chunkActivationRange = g_gameConfigBlackboard.GetValue("chunkActivationRange", 250.f);
	int maxChunksRadiusX = 1 + ((int)chunkActivationRange) / XSIZE;
	int maxChunksRadiusY = 1 + ((int)chunkActivationRange) / YSIZE;
	int maxChunks = (2 * maxChunksRadiusX) * (2 * maxChunksRadiusY); // neighborhood
	int numChunks = (int)m_activeChunks.size();
	if (numChunks >= maxChunks)
	{
		DeactivateFurthestChunk();
	}
	else if (numChunks < maxChunks)
	{
		ActivateNearestChunk();
	}

	HandleReadyChunkJobs();

	ProcessDirtyLighting();

	
	for (int i = 0; i < m_maxMeshesToRebuildPerFrame; i++)
	{
		Chunk* chunkToRebuildMesh = GetClosestChunkToRebuildMesh();
		if (chunkToRebuildMesh != nullptr)
		{
			chunkToRebuildMesh->m_isMeshDirty = false;
			chunkToRebuildMesh->RebuildMesh();
		}
	}
	
	
	m_chunkMeshRebuiltThisFrame = false;
	size_t numVerts = 0;
	for (auto it = m_activeChunks.begin(); it != m_activeChunks.end(); it++)
	{
		//it->second->Update(deltaSeconds);
		numVerts += it->second->m_cpuVerts.size();
	}
	

	UpdateSimpleMinerConstants(deltaSeconds);

	int numBlocks = numChunks * XSIZE * YSIZE * ZSIZE;
	if (m_debugTimer.HasPeriodElapsed())
	{
		m_debugMessage = Stringf("FPS:%6.0f MS%4.1f Chunks: %d Blocks: %d Verts: %d", 1.f / g_theApp->GetGameClock()->GetDeltaSeconds(), g_theApp->GetTimeLastFrameMS(), numChunks, numBlocks, numVerts);
		m_debugTimer.Start();
	}
	//DebugAddScreenText(m_debugMessage, Vec2(50.f, 780.f), 20.f, Vec2(1.f, .5f), 0.f);
}

void World::Render()
{
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(SPRITE_SHEET_BASIC_SPRITES->GetTexture());
	g_theRenderer->BindShader(g_theRenderer->CreateOrGetShaderFromFile("Data/Shaders/World"));
	for (auto it = m_activeChunks.begin(); it != m_activeChunks.end(); it++)
	{
		it->second->Render();
	}
	if (m_selectedBlock.m_blockHit.GetBlock() != nullptr)
	{
		DrawBlockSelection();
	}
}

void World::ShutDown()
{
	//make sure all write jobs are finished so that all data is saved
	while (g_theJobSystem->GetQueuedJobsWithBit(JOB_WRITE_CHUNK_TO_DISK) > 0)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
	std::vector<Job*> completedJobs;
	g_theJobSystem->GetCompletedJobs(completedJobs);
	for (int i = 0; i < (int)completedJobs.size(); i++)
	{
		if (completedJobs[i]->m_jobTypeBit == JOB_WRITE_CHUNK_TO_DISK)
		{
			ChunkJobWriteToDisk* writeJob = dynamic_cast<ChunkJobWriteToDisk*>(completedJobs[i]);
			delete writeJob->m_chunk;
		}
		delete completedJobs[i];
	}
	

	delete m_simpleMinerCBO;

	for (auto it = m_activeChunks.begin(); it != m_activeChunks.end(); it++)
	{
		if (it->second->m_needsSaving)
		{
			it->second->SaveChunkToDisk();
		}
		delete it->second;
		it->second = nullptr;
	}
	m_activeChunks.clear();
}

void World::RebuildWorld()
{
	for (auto it = m_activeChunks.begin(); it != m_activeChunks.end(); it++)
	{
		delete it->second;
		it->second = nullptr;
	}
	m_activeChunks.clear();
	StartUp();
}

void World::QueueChunkForActivation(IntVec2 chunkCoords)
{
	//Chunk* chunkToActivate = new Chunk(chunkCoords);
	//m_activeChunks[chunkCoords] = chunkToActivate;
	
	Chunk* queuedChunk = new Chunk(chunkCoords);
	m_chunksQueuedForGeneration.insert(chunkCoords);
	if (std::filesystem::exists(Stringf("Saves/World_%u/Chunk(%d,%d).chunk", m_worldSeed, chunkCoords.x, chunkCoords.y)))
	{
		queuedChunk->m_chunkState = ChunkState::ACTIVATING_QUEUED_LOAD;
		ChunkJobReadFromDisk* readJob = new ChunkJobReadFromDisk(queuedChunk);
		g_theJobSystem->QueueJob(readJob);
	}
	else
	{
;		queuedChunk->m_chunkState = ChunkState::ACTIVATING_QUEUED_GENERATE;
		ChunkJobGenerate* generatorJob = new ChunkJobGenerate(queuedChunk);
		g_theJobSystem->QueueJob(generatorJob);
	}
	
}

void World::QueueChunkForDeactivation(IntVec2 chunkCoords)
{
	if (m_activeChunks.find(chunkCoords) == m_activeChunks.end())
	{
		return;
	}
	Chunk* chunkToDeactivate = m_activeChunks[chunkCoords];
	m_activeChunks.erase(chunkCoords);
	
	if (chunkToDeactivate->m_needsSaving)
	{
		ChunkJobWriteToDisk* writeToDiskJob = new ChunkJobWriteToDisk(chunkToDeactivate);
		g_theJobSystem->QueueJob(writeToDiskJob);
	}
	else
	{
		delete chunkToDeactivate;
	}
}

IntVec2 World::GetChunkCoordsFromWorldPos(Vec3 worldPos)
{
	IntVec2 chunkCoord;
	chunkCoord.x = RoundDownToInt(worldPos.x / (float)XSIZE);
	chunkCoord.y = RoundDownToInt(worldPos.y / (float)YSIZE);
	return chunkCoord;
}

Vec2 World::GetChunkCoordsCenter(IntVec2 chunkCoords)
{
	return Vec2((chunkCoords.x * (float)XSIZE) + ((float)XSIZE *.5f), (chunkCoords.y * (float)YSIZE) + ((float)YSIZE * .5f));
}

Chunk* World::GetChunkAtCoords(IntVec2 chunkCoords)
{
	if (m_activeChunks.find(chunkCoords) == m_activeChunks.end())
	{
		return nullptr;
	}
	return m_activeChunks[chunkCoords];
}

Chunk* World::GetClosestChunkToRebuildMesh()
{
	float nearestChunkDistanceSquared = 99999999999999999999.f;
	Chunk* nearestChunk = nullptr;
	for (auto chunkPair : m_activeChunks)
	{
		if (!chunkPair.second->m_isMeshDirty || 
			chunkPair.second->m_northNeighbor == nullptr || 
			chunkPair.second->m_eastNeighbor == nullptr || 
			chunkPair.second->m_southNeighbor == nullptr ||
			chunkPair.second->m_westNeighbor == nullptr)
		{
			continue;
		}
		Vec2 currCenter = GetChunkCoordsCenter(chunkPair.first);
		Vec2 playerPosXY = g_theGame->m_player->m_position.GetXY();
		float currDistSqured = GetDistanceSquared2D(playerPosXY, currCenter);
		if (currDistSqured < nearestChunkDistanceSquared)
		{
			currDistSqured = nearestChunkDistanceSquared;
			nearestChunk = chunkPair.second;
		}
	}
	return nearestChunk;
}

void World::RaycastVsWorld(RaycastResultSimpleMiner& result, Vec3 const& rayStartPos, Vec3 const& rayFwdNormal, float maxRaylength)
{
	//set up curr raycast
	result.m_rayFwdNormal = rayFwdNormal;
	result.m_rayMaxLength = maxRaylength;
	result.m_rayStartPos = rayStartPos;
	result.m_didImpact = false;
	result.m_impactDist = 0.f;
	result.m_impactNormal = Vec3::ZERO;
	result.m_blockHit = BlockIterator();

	BlockIterator startBlock = GetBlockIteratorForWorldPos(rayStartPos);
	if (!startBlock.IsValid())
	{
		return;
	}
	if (startBlock.GetBlock()->IsBlockSolid())
	{
		result.m_impactPos = rayStartPos;
		result.m_impactDist = 0;
		result.m_didImpact = true;
		result.m_impactNormal = -rayFwdNormal;
		result.m_rayFwdNormal = rayFwdNormal;
		result.m_rayMaxLength = maxRaylength;
		result.m_rayStartPos = rayStartPos;
		return;
	}

	int xStepDir = rayFwdNormal.x > 0 ? 1 : -1;
	int yStepDir = rayFwdNormal.y > 0 ? 1 : -1;
	int zStepDir = rayFwdNormal.z > 0 ? 1 : -1;

	float distanceLeft = maxRaylength;
	Vec3 currPos = rayStartPos;
	Vec3 rayEndPos = rayStartPos + rayFwdNormal * maxRaylength;
	IntVec3 currBlockCoords(RoundDownToInt(currPos.x), RoundDownToInt(currPos.y), RoundDownToInt(currPos.z));
	BlockIterator currentBlock = startBlock;
	while (distanceLeft > 0)
	{
		//figure out how long to travel along forward normal to reach next x and y
		int nextX = currBlockCoords.x + (xStepDir + 1) / 2;
		float distToNextX = static_cast<float>(nextX) - currPos.x;
		float forwardLenToNextX = fabsf(distToNextX / rayFwdNormal.x);

		int nextY = currBlockCoords.y + (yStepDir + 1) / 2;
		float distToNextY = static_cast<float>(nextY) - currPos.y;
		float forwardLenToNextY = fabsf(distToNextY / rayFwdNormal.y);

		int nextZ = currBlockCoords.z + (zStepDir + 1) / 2;
		float distToNextZ = static_cast<float>(nextZ) - currPos.z;
		float forwardLenToNextZ = fabsf(distToNextZ / rayFwdNormal.z);

		Vec3 nextStepDisplacment;
		if (forwardLenToNextX < forwardLenToNextY && forwardLenToNextX < forwardLenToNextZ)
		{
			nextStepDisplacment = forwardLenToNextX * rayFwdNormal;
			currBlockCoords.x += xStepDir;
			if (xStepDir > 0)
			{
				currentBlock = currentBlock.GetEastNeighbor();
			}
			else
			{
				currentBlock = currentBlock.GetWestNeighbor();
			}
		}
		else if (forwardLenToNextY < forwardLenToNextX && forwardLenToNextY < forwardLenToNextZ)
		{
			nextStepDisplacment = forwardLenToNextY * rayFwdNormal;
			currBlockCoords.y += yStepDir;
			if (yStepDir > 0)
			{
				currentBlock = currentBlock.GetNorthNeighbor();
			}
			else
			{
				currentBlock = currentBlock.GetSouthNeighbor();
			}
		}
		else
		{
			nextStepDisplacment = forwardLenToNextZ * rayFwdNormal;
			currBlockCoords.z += zStepDir;
			if (zStepDir > 0)
			{
				currentBlock = currentBlock.GetTopNeighbor();
			}
			else
			{
				currentBlock = currentBlock.GetBottomNeighbor();
			}
		}

		if (currentBlock.GetBlock() == nullptr)
		{
			return;
		}

		distanceLeft -= nextStepDisplacment.GetLength();
		if (distanceLeft < 0)
		{
			break;
		}
		currPos += nextStepDisplacment;

		if (currentBlock.GetBlock()->IsBlockSolid())
		{
			//crossing x
			if (forwardLenToNextX < forwardLenToNextY && forwardLenToNextX < forwardLenToNextZ)
			{
				if (xStepDir > 0)
				{
					result.m_impactNormal = Vec3(-1.f, 0.f, 0.f);
				}
				else
				{
					result.m_impactNormal = Vec3(1.f, 0.f, 0.f);
				}
			}
			//crossing y
			else if (forwardLenToNextY < forwardLenToNextX && forwardLenToNextY < forwardLenToNextZ)
			{
				if (yStepDir > 0)
				{
					result.m_impactNormal = Vec3(0.f, -1.f, 0.f);
				}
				else
				{
					result.m_impactNormal = Vec3(0.f, 1.f, 0.f);
				}
			}
			//crossing z
			else
			{
				if (zStepDir > 0)
				{
					result.m_impactNormal = Vec3(0.f, 0.f, -1.f);
				}
				else
				{
					result.m_impactNormal = Vec3(0.f, 0.f, 1.f);
				}
			}
			result.m_impactPos = currPos;
			result.m_impactDist = GetDistance3D(rayStartPos, currPos);
			result.m_didImpact = true;
			result.m_blockHit = currentBlock;
			return;
		}
	}
}

BlockIterator World::GetBlockIteratorForWorldPos(Vec3 const& worldPos)
{
	//check if start pos is a hit
	IntVec2 chunkCoordsToMine = GetChunkCoordsFromWorldPos(worldPos);
	auto it = m_activeChunks.find(chunkCoordsToMine);
	if (it == m_activeChunks.end())
	{
		return BlockIterator();
	}

	Chunk* chunkToMine = it->second;
	int blockIdx = chunkToMine->GetBlockIndexFromWorldPos(worldPos);
	return BlockIterator(chunkToMine, blockIdx);
}

void World::DirtyBlockLighting(BlockIterator blockToDirty)
{
	if (blockToDirty.GetBlock() != nullptr && !blockToDirty.GetBlock()->IsLightDirty())
	{
		blockToDirty.GetBlock()->SetIsLightDirty(true);
		m_dirtyLightBlocks.push_back(blockToDirty);
	}
}

void World::UndirtyAllBlocksInChunk(Chunk* chunk)
{
	for (auto it = m_dirtyLightBlocks.begin(); it != m_dirtyLightBlocks.end(); it++)
	{
		if (it->m_chunk == chunk)
		{
			it->m_chunk = nullptr;
		}
	}
}

void World::ActivateNearestChunk()
{
	float chunkActivationRange = g_gameConfigBlackboard.GetValue("chunkActivationRange", 250.f);
	int maxChunksRadiusX = 1 + ((int)chunkActivationRange) / XSIZE;
	int maxChunksRadiusY = 1 + ((int)chunkActivationRange) / YSIZE;

	IntVec2 playerChunkCoords = GetChunkCoordsFromWorldPos(g_theGame->m_player->m_position);
	IntVec2 neighborhoodMin = playerChunkCoords - IntVec2(maxChunksRadiusX, maxChunksRadiusY);
	IntVec2 neighborhoodMax = playerChunkCoords + IntVec2(maxChunksRadiusX, maxChunksRadiusY);

	IntVec2 bestCandidate;
	float bestDistSquared = 99999999999.f;
	bool candidateFound = false;
	Vec2 playerPosXY = g_theGame->m_player->m_position.GetXY();
	for (int y = neighborhoodMin.y; y <= neighborhoodMax.y; y++)
	{
		for (int x = neighborhoodMin.x; x <= neighborhoodMax.x; x++)
		{
			IntVec2 currChunkCoords = IntVec2(x, y);
			Vec2 currCenter = GetChunkCoordsCenter(currChunkCoords);
			float currDistSqured = GetDistanceSquared2D(playerPosXY, currCenter);

			//chunk is closest so far and does not already exist
			if (currDistSqured < bestDistSquared && currDistSqured < chunkActivationRange * chunkActivationRange && m_activeChunks.find(currChunkCoords) == m_activeChunks.end())
			{
				//make sure closest so far is not already queued for activation
				if (m_chunksQueuedForGeneration.find(currChunkCoords) == m_chunksQueuedForGeneration.end())
				{
					bestDistSquared = currDistSqured;
					bestCandidate = currChunkCoords;
					candidateFound = true;
				}
			}
		}
	}

	if (candidateFound)
	{
		QueueChunkForActivation(bestCandidate);
	}
}

void World::HandleReadyChunkJobs()
{
	std::vector<Job*> completedJobs;
	g_theJobSystem->GetCompletedJobs(completedJobs);
	for (int i = 0; i < completedJobs.size(); i++)
	{
		ChunkJob* chunkJob = dynamic_cast<ChunkJob*>(completedJobs[i]);
		if (chunkJob->m_jobTypeBit != JOB_WRITE_CHUNK_TO_DISK)
		{
			chunkJob->m_chunk->SetNeighboringChunks();
			chunkJob->m_chunk->InitializeLightingData();
			m_chunksQueuedForGeneration.erase(chunkJob->m_chunk->m_chunkCoords);
			m_activeChunks[chunkJob->m_chunk->m_chunkCoords] = chunkJob->m_chunk;
			chunkJob->m_chunk->m_chunkState = ChunkState::ACTIVE;
		}
		else
		{
			delete chunkJob->m_chunk;
		}
		delete chunkJob;
		chunkJob = nullptr;
	}
}

void World::DeactivateFurthestChunk()
{
	float chunkActivationRange = g_gameConfigBlackboard.GetValue("chunkActivationRange", 250.f);

	float deactivationRange = chunkActivationRange + (float)XSIZE + (float)YSIZE;
	float deactivationRangeSquared = deactivationRange * deactivationRange;
	Vec2 playerPosXY = g_theGame->m_player->m_position.GetXY();
	float furthestChunkDistanceSquared = 0.f;
	IntVec2 furthestChunk;
	bool chunkOutsideRange = false;
	for (auto it = m_activeChunks.begin(); it != m_activeChunks.end(); it++)
	{
		float currChunkDistanceSqured = GetDistanceSquared2D(playerPosXY, GetChunkCoordsCenter(it->first));
		if (currChunkDistanceSqured > furthestChunkDistanceSquared && currChunkDistanceSqured > deactivationRangeSquared)
		{
			furthestChunkDistanceSquared = currChunkDistanceSqured;
			furthestChunk = it->first;
			chunkOutsideRange = true;
		}
	}

	if (chunkOutsideRange)
	{
		QueueChunkForDeactivation(furthestChunk);
	}
}

void World::MineBlock()
{
	BlockIterator blockToMineIter = m_selectedBlock.m_blockHit;
	Block* blockToMine = m_selectedBlock.m_blockHit.GetBlock();
	if (blockToMine == nullptr)
	{
		return;
	}
	//turn block into air
	blockToMine->m_blockType = 0;
	DirtyBlockLighting(blockToMineIter);

	//Update new blocks that should now be sky and dirty their lighting
	BlockIterator topBlockIter = blockToMineIter.GetTopNeighbor();
	if (topBlockIter.GetBlock()->IsBlockSky())
	{
		BlockIterator currBlockIter = topBlockIter;
		while (currBlockIter.GetBlock() && !currBlockIter.GetBlock()->IsBlockOpaque())
		{
			currBlockIter.GetBlock()->SetIsBlockSky(true);
			DirtyBlockLighting(currBlockIter);
			currBlockIter = currBlockIter.GetBottomNeighbor();
		}
	}
	m_selectedBlock.m_blockHit.m_chunk->m_isMeshDirty = true;
	blockToMineIter.GetEastNeighbor().m_chunk->m_isMeshDirty = true;
	blockToMineIter.GetWestNeighbor().m_chunk->m_isMeshDirty = true;
	blockToMineIter.GetNorthNeighbor().m_chunk->m_isMeshDirty = true;
	blockToMineIter.GetSouthNeighbor().m_chunk->m_isMeshDirty = true;
	m_selectedBlock.m_blockHit.m_chunk->m_needsSaving = true;
}

void World::PlaceBlock(BlockType blockType)
{
	BlockIterator selectedBlock = m_selectedBlock.m_blockHit;
	if (selectedBlock.GetBlock() == nullptr)
	{
		return;
	}

	//EAST
	if (m_selectedBlock.m_impactNormal.x > 0)
	{
		HandleBlockPlacment(selectedBlock.GetEastNeighbor(), blockType);
	}

	//WEST
	if (m_selectedBlock.m_impactNormal.x < 0)
	{
		HandleBlockPlacment(selectedBlock.GetWestNeighbor(), blockType);

	}

	//NORTH
	if (m_selectedBlock.m_impactNormal.y > 0)
	{
		HandleBlockPlacment(selectedBlock.GetNorthNeighbor(), blockType);

	}

	//SOUTH
	if (m_selectedBlock.m_impactNormal.y < 0)
	{
		HandleBlockPlacment(selectedBlock.GetSouthNeighbor(), blockType);

	}

	//TOP
	if (m_selectedBlock.m_impactNormal.z > 0)
	{
		HandleBlockPlacment(selectedBlock.GetTopNeighbor(), blockType);

	}

	//BOTTOM
	if (m_selectedBlock.m_impactNormal.z < 0)
	{
		HandleBlockPlacment(selectedBlock.GetBottomNeighbor(), blockType);
	}
}

void World::ProcessDirtyLighting()
{
	while (m_dirtyLightBlocks.size() > 0)
	{
		BlockIterator currBlock = m_dirtyLightBlocks.front();

		m_dirtyLightBlocks.pop_front();
		ProcessNextDirtyLightBlock(currBlock);
	}
}

void World::ProcessNextDirtyLightBlock(BlockIterator blockIter)
{
	if (!blockIter.IsValid())
	{
		return;
	}

	blockIter.GetBlock()->SetIsLightDirty(false);
	Block* blockPtr = blockIter.GetBlock();
	int correctIndoorInfluence = 0;
	int correctOutdoorInfluence = 0;

	if (blockPtr->IsBlockSky())
	{
		correctOutdoorInfluence = 15;
	}
	if (correctIndoorInfluence < blockPtr->GetBlockDef()->m_indoorLightEmitted)
	{
		correctIndoorInfluence = blockPtr->GetBlockDef()->m_indoorLightEmitted;
	}

	//non opaque blocks must have lighting at least 1 less than nearest neighbor
	if (!blockPtr->GetBlockDef()->m_isOpaque)
	{
		for (int dir = 0; dir < (int)Direction::COUNT; dir++)
		{
			Block* neighborBlockPtr = blockIter.GetNeighbor((Direction)dir).GetBlock();
			if (neighborBlockPtr == nullptr)
			{
				continue;
			}
			if (correctIndoorInfluence < neighborBlockPtr->GetIndoorLightInfluence() - 1)
			{
				correctIndoorInfluence = neighborBlockPtr->GetIndoorLightInfluence() - 1;
			}
		}

		for (int dir = 0; dir < (int)Direction::COUNT; dir++)
		{
			Block* neighborBlockPtr = blockIter.GetNeighbor((Direction)dir).GetBlock();
			if (neighborBlockPtr == nullptr)
			{
				continue;
			}
			if (correctOutdoorInfluence < neighborBlockPtr->GetOutdoorLightInfluence() - 1)
			{
				correctOutdoorInfluence = neighborBlockPtr->GetOutdoorLightInfluence() - 1;
			}
		}
	}

	bool wasLightWrong = false;
	if (correctIndoorInfluence != blockPtr->GetIndoorLightInfluence())
	{
		//blockPtr->m_blockType = 10;
		blockPtr->SetIndoorLightInfluence((unsigned char)correctIndoorInfluence);
		wasLightWrong = true;
	}
	if (correctOutdoorInfluence != blockPtr->GetOutdoorLightInfluence())
	{
		blockPtr->SetOutdoorLightInfluence((unsigned char)correctOutdoorInfluence);
		wasLightWrong = true;
	}

	if (wasLightWrong)
	{
		blockIter.m_chunk->m_isMeshDirty = true;
		for (int dir = 0; dir < (int)Direction::COUNT; dir++)
		{
			BlockIterator currIter = blockIter.GetNeighbor((Direction)dir);
			if (currIter.m_chunk != nullptr)
			{
				currIter.m_chunk->m_isMeshDirty = true;
				if (!currIter.GetBlock()->GetBlockDef()->m_isOpaque)
				{
					DirtyBlockLighting(currIter);
				}
			}

		}

	}

}

void World::HandleBlockPlacment(BlockIterator blockToPlaceAt, BlockType newBlockType)
{
	Block* blockPtr = blockToPlaceAt.GetBlock();
	if (blockPtr != nullptr)
	{
		blockPtr->m_blockType = newBlockType;
		DirtyBlockLighting(blockToPlaceAt);
		if (blockPtr->GetBlockDef()->m_isOpaque && blockPtr->IsBlockSky())
		{
			blockPtr->SetIsBlockSky(false);
			BlockIterator currentBlock = blockToPlaceAt.GetBottomNeighbor();
			while (currentBlock.IsValid() && !currentBlock.GetBlock()->IsBlockOpaque() && currentBlock.GetBlock()->IsBlockSky())
			{
				currentBlock.GetBlock()->SetIsBlockSky(false);
				currentBlock = currentBlock.GetBottomNeighbor();
			}
		}
	}
	if (blockToPlaceAt.m_chunk != nullptr)
	{
		blockToPlaceAt.m_chunk->m_isMeshDirty = true;
		blockToPlaceAt.m_chunk->m_needsSaving = true;
	}
}

void World::DrawBlockSelection()
{
	Vec3 blockMins = Vec3((float)RoundDownToInt(m_selectedBlock.m_impactPos.x), (float)RoundDownToInt(m_selectedBlock.m_impactPos.y), (float)RoundDownToInt(m_selectedBlock.m_impactPos.z) );
	std::vector<Vertex_PCU> selectedBlockVerts;
	Vec3 blockFaceI = m_selectedBlock.m_impactNormal;
	Vec3 blockFaceJ;
	Vec3 blockFaceK;

	if (fabsf(blockFaceI.z) != 1.f)
	{
		blockFaceJ = CrossProduct3D(blockFaceI, Vec3(0.f, 0.f, 1.f));
		blockFaceJ = blockFaceJ.GetNormalized();
	}
	else
	{
		blockFaceJ = CrossProduct3D(blockFaceI, Vec3(1.f, 0.f, 0.f));
	}
	blockFaceK = CrossProduct3D(blockFaceI, blockFaceJ);
	Vec3 blockFaceCenter = m_selectedBlock.m_blockHit.GetWorldCenter() + blockFaceI * .51f;

	Vec3 blockbottomRight = blockFaceCenter - (.5f * blockFaceJ) - (.5f * blockFaceK);
	Vec3 blockbottomLeft = blockFaceCenter + (.5f * blockFaceJ) - (.5f * blockFaceK);
	Vec3 blockTopLeft = blockFaceCenter + (.5f * blockFaceJ) + (.5f * blockFaceK);
	Vec3 blockTopRight = blockFaceCenter - (.5f * blockFaceJ) + (.5f * blockFaceK);

	AddVertsForLine3D(selectedBlockVerts, blockbottomRight, blockbottomLeft, .01f, Rgba8::MAGENTA, Rgba8::MAGENTA);
	AddVertsForLine3D(selectedBlockVerts, blockTopRight, blockTopLeft, .01f, Rgba8::MAGENTA, Rgba8::MAGENTA);
	AddVertsForLine3D(selectedBlockVerts, blockbottomRight, blockTopRight, .01f, Rgba8::MAGENTA, Rgba8::MAGENTA);
	AddVertsForLine3D(selectedBlockVerts, blockbottomLeft, blockTopLeft, .01f, Rgba8::MAGENTA, Rgba8::MAGENTA);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexArray(selectedBlockVerts.size(), selectedBlockVerts.data());
}

void World::UpdateSimpleMinerConstants(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed('Y'))
	{
		m_timeScale *= 50.f;
	}

	if (g_theInput->WasKeyJustReleased('Y'))
	{
		m_timeScale /= 50.f;
	}

	m_worldDay += deltaSeconds * m_timeScale / (60.f * 60.f * 24.f);

	Rgba8 nightSkyColor(10, 15, 20);
	Rgba8 middaySkyColor(75, 200, 255);
	Rgba8 middayOutdoorColor = Rgba8::WHITE;
	Rgba8 nightOutdoorColor = nightSkyColor;
	Rgba8 currentOutdoorColor = nightOutdoorColor;
	SkyColor = nightSkyColor;
	float currentDayFraction = m_worldDay - (float)RoundDownToInt(m_worldDay);
	if (g_gameConfigBlackboard.GetValue("disableDayNight", false))
	{
		currentDayFraction = .5f;
	}
	if (currentDayFraction > .25f && currentDayFraction <= .5f)
	{
		SkyColor = LerpColor(nightSkyColor, middaySkyColor, (currentDayFraction - .25f) * 4.f);
		currentOutdoorColor = LerpColor(nightOutdoorColor, middayOutdoorColor, (currentDayFraction - .25f) * 4.f);
	}

	if (currentDayFraction > .5f && currentDayFraction < .75f)
	{
		SkyColor = LerpColor(middaySkyColor, nightSkyColor, (currentDayFraction - .5f) * 4.f);
		currentOutdoorColor = LerpColor(middayOutdoorColor, nightOutdoorColor, (currentDayFraction - .5f) * 4.f);
	}

	float lightningPerlin = Compute1dPerlinNoise(currentDayFraction, 10.f, 9, .5f, 50.f, true, m_worldSeed) * 1.5f;
	float lightningStrength = RangeMapClamped(lightningPerlin, .6f, .9f, 0.f, 1.f);
	lightningStrength = 0.f;
	float glowPerlin = Compute1dPerlinNoise(currentDayFraction, 1.f, 5, .5f, 20.f, true, m_worldSeed);
	float glowStrength = RangeMapClamped(glowPerlin, -1.f, 1.f, .8f, 1.f);

	currentOutdoorColor = LerpColor(currentOutdoorColor, Rgba8::WHITE, lightningStrength);
	SkyColor = LerpColor(SkyColor, Rgba8::WHITE, lightningStrength);

	SimpleMinerConstants simpleConstants;
	simpleConstants.CameraWorldPos = g_theGame->m_player->m_position;
	SkyColor.GetAsFloats(simpleConstants.SkyColor);
	currentOutdoorColor.GetAsFloats(simpleConstants.OutdoorLightColor);

	Rgba8(255, 230, 204).GetAsFloats(simpleConstants.IndoorLightColor);
	simpleConstants.IndoorLightColor[0] *= glowStrength;
	simpleConstants.IndoorLightColor[1] *= glowStrength;
	simpleConstants.IndoorLightColor[2] *= glowStrength;

	simpleConstants.FogFarDistance = g_gameConfigBlackboard.GetValue("chunkActivationRange", 250.f) - 16.f;
	simpleConstants.FogNearDistance = simpleConstants.FogFarDistance * .5f;

	g_theRenderer->CopyCPUToGPU((void*)&simpleConstants, m_simpleMinerCBO);
	g_theRenderer->BindConstantBuffer(k_simpleMinerConstantsSlot, m_simpleMinerCBO);
}

void World::InitializeBlockTemplates()
{
	//OAK TREE
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(14, IntVec3(0, 0, 0)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(14, IntVec3(0, 0, 1)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(14, IntVec3(0, 0, 2)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(14, IntVec3(0, 0, 3)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(14, IntVec3(0, 0, 4)));

	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(1, 0, 3)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(-1, 0, 3)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(0, 1, 3)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(0, -1, 3)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(1, 1, 3)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(-1, 1, 3)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(-1, -1, 3)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(1, -1, 3)));

	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(1, 0, 4)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(-1, 0, 4)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(0, 1, 4)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(0, -1, 4)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(1, 1, 4)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(-1, 1, 4)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(-1, -1, 4)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(1, -1, 4)));

	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(2, 0, 3)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(-2, 0, 3)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(0, 2, 3)));
	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(0, -2, 3)));

	m_oakTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(15, IntVec3(0, 0, 5)));

	//CACTUS
	m_cactusTemplate.m_templateEntries.push_back(BlockTemplateEntry(13, IntVec3(0, 0, 0)));
	m_cactusTemplate.m_templateEntries.push_back(BlockTemplateEntry(13, IntVec3(0, 0, 1)));
	m_cactusTemplate.m_templateEntries.push_back(BlockTemplateEntry(13, IntVec3(0, 0, 2)));

	//SPRUCE
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(16, IntVec3(0, 0, 0)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(16, IntVec3(0, 0, 1)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(16, IntVec3(0, 0, 2)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(16, IntVec3(0, 0, 3)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(16, IntVec3(0, 0, 4)));

	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(1, 0, 2)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(-1, 0, 2)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(0, 1, 2)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(0, -1, 2)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(1, 1, 2)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(-1, 1, 2)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(-1, -1, 2)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(1, -1, 2)));
																	  
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(1, 0, 3)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(-1, 0, 3)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(0, 1, 3)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(0, -1, 3)));
															  
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(0, 0, 4)));
	m_spruceTreeTemplate.m_templateEntries.push_back(BlockTemplateEntry(17, IntVec3(0, 0, 5)));
}
