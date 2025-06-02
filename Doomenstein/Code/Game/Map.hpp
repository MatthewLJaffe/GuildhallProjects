#pragma once
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Tile.hpp"
#include "Engine/Core/HeatMaps.hpp"
#include "Game/ActorUID.hpp"

class VertexBuffer;
class Actor;
class PlayerController;

constexpr float WALL_HEAT_VALUE = 1.f;
constexpr float OPEN_HEAT_VALUE = 0.f;

struct RaycastResultDoomenstein
{
	// Basic raycast result information (required)
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	Vec3	m_impactPos;
	Vec3	m_impactNormal;

	// Original raycast information (optional)
	Vec3	m_rayFwdNormal;
	Vec3	m_rayStartPos;
	float	m_rayMaxLength = 1.f;

	ActorUID m_actorHit = ActorUID::INVALID;
};


class Map
{
public:
	Actor* SpawnActor(const SpawnInfo& spawnInfo);
	void SpawnActor(Actor* actorToSpawn);
	void SpawnPlayer(Actor* playerActor);
	void SpawnPlayer(int playerIdx);
	void RespawnBinkey(Actor* binkeyActor);
	Actor* GetActorByUID(const ActorUID uid) const;
	Actor* GetFirstActorWithName(std::string name) const;

	Map(MapDefinition const& definition);
	~Map();
	MapDefinition m_def;
	IntVec2 m_dimensions;
	std::vector<Tile> m_tiles;
	std::vector<Vertex_PCUTBN> m_vertexes;
	std::vector<unsigned int> m_indexes;
	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	Vec3 m_sunDirection = Vec3(2.f, 1.f, -1.f);
	float m_sunIntensity = .7f;
	float m_ambientIntensity = .4f;
	PointLight m_pointLights[MAX_NUM_POINT_LIGHTS];
	int m_currNumPointLights = 0;

public:
	void InitializeMap();
	void Update(float deltaSeconds);
	void Render();
	RaycastResultDoomenstein RaycastVsMap(Vec3 const& startPos, Vec3 const& rayFwdNormal, float rayDistance, ActorUID actorToIgnore = ActorUID::INVALID);
	void RaycastMapWalls(RaycastResultDoomenstein& result, Vec3 const& rayStart, Vec3 const& rayFwdNormal, float rayDistance);
	void RaycastMapFloorAndCeiling(RaycastResultDoomenstein& result, Vec3 const& rayStart, Vec3 const& rayFwdNormal, float rayDistance);
	void RaycastWorldActors(RaycastResultDoomenstein& result, Vec3 const& rayStart, Vec3 const& rayFwdNormal, float rayDistance, ActorUID actorToIgnore = ActorUID::INVALID);
	IntVec2 GetCoordsFromPos(Vec3 const& pos);
	const Tile& GetTileFromCoords(IntVec2 const& coords);
	bool IsInBounds(IntVec2 const& tileCoords);
	void DebugPossessNext();
	ActorUID FindTargetForActor(Actor* searchingActor);
	void GetActorsInMeleeSwing(std::vector<Actor*>& actorsInSwing, Vec3 const& swingPos, Vec2 swingDirXY, float swingArc, float swingRange);
	void AddPointLight(PointLight pointLightToAdd);
	void RemovePointLight(int idxToRemoveAt);
	void UpdatePointLight(int idxToUpdate, PointLight updatedValues);
	void SpawnEnemy();
	int GetNumPointLights();
	int GetNumEnemies();
	void DestroyAllActorsWithName(std::string name);
	void DestroyLightTiles();
	Tile GetTileFromPosition(Vec3 position);
	void RemoveActor(ActorUID actorToRemove);

private:
	void AdjustLightingCommands();
	void AddActors();
	void PushActorsOutOfEachother();
	void PushActorsOutOfWorld();
	void PushActorOutOfTile(Actor* actorToPush, Tile const& tile, bool& didCollide);
	void DeleteGarbageActors();
	bool IsActorAlive(Actor* actor);
	bool IsNonGarbage(Actor* actor);
	void HandlePlayerRespawn();
	void ConstrainActorToMapDimensions(Actor* actor);
private:
	std::vector<Actor*> m_allActors;
	unsigned int m_actorSalt = 0;
};