#pragma once
#include <queue>

#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/HeatMaps.hpp"
#include "Game/MapDefinition.hpp"

class Explosion;
class Entity;
class Player;
class Image;

constexpr float SPECIAL_TILE_HEAT = 9999.f;

enum HeatMapType
{
	NO_HEAT_MAP,
	HEAT_MAP_DISTANCE_FROM_START,
	HEAT_MAP_SOLID_AMPHIBIANS,
	HEAT_MAP_SOLID_LAND_BASED,
	HEAT_MAP_DISTANCE_TO_ENTITY_GOAL
};

class Map
{
public:
	Map(MapDefinition const& config);
	std::vector<Tile> m_tiles;
	void StartUp();
	void UpdateMapVerts();
	void UpdateMapEntities(float deltaSeconds);
	void RenderMapTiles() const;
	void RenderMapEntities() const;
	void RenderDebugMapEntities() const;
	void RenderMapHealthBars() const;
	void RenderDebugHeatmap(TileHeatMap const& heatMap, Rgba8 const& minColor, Rgba8 const& maxColor, Rgba8 const& specialColor) const;
	void DrawCurrentHeatMap() const;
	void DeleteMapEntities();
	void PushEntitiesOutOfWalls();
	void PushEntitiesOutOfEachother();
	void DeleteGarbageEntities();
	void SpawnEnemyInMap(EntityType entityType);
	Vec2 GetRandomReachableTargetPos(IntVec2 const& currCoords, TileHeatMap* Out_TargetDistanceField, bool treatWaterAsInterior);
	Tile& GetTileFromCoords(IntVec2 const& tileCoords);
	Tile& GetTileFromCoords(int xPos, int yPos);
	Tile& GetTileFromPos(Vec2 const& pos);
	void ConstrainWorldCameraToMapBounds();
	Entity* SpawnNewEntity(EntityType type, Vec2 const& position, float orientationDegrees);
	Explosion* SpawnNewExplosion(Vec2 const& position, float scale, float duration, bool isMuzzleFlash = false);
	void AddEntityToMap(Entity* e);
	void RemoveEntityFromMap(Entity* e);
	IntVec2 GetCoordsFromPos(Vec2 const& pos);
	bool IsPointInSolid(Vec2 const& point, bool treatWaterAsSolid);
	bool HasLineOfSight(Entity* entityLooking, float visionRadius, Entity* entityToFind);
	bool IsInStartBunker(IntVec2 const& coords);
	bool IsInEndBunker(IntVec2 const& coords);
	bool IsInteriorPos(IntVec2 const& coords);
	void RespawnPlayer();
	void RemoveEntityFromList( Entity* e, std::vector<Entity*>& entityList);
	void Update(float deltaSeconds);
	Player* GetNearestPlayerAlive();
	Player* GetFirstPlayer();
	void ShutDown();
	void CheckForBulletCollision();
	void CheckForBulletCollisionBetweenFactions(EntityType bulletType, EntityFaction targetFaction);
	void CheckForMissileCollisionBetweenFactions(EntityType bulletType, EntityFaction targetFaction);
	bool IsAlive(Entity* e) const;
	void GenerateEntityPathToGoal(TileHeatMap* entityHeatMap, std::vector<Vec2>& out_PathToGoal, IntVec2 const& goalCoords, IntVec2 const& entityCoords);
	void PopulateDistanceField(TileHeatMap& out_DistanceField, IntVec2 startCoords, float maxCost, bool treatWaterAsSolid=true);
	void PopulateSolidMap(TileHeatMap& out_distanceField, bool treatWaterAsSolid, bool treatScorpioAsSolid = true);
	void RepopulateSolidMaps();
	void CycleDebugHeatMap();
	bool IsScorpioInTile(IntVec2 const& coords);
	void PlayDiscoverySound();
	Image* m_mapImage;
	HeatMapType m_heatMapToDraw = NO_HEAT_MAP;
	TileHeatMap m_solidMapForLandbased;
	TileHeatMap m_opaqueMap;
	TileHeatMap m_solidMapForAmphibians;
	TileHeatMap m_distanceFieldFromStart;
	IntVec2 m_dimensions;
	MapDefinition m_config;
	Vec2 m_exitPosition;
	EntityList m_entityListsByType[NUM_ENTITY_TYPES];
	EntityList m_entityListsByFaction[NUM_FACTION_TYPES];
	EntityList m_evilTankList;
private:
	void SetHealthOfTiles();
	bool IsEntityEvilTank(EntityType type);
	bool m_isMapValid = false;
	void GenerateMap();
	void FillUnreachableTiles();
	void AddWormTiles(std::string wormTileType, int numWorms, int wormLength);
	void CheckForTransitionToNewMap();
	void PushEntityOutOfTile(Entity* entityToPush, Tile const& tileToPushOutOf);
	void AddVertsForTile(std::vector<Vertex_PCU>& verts, Tile const& tile) const;
	void EvaluateTileInDistanceField(IntVec2 const& tileCoords, float prevTileHeat, TileHeatMap& out_distanceField, std::queue<IntVec2>& out_tilesToExplore, bool treatWaterAsSolid);
	void DrawMapImage();
	double m_lastDiscoverySoundTime = 0.f;
	std::vector<Vertex_PCU> m_tileVerts;
	std::vector<Entity*> m_allEntities;
	float m_maxDistanceFieldHeat = 0.f;
};