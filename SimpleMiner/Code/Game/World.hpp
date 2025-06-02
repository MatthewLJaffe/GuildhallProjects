#pragma once
#include "Game/Chunk.hpp"
#include "Engine/Core/Timer.hpp"
#include "Game/BlockIterator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/BlockTemplate.hpp"
#include <deque>
#include <set>
#include <filesystem>

constexpr uint32_t JOB_GENERATE_CHUNK =			1 << 0;
constexpr uint32_t JOB_READ_CHUNK_FROM_DISK =	1 << 1;
constexpr uint32_t JOB_WRITE_CHUNK_TO_DISK =	1 << 2;

struct RaycastResultSimpleMiner
{
	// Basic raycast result information (required)
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	Vec3	m_impactPos;
	Vec3	m_impactNormal;
	BlockIterator m_blockHit;

	// Original raycast information (optional)
	Vec3	m_rayFwdNormal;
	Vec3	m_rayStartPos;
	float	m_rayMaxLength = 1.f;
};

struct SimpleMinerConstants
{
	Vec3 CameraWorldPos;
	float padding;
	float IndoorLightColor[4];
	float OutdoorLightColor[4];
	float SkyColor[4];
	float FogFarDistance;
	float FogNearDistance;
	Vec2 padding2;
};


class World
{
public:
	World();
	void Update(float deltaSeconds);
	void StartUp();
	void Render();
	void ShutDown();
	void RebuildWorld();
	void QueueChunkForActivation(IntVec2 chunkCoords);
	void QueueChunkForDeactivation(IntVec2 chunkCoords);
	IntVec2 GetChunkCoordsFromWorldPos(Vec3 worldPos);
	Vec2 GetChunkCoordsCenter(IntVec2 chunkCoords);
	Chunk* GetChunkAtCoords(IntVec2 chunkCoords);
	Chunk* GetClosestChunkToRebuildMesh();
	bool m_chunkMeshRebuiltThisFrame = false;
	void RaycastVsWorld(RaycastResultSimpleMiner& result, Vec3 const& rayStartPos, Vec3 const& rayFwdNormal, float maxRaylength);
	BlockIterator GetBlockIteratorForWorldPos(Vec3 const& worldPos);
	void DirtyBlockLighting(BlockIterator blockToDirty);
	void UndirtyAllBlocksInChunk(Chunk* chunk);
	unsigned int m_worldSeed;
	BlockTemplate m_oakTreeTemplate;
	BlockTemplate m_cactusTemplate;
	BlockTemplate m_spruceTreeTemplate;
	int m_maxMeshesToRebuildPerFrame = 2;
	int m_meshesRebuildThisFrame = 0;

private:
	void HandleBlockPlacment(BlockIterator blockToPlaceAt, BlockType newBlockType);
	void ActivateNearestChunk();
	void HandleReadyChunkJobs();
	void DeactivateFurthestChunk();
	void MineBlock();
	void PlaceBlock(BlockType blockType);
	//processes and propagates all dirty light blocks until none remain
	void ProcessDirtyLighting();
	void ProcessNextDirtyLightBlock(BlockIterator dirtyBlock);
	void DrawBlockSelection();
	void UpdateSimpleMinerConstants(float deltaSconds);
	void InitializeBlockTemplates();
	std::deque<BlockIterator> m_dirtyLightBlocks;
	std::map< IntVec2, Chunk* > m_activeChunks;
	std::set<IntVec2> m_chunksQueuedForGeneration;
	BlockType m_blockTypeToPlace = 9;
	float m_debugMessageTime = .1f;
	Timer m_debugTimer;
	std::string m_debugMessage;
	RaycastResultSimpleMiner m_selectedBlock;
	Vec3 m_raycastStarPos;
	Vec3 m_raycastDir;
	bool m_isRaycastLocked = false;
	ConstantBuffer* m_simpleMinerCBO = nullptr;
	float m_worldDay = 0.f;
	float m_timeScale = 200.f;
};
