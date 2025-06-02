#include "Game/Map.hpp"
#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Game/Scorpio.hpp"
#include "Game/Leo.hpp"
#include "Game/Aries.hpp"
#include "Game/Bullet.hpp"
#include "Game/Player.hpp"
#include "Game/Capricorn.hpp"
#include "Game/GuidedMissile.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Explosion.hpp"


Map::Map(MapDefinition const& config)
	: m_config(config)
	, m_distanceFieldFromStart(TileHeatMap(config.m_dimensions))
	, m_solidMapForLandbased(TileHeatMap(config.m_dimensions))
	, m_solidMapForAmphibians(TileHeatMap(config.m_dimensions))
	, m_opaqueMap(TileHeatMap(config.m_dimensions))
{
	m_dimensions = m_config.m_dimensions;
	int mapSize = m_dimensions.x * m_dimensions.y;
	m_tiles.resize(static_cast<size_t>(mapSize));
	for (int x = 0; x < m_dimensions.x; x++)
	{
		for (int y = 0; y < m_dimensions.y; y++)
		{
			int tileIdx = x + y * m_dimensions.x;
			m_tiles[tileIdx].m_tileCoords = IntVec2(x, y);
			m_tiles[tileIdx].SetTileDefinition(m_config.m_fillTileType);
		}
	}
}

void Map::StartUp()
{
	while (!m_isMapValid)
	{
		GenerateMap();
	}
	SetHealthOfTiles();
	if (m_config.m_mapImageName != "")
	{
		m_mapImage = new Image(m_config.m_mapImageName.c_str());
		DrawMapImage();
		PopulateDistanceField(m_distanceFieldFromStart, IntVec2(1, 1), SPECIAL_TILE_HEAT, true);
	}
	//adding verts
	int vertsToReserve = m_dimensions.x * m_dimensions.y * 6;
	m_tileVerts.reserve(vertsToReserve);
	UpdateMapVerts();

	//spawn enemies
	for (int i = 0; i < m_config.m_ariesCount; i++)
	{
		SpawnEnemyInMap(ENTITY_TYPE_EVIL_ARIES);
	}
	for (int i = 0; i < m_config.m_leoCount; i++)
	{
		SpawnEnemyInMap(ENTITY_TYPE_EVIL_LEO);
	}
	for (int i = 0; i < m_config.m_scorpioCount; i++)
	{
		SpawnEnemyInMap(ENTITY_TYPE_EVIL_SCORPIO);
	}
	for (int i = 0; i < m_config.m_capricornCount; i++)
	{
		SpawnEnemyInMap(ENTITY_TYPE_EVIL_CAPRICORN);
	}
	RepopulateSolidMaps();
}

void Map::UpdateMapVerts()
{
	m_tileVerts.clear();
	for (size_t i = 0; i < m_tiles.size(); i++)
	{
		AddVertsForTile(m_tileVerts, m_tiles[i]);
	}
}

void Map::GenerateMap()
{
	//add stone
	for (int y = 0; y < m_dimensions.y; y++)
	{
		for (int x = 0; x < m_dimensions.x; x++)
		{
			//start as 
			GetTileFromCoords(IntVec2(x, y)).SetTileDefinition(m_config.m_fillTileType);
			//no stone in 1,1 through 5,5
			if (x >= 1 && x <= m_config.m_startBunkerSize && y >= 1 && y <= m_config.m_startBunkerSize)
			{
				//draw L shape
				if ((x == m_config.m_startBunkerSize - 1 && y > 1 && y < m_config.m_startBunkerSize) || (y == m_config.m_startBunkerSize - 1 && x > 1 && x < m_config.m_startBunkerSize))
				{
					GetTileFromCoords(IntVec2(x, y)).SetTileDefinition(m_config.m_startBunkerTileType);
				}
				else
				{
					GetTileFromCoords(IntVec2(x, y)).SetTileDefinition(m_config.m_startFloorTileType);
				}
				continue;
			}
			if (x >= (m_dimensions.x - 1) - m_config.m_endBunkerSize && x <= m_dimensions.x - 2 && y >= (m_dimensions.y - 1) - m_config.m_endBunkerSize && y <= m_dimensions.y - 2)
			{
				//draw L shape
				if ((x == m_dimensions.x - m_config.m_endBunkerSize && y > (m_dimensions.y - 1) - m_config.m_endBunkerSize && y < m_dimensions.y - 2)
					|| y == (m_dimensions.y) - m_config.m_endBunkerSize && x > (m_dimensions.x - 1) - m_config.m_endBunkerSize && x < m_dimensions.x - 2)
				{
					GetTileFromCoords(IntVec2(x, y)).SetTileDefinition(m_config.m_endBunkerTileType);
				}
				else
				{
					GetTileFromCoords(IntVec2(x, y)).SetTileDefinition(m_config.m_endFloorTileType);
				}
				continue;
			}

			if (x == m_dimensions.x - 1 ||
				y == m_dimensions.y - 1 ||
				x == 0 || y == 0)
			{
				GetTileFromCoords(IntVec2(x, y)).SetTileDefinition(m_config.m_edgeTileType);
			}
			//entrance and exit tiles
			GetTileFromCoords(IntVec2(1, 1)).SetTileDefinition(m_config.m_entranceTileType);
			GetTileFromCoords(IntVec2(m_dimensions.x - 2, m_dimensions.y - 2)).SetTileDefinition(m_config.m_exitTileType);
			m_exitPosition = Vec2((m_dimensions.x - 2) + .5f, (m_dimensions.y - 2) + .5f);
		}
	}
	AddWormTiles(m_config.m_worm1TileType, m_config.m_worm1Count, m_config.m_worm1MaxLength);
	AddWormTiles(m_config.m_worm2TileType, m_config.m_worm2Count, m_config.m_worm2MaxLength);
	AddWormTiles(m_config.m_worm3TileType, m_config.m_worm3Count, m_config.m_worm3MaxLength);
	PopulateDistanceField(m_distanceFieldFromStart, IntVec2(1, 1), SPECIAL_TILE_HEAT, true);
	FillUnreachableTiles();
	if (m_distanceFieldFromStart.GetHeatValue( IntVec2( (int)m_exitPosition.x, (int)m_exitPosition.y) ) != SPECIAL_TILE_HEAT )
	{
		m_isMapValid = true;
	}
	else
	{
		DebuggerPrintf("Throwing out map and regenerating\n");
	}
}

void Map::DrawMapImage()
{
	IntVec2 dimensions = m_mapImage->GetDimensions();
	for (int y = 0; y < dimensions.y; y++)
	{
		for (int x = 0; x < dimensions.x; x++)
		{
			Rgba8 texelColor = m_mapImage->GetTexelColor(IntVec2(x, y));
			if (texelColor.a > 0)
			{
				for (size_t i = 0; i < TileDefinition::s_tileDefinitions.size(); i++)
				{
					IntVec2 corrdsToDraw = IntVec2(x, y) + m_config.m_mapImageOffset;
					GUARANTEE_OR_DIE(corrdsToDraw.x >= 0 && corrdsToDraw.x <= m_dimensions.x - 1 && corrdsToDraw.y >= 0 && corrdsToDraw.y <= m_dimensions.y - 1, 
						"Trying to draw tiles outside of map bounds");
					TileDefinition const* definition = TileDefinition::GetTileDefFromColor(texelColor);
					if (g_randGen->RollRandomIntInRange(0, 254) < texelColor.a)
					{
						GetTileFromCoords(x, y).m_tileDef = definition;
					}
				}
			}
		}
	}
}

void Map::AddWormTiles(std::string wormTileName, int numWorms, int wormLength)
{
	for (int currWorm = 0; currWorm < numWorms; currWorm++)
	{
		//get random start pos that isn't in a bunker
		IntVec2 randomStartPos = IntVec2::NORTH_EAST;
		while (IsInEndBunker(randomStartPos) || IsInStartBunker(randomStartPos))
		{
			randomStartPos = IntVec2(g_randGen->RollRandomIntInRange(1, m_dimensions.x - 2), g_randGen->RollRandomIntInRange(1, m_dimensions.y - 2));
		}
		GetTileFromCoords(randomStartPos.x, randomStartPos.y).SetTileDefinition(wormTileName);
		IntVec2 currPos = randomStartPos;
		for (int currLength = 1; currLength < wormLength; currLength++)
		{
			IntVec2 wanderDirection = IntVec2::ZERO;
			int direction = g_randGen->RollRandomIntInRange(0, 3);
			if (direction == 0)
			{
				wanderDirection = IntVec2::EAST;
			}
			else if (direction == 1)
			{
				wanderDirection = IntVec2::SOUTH;
			}
			else if (direction)
			{
				wanderDirection = IntVec2::WEST;
			}
			else
			{
				wanderDirection = IntVec2::NORTH;
			}
			currPos = currPos + wanderDirection;

			//if we are at the edge of the map continue and try again
			if (currPos.x <= 0 || currPos.x >= m_dimensions.x - 1 || currPos.y <= 0 || currPos.y > m_dimensions.y - 1)
			{
				currPos = currPos - wanderDirection;
				continue;
			}
			if (IsInStartBunker(currPos) || IsInEndBunker(currPos))
			{
				currPos = currPos - wanderDirection;
				continue;
			}
			GetTileFromCoords(currPos.x, currPos.y).SetTileDefinition(wormTileName);

		}
	}
}

void Map::FillUnreachableTiles()
{
	for (int y = 0; y < m_dimensions.y; y++)
	{
		for (int x = 0; x < m_dimensions.x; x++)
		{
			Tile& currTile = GetTileFromCoords(x, y);
			float tileHeat = m_distanceFieldFromStart.GetHeatValue(IntVec2(x, y));
			bool isWalkable = !currTile.IsTileSolid() && !currTile.IsTileWater();
			if (tileHeat == SPECIAL_TILE_HEAT && isWalkable)
			{
				currTile.SetTileDefinition(m_config.m_edgeTileType);
			}
		}
	}
}


void Map::UpdateMapEntities(float deltaSeconds)
{
	for (int i = 0; i < static_cast<int>(m_allEntities.size()); i++)
	{
		if (m_allEntities[i]->IsAlive())
		{
			m_allEntities[i]->Update(deltaSeconds);
		}
	}
}

void Map::RenderMapTiles() const
{
	g_theRenderer->BindTexture(SPRITE_SHEET_TILES->GetTexture());
	g_theRenderer->DrawVertexArray(m_tileVerts.size(), m_tileVerts.data());
}

void Map::RenderMapEntities() const
{
	for (int typeIdx = 0; typeIdx < NUM_ENTITY_TYPES; typeIdx++)
	{
		EntityList const& entityList = m_entityListsByType[typeIdx];
		for (int entityIdx = 0; entityIdx < static_cast<int>(entityList.size()); entityIdx++)
		{
			if (entityList[entityIdx]->IsAlive())
			{
				entityList[entityIdx]->Render();
			}
		}
	}
}

void Map::RenderDebugHeatmap(TileHeatMap const& heatMap, Rgba8 const& minColor, Rgba8 const& maxColor, Rgba8 const& specialColor) const
{
	std::vector<Vertex_PCU> heatMapVerts;
	AABB2 tileHeatMapBounds = AABB2(Vec2(0.f, 0.f), Vec2((float)m_dimensions.x, (float)m_dimensions.y));
	heatMap.AddVertsForDebugDraw(heatMapVerts, tileHeatMapBounds, FloatRange(0.f, m_maxDistanceFieldHeat), minColor, maxColor, SPECIAL_TILE_HEAT, specialColor);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(heatMapVerts.size(), heatMapVerts.data());
}

void Map::DrawCurrentHeatMap() const
{
	Vec2 textBottomLeft;
	Camera& screenCam = g_theGame->m_screenCamera;
	textBottomLeft.x = screenCam.GetOrthoBottomLeft().x + screenCam.GetOrthoDimensions().x * .05f;
	textBottomLeft.y = screenCam.GetOrthoTopRight().y - screenCam.GetOrthoDimensions().y * .1f;
	switch (m_heatMapToDraw)
	{
	case NO_HEAT_MAP:
	{
		return;
	}
	case HEAT_MAP_DISTANCE_FROM_START:
	{
		RenderDebugHeatmap(m_distanceFieldFromStart, Rgba8::WHITE, Rgba8::BLACK, Rgba8::BLUE);
		g_bitmapFont->AddVertsForText2D(g_theGame->m_debugTextVerts, textBottomLeft, 1.5f, "Current Heatmap: Distance from start, F6 for next heatmap", Rgba8::GREEN);
		break;
	}
	case HEAT_MAP_SOLID_AMPHIBIANS:
	{
		RenderDebugHeatmap(m_solidMapForAmphibians, Rgba8::BLACK, Rgba8::BLACK, Rgba8::WHITE);
		g_bitmapFont->AddVertsForText2D(g_theGame->m_debugTextVerts, textBottomLeft, 1.5f, "Current Heatmap: Amphibian solid map, F6 for next heatmap", Rgba8::GREEN);

		break;
	}
	case HEAT_MAP_SOLID_LAND_BASED:
	{
		RenderDebugHeatmap(m_solidMapForLandbased, Rgba8::BLACK, Rgba8::BLACK, Rgba8::WHITE);
		g_bitmapFont->AddVertsForText2D(g_theGame->m_debugTextVerts, textBottomLeft, 1.5f, "Current Heatmap: Landbased solid map, F6 for next heatmap", Rgba8::GREEN);
		break;
	}
	case HEAT_MAP_DISTANCE_TO_ENTITY_GOAL:
	{
		g_bitmapFont->AddVertsForText2D(g_theGame->m_debugTextVerts, textBottomLeft, 1.5f, "Current Heatmap: Distance map to enemy goal, F6 for next heatmap", Rgba8::GREEN);
		for (size_t i = 0; i < m_evilTankList.size(); i++)
		{
			Entity const* currEntity = m_evilTankList[i];
			if (currEntity->m_heatMapOfTargetPos != nullptr)
			{
				RenderDebugHeatmap(*currEntity->m_heatMapOfTargetPos, Rgba8::WHITE, Rgba8::BLACK, Rgba8::BLUE);
				std::vector<Vertex_PCU> arrowVerts;
				AddVertsForArrow2D(arrowVerts, Vec2(4, 100), currEntity->m_position + Vec2::UP, .3f, .1f, Rgba8::MAGENTA);
				g_theRenderer->BindTexture(nullptr);
				g_theRenderer->DrawVertexArray(arrowVerts.size(), arrowVerts.data());
				break;
			}
		}
		break;
	}
	default:
		break;
	}
}

void Map::RenderDebugMapEntities() const
{
	for (int i = 0; i < static_cast<int>(m_allEntities.size()); i++)
	{
		m_allEntities[i]->RenderDebug();
	}
}

void Map::RenderMapHealthBars() const
{
	for (size_t i = 0; i < m_allEntities.size(); i++)
	{
		Entity* currEntity = m_allEntities[i];
		if (IsAlive(currEntity))
		{
			currEntity->RenderHealthBar();
		}
	}
}

void Map::DeleteMapEntities()
{
	for (int i = 0; i < static_cast<int>(m_allEntities.size()); i++)
	{
		if (m_allEntities[i] != nullptr)
		{
			delete m_allEntities[i];
		}
	}
}

void Map::PushEntitiesOutOfWalls()
{
	for (size_t i = 0; i < m_allEntities.size(); i++)
	{
		Entity*& entityToPush = m_allEntities[i];
		if (!entityToPush->m_isPushedByWalls)
		{
			continue;
		}
		if (entityToPush->m_entityType == ENTITY_TYPE_GOOD_PLAYER && g_theApp->m_noClip)
		{
			continue;
		}
		IntVec2 entityCoords = GetCoordsFromPos(entityToPush->m_position);

		//north
		PushEntityOutOfTile(entityToPush, GetTileFromCoords(IntVec2(entityCoords.x, entityCoords.y + 1)));
		//south
		PushEntityOutOfTile(entityToPush, GetTileFromCoords(IntVec2(entityCoords.x, entityCoords.y - 1)));
		//east
		PushEntityOutOfTile(entityToPush, GetTileFromCoords(IntVec2(entityCoords.x + 1, entityCoords.y)));
		//west
		PushEntityOutOfTile(entityToPush, GetTileFromCoords(IntVec2(entityCoords.x - 1, entityCoords.y)));

		//northeast
		PushEntityOutOfTile(entityToPush, GetTileFromCoords(IntVec2(entityCoords.x + 1, entityCoords.y + 1)));
		//northwest
		PushEntityOutOfTile(entityToPush, GetTileFromCoords(IntVec2(entityCoords.x - 1, entityCoords.y + 1)));
		//southeast
		PushEntityOutOfTile(entityToPush, GetTileFromCoords(IntVec2(entityCoords.x + 1, entityCoords.y - 1)));
		//southwest
		PushEntityOutOfTile(entityToPush, GetTileFromCoords(IntVec2(entityCoords.x - 1, entityCoords.y - 1)));

	}
}

void Map::PushEntitiesOutOfEachother()
{
	for (size_t i = 0; i < m_allEntities.size() - 1; i++)
	{
		for (size_t j = 1; j < m_allEntities.size(); j++)
		{
			Entity& entityA = *m_allEntities[i];
			Entity& entityB = *m_allEntities[j];

			//neither interact
			if (!entityA.m_isPushedByEntities && !entityB.m_isPushedByEntities)
			{
				continue;
			}

			//push out of eachother
			else if (entityA.m_isPushedByEntities && entityA.m_doesPushEntities && entityB.m_isPushedByEntities && entityB.m_doesPushEntities)
			{
				PushDiscsOutOfEachOther2D(entityA.m_position, entityA.m_physicsRadius, entityB.m_position, entityB.m_physicsRadius);
			}

			//push a out of b
			else if (entityA.m_isPushedByEntities && entityB.m_doesPushEntities)
			{
				PushDiscOutOfFixedDisc2D(entityA.m_position, entityA.m_physicsRadius, entityB.m_position, entityB.m_physicsRadius);
			}

			else if (entityB.m_isPushedByEntities && entityA.m_doesPushEntities)
			{
				PushDiscOutOfFixedDisc2D(entityB.m_position, entityB.m_physicsRadius, entityA.m_position, entityA.m_physicsRadius);
			}
		}
	}
}

void Map::DeleteGarbageEntities()
{
	for (size_t i = 0; i < m_allEntities.size(); i++)
	{
		if (m_allEntities[i]->m_isGarbage)
		{
			Entity* entityToDelete = m_allEntities[i];
			RemoveEntityFromMap(entityToDelete);
			delete entityToDelete;
		}
	}
}

bool Map::IsInteriorPos(IntVec2 const& coords)
{
	return coords.x >= 1 && coords.x <= m_dimensions.x - 2 && coords.y >= 1 && coords.y <= m_dimensions.y - 2;
}

void Map::ShutDown()
{
	for (size_t i = 0; i < m_allEntities.size(); i++)
	{
		if (m_allEntities[i] != nullptr)
		{
			delete m_allEntities[i];
		}
	}
}

bool Map::IsInStartBunker(IntVec2 const& coords)
{
	return (coords.x <= m_config.m_startBunkerSize && coords.y <= m_config.m_startBunkerSize);
}



bool Map::IsInEndBunker(IntVec2 const& coords)
{
	return (coords.x >= m_dimensions.x - m_config.m_endBunkerSize -1 && coords.y >= m_dimensions.y - m_config.m_endBunkerSize -1);

}


bool Map::IsScorpioInTile(IntVec2 const& coords)
{
	for (size_t i = 0; i < m_entityListsByType[ENTITY_TYPE_EVIL_SCORPIO].size(); i++)
	{
		Entity* currScorpio = m_entityListsByType[ENTITY_TYPE_EVIL_SCORPIO][i];
		if (IsAlive(currScorpio))
		{
			IntVec2 scorpioPos = GetCoordsFromPos(currScorpio->m_position);
			if (scorpioPos == coords)
			{
				return true;
			}
		}
	}
	return false;
}

void Map::PlayDiscoverySound()
{
	double currDiscoverySoundTime = GetCurrentTimeSeconds();
	if (currDiscoverySoundTime - m_lastDiscoverySoundTime > .1f)
	{
		g_theAudio->StartSound(SOUND_ID_DISCOVER);
		m_lastDiscoverySoundTime = currDiscoverySoundTime;
	}
}

void Map::CheckForTransitionToNewMap()
{
	for (size_t i = 0; i < m_entityListsByType[ENTITY_TYPE_GOOD_PLAYER].size(); i++)
	{
		Entity* player = m_entityListsByType[ENTITY_TYPE_GOOD_PLAYER][i];
		if (IsPointInsideDisc2D(m_exitPosition, player->m_position, player->m_physicsRadius))
		{
			g_theGame->SwitchMaps();
		}
	}
}

void Map::CheckForBulletCollision()
{
	CheckForBulletCollisionBetweenFactions(EntityType::ENTITY_TYPE_GOOD_BULLET, FACTION_EVIL);
	CheckForBulletCollisionBetweenFactions(EntityType::ENTITY_TYPE_EVIL_BULLET, FACTION_GOOD);
	CheckForBulletCollisionBetweenFactions(EntityType::ENTITY_TYPE_GOOD_FLAME_BULLET, FACTION_EVIL);
	CheckForMissileCollisionBetweenFactions(EntityType::ENTITY_TYPE_EVIL_MISSILE, FACTION_GOOD);
}

void Map::CheckForBulletCollisionBetweenFactions(EntityType bulletType, EntityFaction targetFaction)
{
	for (size_t bulletIdx = 0; bulletIdx < m_entityListsByType[bulletType].size(); bulletIdx++)
	{
		Bullet* bullet = dynamic_cast<Bullet*>(m_entityListsByType[bulletType][bulletIdx]);
		for (size_t targetIdx = 0; targetIdx < m_entityListsByFaction[targetFaction].size(); targetIdx++)
		{
			Entity* targetEntity = m_entityListsByFaction[targetFaction][targetIdx];
			if (!targetEntity->IsAlive() || !targetEntity->m_isHitByBullets)
			{
				continue;
			}
			if (DoDiscsOverlap(targetEntity->m_position, targetEntity->m_physicsRadius, bullet->m_position, bullet->m_physicsRadius))
			{
				targetEntity->HandleIncomingBullet(bullet);
			}
		}
	}
}

void Map::CheckForMissileCollisionBetweenFactions(EntityType missileType, EntityFaction targetFaction)
{
	for (size_t bulletIdx = 0; bulletIdx < m_entityListsByType[missileType].size(); bulletIdx++)
	{
		GuidedMissile* missile = dynamic_cast<GuidedMissile*>(m_entityListsByType[missileType][bulletIdx]);
		for (size_t targetIdx = 0; targetIdx < m_entityListsByFaction[targetFaction].size(); targetIdx++)
		{
			Entity* targetEntity = m_entityListsByFaction[targetFaction][targetIdx];
			if (!targetEntity->IsAlive() || !targetEntity->m_isHitByBullets)
			{
				continue;
			}
			if (DoDiscsOverlap(targetEntity->m_position, targetEntity->m_physicsRadius, missile->m_position, missile->m_physicsRadius))
			{
				targetEntity->HandleIncomingMissile(missile);
			}
		}
	}
}

Player* Map::GetNearestPlayerAlive()
{
	for (size_t i = 0; i < m_entityListsByType[ENTITY_TYPE_GOOD_PLAYER].size(); i++)
	{
		Player* player = dynamic_cast<Player*>(m_entityListsByType[ENTITY_TYPE_GOOD_PLAYER][i]);
		if (player->IsAlive())
		{
			return player;
		}
	}
	return nullptr;
}

Player* Map::GetFirstPlayer()
{
	return dynamic_cast<Player*>(m_entityListsByType[ENTITY_TYPE_GOOD_PLAYER][0]);
}

bool Map::IsAlive(Entity* e) const
{
	return e != nullptr && e->IsAlive();
}

void Map::GenerateEntityPathToGoal(TileHeatMap* entityHeatMap, std::vector<Vec2>& out_PathToGoal, IntVec2 const& goalCoords, IntVec2 const& entityCoords)
{
	IntVec2 currCoords(entityCoords.x, entityCoords.y);
	out_PathToGoal.insert(out_PathToGoal.begin(), Vec2((float)currCoords.x + .5f, (float)currCoords.y + .5f));

	while (currCoords != goalCoords)
	{
		IntVec2 bestCoords = currCoords + IntVec2::NORTH;
		float bestCost = SPECIAL_TILE_HEAT;
		float currCost = bestCost;
		if (IsInteriorPos(bestCoords))
		{
			bestCost = entityHeatMap->GetHeatValue(currCoords + IntVec2::NORTH);
		}

		if (IsInteriorPos(currCoords + IntVec2::EAST))
		{
			currCost = entityHeatMap->GetHeatValue(currCoords + IntVec2::EAST);
			if (currCost < bestCost)
			{
				bestCost = currCost;
				bestCoords = currCoords + IntVec2::EAST;
			}
		}

		if (IsInteriorPos(currCoords + IntVec2::SOUTH))
		{
			currCost = entityHeatMap->GetHeatValue(currCoords + IntVec2::SOUTH);
			if (currCost < bestCost)
			{
				bestCost = currCost;
				bestCoords = currCoords + IntVec2::SOUTH;
			}
		}

		if (IsInteriorPos(currCoords + IntVec2::WEST))
		{
			currCost = entityHeatMap->GetHeatValue(currCoords + IntVec2::WEST);
			if (currCost < bestCost)
			{
				bestCost = currCost;
				bestCoords = currCoords + IntVec2::WEST;
			}
		}

		//no valid moves
		if (bestCost == SPECIAL_TILE_HEAT)
		{
			break;
		}
		currCoords = bestCoords;
		Vec2 nextPos = Vec2((float)currCoords.x + .5f, (float)currCoords.y + .5f);
		//pacing back and forth
		if (nextPos == out_PathToGoal[0])
		{
			break;
		}
		out_PathToGoal.insert(out_PathToGoal.begin(), nextPos);
	}
}

void Map::PushEntityOutOfTile(Entity* entityToPush, Tile const& tileToPushOutOf)
{
	if ( tileToPushOutOf.IsTileSolid() || (tileToPushOutOf.IsTileWater() && !entityToPush->m_canSwim) )
	{
		PushDiscOutOfFixedAABB2D(entityToPush->m_position, entityToPush->m_physicsRadius, tileToPushOutOf.GetTileBounds());
	}
}

Tile& Map::GetTileFromPos(Vec2 const& pos)
{
	return GetTileFromCoords(GetCoordsFromPos(pos));
}

//#ToDo keep enemies from spawning on top of eachother
void Map::SpawnEnemyInMap(EntityType entityType)
{
	IntVec2 spawnCoords;
	for (int count = 0; count < 10000; count++)
	{
		spawnCoords.x = g_randGen->RollRandomIntInRange(1, m_dimensions.x - 1);
		spawnCoords.y = g_randGen->RollRandomIntInRange(1, m_dimensions.y - 1);

		if (spawnCoords.x <= 5 && spawnCoords.y <= 5)
		{
			continue;
		}
		else if (spawnCoords.x >= (m_dimensions.x - 1) - m_config.m_endBunkerSize && spawnCoords.y >= (m_dimensions.y - 1) - m_config.m_endBunkerSize)
		{
			continue;
		}
		if (!GetTileFromCoords(spawnCoords).IsTileSolid() && !GetTileFromCoords(spawnCoords).IsTileWater())
		{
			Vec2 spawnPos = Vec2((float)spawnCoords.x + .5f, (float)spawnCoords.y + .5f);
			spawnPos += g_randGen->RollRandomNormalizedVec2() * .01f;
			SpawnNewEntity(entityType, spawnPos, 0.f);
			return;
		}
	}
}

Tile& Map::GetTileFromCoords(int xCoord, int yCoord)
{
	IntVec2 coords(xCoord, yCoord);
	if (coords.x < 0)
	{
		coords.x = 0;
	}
	if (coords.y < 0)
	{
		coords.y = 0;
	}
	if (coords.x >= m_dimensions.x)
	{
		coords.x = m_dimensions.x - 1;
	}
	if (coords.y >= m_dimensions.y)
	{
		coords.y = m_dimensions.y - 1;
	}
	int tileIdx = coords.x + (coords.y * m_dimensions.x);
	return m_tiles[tileIdx];
}

Tile& Map::GetTileFromCoords(IntVec2 const& tileCoords)
{
	IntVec2 coords = tileCoords;
	if (coords.x < 0)
	{
		coords.x = 0;
	}
	if (coords.y < 0)
	{
		coords.y = 0;
	}
	if (coords.x >= m_dimensions.x)
	{
		coords.x = m_dimensions.x - 1;
	}
	if (coords.y >= m_dimensions.y)
	{
		coords.y = m_dimensions.y - 1;
	}
	int tileIdx = coords.x +  (coords.y * m_dimensions.x);
	return m_tiles[tileIdx];
}

void Map::AddVertsForTile(std::vector<Vertex_PCU>& verts, Tile const& tile) const
{
	Vec2 tileMins(static_cast<float>(tile.m_tileCoords.x), static_cast<float>(tile.m_tileCoords.y));
	Vec2 tileMaxs = Vec2(tileMins.x + 1.f, tileMins.y + 1.f);
	TileDefinition const& tileDef = *tile.m_tileDef;
	AddVertsForAABB2D(verts, tileMins, tileMaxs, tileDef.m_tintColor, tileDef.m_uvAtMins, tileDef.m_uvAtMaxs);
}

void Map::Update(float deltaSeconds)
{
	UpdateMapEntities(deltaSeconds);
	CheckForBulletCollision();
	PushEntitiesOutOfEachother();
	PushEntitiesOutOfWalls();
	g_theGame->m_worldCamera.SetCameraPos(GetFirstPlayer()->m_position);
	ConstrainWorldCameraToMapBounds();
	DeleteGarbageEntities();
	CheckForTransitionToNewMap();
}

void Map::ConstrainWorldCameraToMapBounds()
{
	Vec2 mapMin(0, 0);
	Vec2 mapMax (static_cast<float>(m_dimensions.x), static_cast<float>(m_dimensions.y));
	Camera& worldCam = g_theGame->m_worldCamera;
	Vec2 camMins = worldCam.GetOrthoBottomLeft();
	Vec2 camMaxs = worldCam.GetOrthoTopRight();
	Vec2 camDimensions = worldCam.GetOrthoDimensions();
	if (camMins.x < mapMin.x)
	{
		worldCam.SetCameraPos( Vec2(mapMin.x + camDimensions.x * .5f, worldCam.GetCameraPos().y) );
	}
	if (camMins.y < mapMin.y)
	{
		worldCam.SetCameraPos(Vec2(worldCam.GetCameraPos().x, mapMin.y + camDimensions.y * .5f));
	}
	if (camMaxs.x > mapMax.x)
	{
		worldCam.SetCameraPos(Vec2(mapMax.x - camDimensions.x * .5f, worldCam.GetCameraPos().y));
	}
	if (camMaxs.y > mapMax.y)
	{
		worldCam.SetCameraPos(Vec2(worldCam.GetCameraPos().x, mapMax.y - camDimensions.y * .5f));
	}
}

Entity* Map::SpawnNewEntity(EntityType type, Vec2 const& position, float orientationDegrees)
{
	switch (type)
	{
	case ENTITY_TYPE_GOOD_PLAYER:
	{
		Player* player = new Player(position, orientationDegrees);
		AddEntityToMap(player);
		return player;
	}
	case ENTITY_TYPE_EVIL_SCORPIO:
	{
		Scorpio* scorpio = new Scorpio(position, orientationDegrees);
		AddEntityToMap(scorpio);
		return scorpio;
	}
	case ENTITY_TYPE_EVIL_LEO:
	{
		Leo* leo = new Leo(position, orientationDegrees);
		AddEntityToMap(leo);
		return leo;	
	}
	case ENTITY_TYPE_EVIL_ARIES:
	{
		Aries* aries = new Aries(position, orientationDegrees);
		AddEntityToMap(aries);
		return aries;	
	}
	case ENTITY_TYPE_GOOD_BULLET:
	{
		Bullet* goodBullet = new Bullet(position, orientationDegrees, ENTITY_TYPE_GOOD_BULLET);
		AddEntityToMap(goodBullet);
		return goodBullet;
	}
	case ENTITY_TYPE_EVIL_BULLET:
	{
		Bullet* evilBullet = new Bullet(position, orientationDegrees, ENTITY_TYPE_EVIL_BULLET);
		AddEntityToMap(evilBullet);
		return evilBullet;	
	}
	case ENTITY_TYPE_EVIL_CAPRICORN:
	{
		Capricorn* evilCap = new Capricorn(position, orientationDegrees);
		AddEntityToMap(evilCap);
		return evilCap;
	}
	case ENTITY_TYPE_EVIL_MISSILE:
	{
		GuidedMissile* missile = new GuidedMissile(position, orientationDegrees, FACTION_EVIL);
		AddEntityToMap(missile);
		return missile;
	}
	case ENTITY_TYPE_EXPLOSION:
	{
		Explosion* explosion = new Explosion(position, orientationDegrees, *ANIM_DEFINITION_EXPLOSION, false);
		AddEntityToMap(explosion);
		return explosion;
	}
	case ENTITY_TYPE_MUZZLE_FLASH:
	{
		Explosion* explosion = new Explosion(position, orientationDegrees, *ANIM_DEFINITION_EXPLOSION, true);
		AddEntityToMap(explosion);
		return explosion;
	}
	case ENTITY_TYPE_GOOD_FLAME_BULLET:
	{
		Bullet* goodFlameBullet = new Bullet(position, orientationDegrees, ENTITY_TYPE_GOOD_FLAME_BULLET);
		AddEntityToMap(goodFlameBullet);
		return goodFlameBullet;
	}
	default:
		return nullptr;
	}
}

Explosion* Map::SpawnNewExplosion(Vec2 const& position, float scale, float duration, bool isMuzzleFlash)
{
	Explosion* explosion;
	if (isMuzzleFlash)
	{
		explosion = dynamic_cast<Explosion*>( SpawnNewEntity(ENTITY_TYPE_MUZZLE_FLASH, position, g_randGen->RollRandomFloatInRange(0.f, 360.f)) );
	}
	else
	{
		explosion = dynamic_cast<Explosion*>(SpawnNewEntity(ENTITY_TYPE_EXPLOSION, position, g_randGen->RollRandomFloatInRange(0.f, 360.f)));
	}
	explosion->m_scale = scale;
	explosion->m_duration = duration;
	return explosion;
}

void Map::AddEntityToMap(Entity* e)
{
	e->m_map = this;
	m_allEntities.push_back(e);
	m_entityListsByType[e->m_entityType].push_back(e);
	m_entityListsByFaction[e->m_entityFaction].push_back(e);
	if (IsEntityEvilTank(e->m_entityType))
	{
		m_evilTankList.push_back(e);
	}
}

void Map::SetHealthOfTiles()
{
	for (int y = 0; y < m_dimensions.y; y++)
	{
		for (int x = 0; x < m_dimensions.x; x++)
		{
			Tile& currTile = GetTileFromCoords(x, y);
			if (currTile.m_tileDef->m_maxHealth > 0)
			{
				currTile.m_health = currTile.m_tileDef->m_maxHealth;
			}
		}
	}
}

bool Map::IsEntityEvilTank(EntityType type)
{
	return type == ENTITY_TYPE_EVIL_CAPRICORN || type == ENTITY_TYPE_EVIL_LEO || type == ENTITY_TYPE_EVIL_ARIES;
}



void Map::RemoveEntityFromMap(Entity* e)
{
	e->m_map = nullptr;
	RemoveEntityFromList(e, m_allEntities);
	RemoveEntityFromList(e, m_entityListsByType[e->m_entityType]);
	RemoveEntityFromList(e, m_entityListsByFaction[e->m_entityFaction]);
	if (IsEntityEvilTank(e->m_entityType))
	{
		RemoveEntityFromList(e, m_evilTankList);
	}
}

IntVec2 Map::GetCoordsFromPos(Vec2 const& pos)
{
	IntVec2 coords((int)pos.x, (int)pos.y);
	if (coords.x < 0)
	{
		coords.x = 0;
	}
	if (coords.y < 0)
	{
		coords.y = 0;
	}
	if (coords.x >= m_dimensions.x)
	{
		coords.x = m_dimensions.x - 1;
	}
	if (coords.y >= m_dimensions.y)
	{
		coords.y = m_dimensions.y - 1;
	}
	return coords;
}

bool Map::IsPointInSolid(Vec2 const& point, bool treatWaterAsSolid)
{
	return GetTileFromPos(point).IsTileSolid() || (treatWaterAsSolid && GetTileFromPos(point).IsTileWater());
}

void Map::RemoveEntityFromList(Entity* e, std::vector<Entity*>& entityList)
{
	for (size_t i = 0; i < entityList.size(); i++)
	{
		if (entityList[i] == e)
		{
			entityList.erase(entityList.begin() + i);
			return;
		}
	}
}

Vec2 Map::GetRandomReachableTargetPos(IntVec2 const& currCoords, TileHeatMap* Out_TargetDistanceField, bool treatWaterAsReachable)
{
	Vec2 tilePos;
	bool reachablePosFound = false;
	int tries = 0;
	while (!reachablePosFound)
	{
		tilePos.x = (float)g_randGen->RollRandomIntInRange(1, m_dimensions.x - 2) + .5f;
		tilePos.y = (float)g_randGen->RollRandomIntInRange(1, m_dimensions.y - 2) + .5f;
		if (GetTileFromPos(tilePos).IsTileSolid() ||
			(!treatWaterAsReachable && GetTileFromPos(tilePos).IsTileWater()) ||
			IsScorpioInTile(GetCoordsFromPos(tilePos)))
		{
			continue;
		}
		tries++;
		if (tries > 100)
		{
			reachablePosFound = true; 
		}
		IntVec2 tileCoords =  IntVec2((int)tilePos.x, (int)tilePos.y);
		PopulateDistanceField(*Out_TargetDistanceField, tileCoords, SPECIAL_TILE_HEAT, !treatWaterAsReachable);
		if (Out_TargetDistanceField->GetHeatValue(currCoords) != SPECIAL_TILE_HEAT)
		{
			reachablePosFound = true;
		}
	}
	return tilePos;
}

void Map::RespawnPlayer()
{
	for (size_t i = 0; i < m_entityListsByType[ENTITY_TYPE_GOOD_PLAYER].size(); i++)
	{
		Player* player = dynamic_cast<Player*>(m_entityListsByType[ENTITY_TYPE_GOOD_PLAYER][i]);
		if (!player->IsAlive())
		{
			player->Respawn();
		}
	}
}

bool Map::HasLineOfSight(Entity* entityLooking, float visionRadius, Entity* entityToFind)
{
	Vec2 rayCastDisplacment = entityToFind->m_position - entityLooking->m_position;
	if (rayCastDisplacment.GetLength() > visionRadius)
	{
		return false;
	}
	RaycastResult2D result = m_solidMapForAmphibians.ImprovedRaycastVsSpecialHeat(entityLooking->m_position, rayCastDisplacment.GetNormalized(), rayCastDisplacment.GetLength(), SPECIAL_TILE_HEAT);
	return !result.m_didImpact;
}

void Map::CycleDebugHeatMap()
{
	switch (m_heatMapToDraw)
	{
	case NO_HEAT_MAP:
	{
		m_heatMapToDraw = HEAT_MAP_DISTANCE_FROM_START;
		break;
	}
	case HEAT_MAP_DISTANCE_FROM_START:
	{
		m_heatMapToDraw = HEAT_MAP_SOLID_AMPHIBIANS;
		break;
	}
	case HEAT_MAP_SOLID_AMPHIBIANS:
	{
		m_heatMapToDraw = HEAT_MAP_SOLID_LAND_BASED;
		break;
	}
	case HEAT_MAP_SOLID_LAND_BASED:
	{
		m_heatMapToDraw = HEAT_MAP_DISTANCE_TO_ENTITY_GOAL;
		break;
	}
	case HEAT_MAP_DISTANCE_TO_ENTITY_GOAL:
	{
		m_heatMapToDraw = NO_HEAT_MAP;
		break;
	}
	default:
		break;
	};
}

void Map::RepopulateSolidMaps()
{
	PopulateSolidMap(m_solidMapForAmphibians, false);
	PopulateSolidMap(m_solidMapForLandbased, true);
	PopulateSolidMap(m_opaqueMap, false, false);
	PopulateDistanceField(m_distanceFieldFromStart, IntVec2(1,1), SPECIAL_TILE_HEAT, true);
}

void Map::PopulateSolidMap(TileHeatMap& out_distanceField, bool treatWaterAsSolid, bool treatScorpioAsSolid)
{
	out_distanceField.SetAllValues(0.f);
	for (int y = 0; y < m_dimensions.y; ++y)
	{
		for (int x = 0; x < m_dimensions.x; ++x)
		{
			//treat scorpio tiles as solid
			Tile& currTile = GetTileFromCoords(x, y);
			
			if (treatScorpioAsSolid)
			{
				if (IsScorpioInTile(IntVec2(x, y)))
				{
					out_distanceField.SetHeatValue(IntVec2(x, y), SPECIAL_TILE_HEAT);
				}
			}
			
			if (currTile.IsTileSolid() || (currTile.IsTileWater() && treatWaterAsSolid))
			{
				out_distanceField.SetHeatValue(IntVec2(x, y), SPECIAL_TILE_HEAT);
			}
		}
	}
}

void Map::PopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 startCoords, float maxCost, bool treatWaterAsSolid)
{
	//set everything to the maxCost
	out_distanceField.SetAllValues(maxCost);
	std::queue<IntVec2> tilesToExplore;
	IntVec2 dimensions = out_distanceField.GetDimensions();
	out_distanceField.SetHeatValue(startCoords, 0.f);
	tilesToExplore.push(startCoords);
	while (tilesToExplore.size() > 0)
	{
		IntVec2 tileBeingExplored = tilesToExplore.front();
		tilesToExplore.pop();
		float tileBeingExploredHeat = out_distanceField.GetHeatValue(tileBeingExplored);
		//tile is reachable and has not been explored yet
		EvaluateTileInDistanceField(tileBeingExplored + IntVec2::WEST, tileBeingExploredHeat, out_distanceField, tilesToExplore, treatWaterAsSolid);
		EvaluateTileInDistanceField(tileBeingExplored + IntVec2::NORTH, tileBeingExploredHeat, out_distanceField, tilesToExplore, treatWaterAsSolid);
		EvaluateTileInDistanceField(tileBeingExplored + IntVec2::EAST, tileBeingExploredHeat, out_distanceField, tilesToExplore, treatWaterAsSolid);
		EvaluateTileInDistanceField(tileBeingExplored + IntVec2::SOUTH, tileBeingExploredHeat, out_distanceField, tilesToExplore, treatWaterAsSolid);
	}
}

void Map::EvaluateTileInDistanceField(IntVec2 const& tileCoords, float prevTileHeat, TileHeatMap& out_distanceField, std::queue<IntVec2>& out_tilesToExplore, bool treatWaterAsSolid)
{
	//non traversable if there is a scorpio or tile is solid
	bool isSolid = GetTileFromCoords(tileCoords).IsTileSolid() || (GetTileFromCoords(tileCoords).IsTileWater() && treatWaterAsSolid);
	if (isSolid)
	{
		return;
	}
	if (IsScorpioInTile(tileCoords))
	{
		return;
	}
	if (tileCoords.x < 0 || tileCoords.x > m_dimensions.x - 1 || tileCoords.y < 0 || tileCoords.y > m_dimensions.y - 1)
	{
		return;
	}

	float currHeat = prevTileHeat + 1.f;
	if (out_distanceField.GetHeatValue(tileCoords) > currHeat)
	{
		if (currHeat > m_maxDistanceFieldHeat)
		{
			m_maxDistanceFieldHeat = currHeat;
		}
		out_distanceField.SetHeatValue(tileCoords, currHeat);
		out_tilesToExplore.push(tileCoords);
	}
}