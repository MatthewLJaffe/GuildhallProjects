#include "Map.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Game/Actor.hpp"
#include "Game/Player.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/AIController.hpp"
#include "Game/Weapon.hpp"

Actor* Map::SpawnActor(const SpawnInfo& spawnInfo)
{
	unsigned int actorIdx = (unsigned int)m_allActors.size();
	for (unsigned int i = 0; i < (unsigned int)m_allActors.size(); i++)
	{
		if (m_allActors[i] == nullptr)
		{
			actorIdx = i;
		}
	}
	m_actorSalt++;
	Actor* actorToSpawn = new Actor(spawnInfo, ActorUID(m_actorSalt, actorIdx), this);
	if (actorIdx == (unsigned int)m_allActors.size())
	{
		m_allActors.push_back(actorToSpawn);
	}
	else
	{
		m_allActors[actorIdx] = actorToSpawn;
	}
	return actorToSpawn;
}

void Map::SpawnActor(Actor* actorToSpawn)
{
	unsigned int actorIdx = (unsigned int)m_allActors.size();
	for (unsigned int i = 0; i < (unsigned int)m_allActors.size(); i++)
	{
		if (m_allActors[i] == nullptr)
		{
			actorIdx = i;
		}
	}
	m_actorSalt++;
	actorToSpawn->SetActorUID(ActorUID(m_actorSalt, actorIdx));
	if (actorIdx == (unsigned int)m_allActors.size())
	{
		m_allActors.push_back(actorToSpawn);
	}
	else
	{
		m_allActors[actorIdx] = actorToSpawn;
	}
	actorToSpawn->m_map = this;
	for (int i = 0; i < (int)actorToSpawn->m_weapons.size(); i++)
	{
		actorToSpawn->m_weapons[i]->m_owningActor = actorToSpawn->GetActorUID();
	}
}

Actor* Map::GetActorByUID(const ActorUID uid) const
{
	if (uid.GetIndex() >= m_allActors.size())
	{
		return nullptr;
	}
	if (m_allActors[uid.GetIndex()] != nullptr && m_allActors[uid.GetIndex()]->GetActorUID() == uid)
	{
		return m_allActors[uid.GetIndex()];
	}
	return nullptr;
}

Actor* Map::GetFirstActorWithName(std::string name) const
{
	for (size_t i = 0; i < m_allActors.size(); i++)
	{
		if (m_allActors[i] != nullptr && m_allActors[i]->m_actorDefinition->m_name == name)
		{
			return m_allActors[i];
		}
	}
	return nullptr;
}

Map::Map(MapDefinition const& definition)
	: m_def(definition)
{
	m_dimensions = m_def.m_tilesImage.GetDimensions();
}

Map::~Map()
{
	delete m_vertexBuffer;
	delete m_indexBuffer;
}

void Map::InitializeMap()
{
	AddActors();

	m_tiles.reserve((size_t)m_dimensions.x * m_dimensions.y);

	size_t vertsSize = (size_t)16 * m_dimensions.x * m_dimensions.y;
	m_vertexes.reserve(vertsSize);

	size_t indexesSize = (size_t)6 * m_dimensions.x * m_dimensions.y;
	m_indexes.reserve(indexesSize);

	for (int y = 0; y < m_dimensions.y; y++)
	{
		for (int x = 0; x < m_dimensions.x; x++)
		{

			//spawn tiles
			TileDefinition const* tileDef = TileDefinition::GetTileDefFromColor(m_def.m_tilesImage.GetTexelColor(IntVec2(x, y)));
			AABB3 tileBounds(Vec3((float)x, (float)y, 0.f), Vec3((float)(x + 1), (float)(y + 1), 1.f));

			if (tileDef->m_name == "Empty")
			{
				m_tiles.push_back(Tile(tileBounds, TileDefinition::GetTileDefFromName("Empty")));
				continue;
			}

			m_tiles.push_back(Tile(tileBounds, tileDef));



			Vec3& mins = tileBounds.m_mins;
			Vec3& maxs = tileBounds.m_maxs;
			Vec3 frontLeftBottom = Vec3(maxs.x, maxs.y, mins.z);
			Vec3 frontRightBottom = Vec3(maxs.x, mins.y, mins.z);
			Vec3 frontLeftTop = Vec3(maxs.x, maxs.y, maxs.z);
			Vec3 frontRightTop = Vec3(maxs.x, mins.y, maxs.z);
			Vec3 backLeftBottom = Vec3(mins.x, maxs.y, mins.z);
			Vec3 backRightBottom = Vec3(mins.x, mins.y, mins.z);
			Vec3 backLeftTop = Vec3(mins.x, maxs.y, maxs.z);
			Vec3 backRightTop = Vec3(mins.x, mins.y, maxs.z);

			if (tileDef->m_isSolid)
			{
				//front
				AddVertsForQuad3D(m_vertexes, m_indexes, frontRightBottom, frontLeftBottom, frontLeftTop, frontRightTop, Rgba8::WHITE, tileDef->GetWallUVs(*m_def.m_spriteSheet));

				//back
				AddVertsForQuad3D(m_vertexes, m_indexes, backLeftBottom, backRightBottom, backRightTop, backLeftTop, Rgba8::WHITE, tileDef->GetWallUVs(*m_def.m_spriteSheet));

				//left
				AddVertsForQuad3D(m_vertexes, m_indexes, frontLeftBottom, backLeftBottom, backLeftTop, frontLeftTop, Rgba8::WHITE, tileDef->GetWallUVs(*m_def.m_spriteSheet));

				//right
				AddVertsForQuad3D(m_vertexes, m_indexes, backRightBottom, frontRightBottom, frontRightTop, backRightTop, Rgba8::WHITE, tileDef->GetWallUVs(*m_def.m_spriteSheet));
			}
			else
			{
				//top
				AddVertsForQuad3D(m_vertexes, m_indexes, backRightTop, backLeftTop,frontLeftTop, frontRightTop, Rgba8::WHITE, tileDef->GetCeilingUVs(*m_def.m_spriteSheet));

				//bottom
				AddVertsForQuad3D(m_vertexes, m_indexes, backLeftBottom, backRightBottom, frontRightBottom, frontLeftBottom, Rgba8::WHITE, tileDef->GetFloorUVs(*m_def.m_spriteSheet));
			}
			if (tileDef->m_isLit)
			{
				PointLight leftLight;
				leftLight.exponentialAttenuation = 10.f;
				leftLight.linearAttenuation = 10.f;
				leftLight.intensity = 5.f;
				leftLight.pointLightColor = Vec4(1.f, 1.f, 1.f, 1.f);
				leftLight.position = backRightBottom * .5f + frontRightTop * .5f;
				leftLight.position -= Vec3(0.f, .25f, 0.f);
				AddPointLight(leftLight);

				PointLight rightLight;
				rightLight.exponentialAttenuation = 10.f;
				rightLight.linearAttenuation = 10.f;
				rightLight.intensity = 5.f;
				rightLight.pointLightColor = Vec4(1.f, 1.f, 1.f, 1.f);
				rightLight.position = backLeftBottom * .5f + frontLeftTop * .5f;
				rightLight.position += Vec3(0.f, .25f, 0.f);
				AddPointLight(rightLight);

				PointLight backLight;
				backLight.exponentialAttenuation = 10.f;
				backLight.linearAttenuation = 10.f;
				backLight.intensity = 5.f;
				backLight.pointLightColor = Vec4(1.f, 1.f, 1.f, 1.f);
				backLight.position = backLeftBottom * .5f + backRightTop * .5f;
				backLight.position -= Vec3(.25f, .0f, 0.f);
				AddPointLight(backLight);

				PointLight frontLight;
				frontLight.exponentialAttenuation = 10.f;
				frontLight.linearAttenuation = 10.f;
				frontLight.intensity = 5.f;
				frontLight.pointLightColor = Vec4(1.f, 1.f, 1.f, 1.f);
				frontLight.position = frontLeftBottom * .5f + frontRightTop * .5f;
				frontLight.position -= Vec3(.25f, .0f, 0.f);
				AddPointLight(frontLight);
			}
		}
	}
	size_t vertexBufferSize = sizeof(Vertex_PCUTBN) * m_vertexes.size();
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(vertexBufferSize);
	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), vertexBufferSize, m_vertexBuffer);

	size_t indexBufferSize = sizeof(unsigned int) * m_indexes.size();
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(indexBufferSize);
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), indexBufferSize, m_indexBuffer);
}

void Map::AddActors()
{
	for (size_t i = 0; i < m_def.m_spawnInfos.size(); i++)
	{
		SpawnActor(m_def.m_spawnInfos[i]);
	}
}

void Map::PushActorsOutOfEachother()
{
	for (size_t currActorIdx = 0; currActorIdx < m_allActors.size() - 1; currActorIdx++)
	{
		for (size_t otherActorIdx = currActorIdx + 1; otherActorIdx < m_allActors.size(); otherActorIdx++)
		{
			Actor* currActor = m_allActors[currActorIdx];
			Actor* otherActor = m_allActors[otherActorIdx];

			if (!IsActorAlive(currActor) || !IsActorAlive(otherActor))
			{
				continue;
			}

			//owning actor and actor it owns should not collide
			if (currActor->m_owningActor == otherActor->GetActorUID() || otherActor->m_owningActor == currActor->GetActorUID())
			{
				continue;
			}
			//check if actors both collide
			if (!currActor->m_actorDefinition->m_collidesWithActors || !otherActor->m_actorDefinition->m_collidesWithActors)
			{
				continue;
			}
			//make sure actors overlap on z
			if (!currActor->GetPhysicsCylinderZMinMax().IsOverlappingWith(otherActor->GetPhysicsCylinderZMinMax()))
			{
				continue;
			}
			//make sure actors of same type should collide
			if (currActor->m_actorDefinition->m_name == otherActor->m_actorDefinition->m_name && !currActor->m_actorDefinition->m_collidesWithSelf)
			{
				continue;
			}


			Vec2 currActorXY = currActor->GetPhysicsCylinderCenter();
			Vec2 otherActorXY = otherActor->GetPhysicsCylinderCenter();
			bool collided = false;

			//push actors out of eachother
			if (!currActor->m_actorDefinition->m_isStatic && !otherActor->m_actorDefinition->m_isStatic)
			{
				if (DoDiscsOverlap(currActorXY, currActor->m_physicsCylinderRadius, otherActorXY, otherActor->m_physicsCylinderRadius))
				{
					PushDiscsOutOfEachOther2D(currActorXY, currActor->m_physicsCylinderRadius, otherActorXY, otherActor->m_physicsCylinderRadius);
					collided = true;
				}
			}

			//push curr out of other
			else if (!currActor->m_actorDefinition->m_isStatic && otherActor->m_actorDefinition->m_isStatic)
			{
				if (PushDiscOutOfFixedDisc2D(currActorXY, currActor->m_physicsCylinderRadius, otherActorXY, otherActor->m_physicsCylinderRadius))
				{
					collided = true;
				}
			}

			//push other out of curr
			else if (currActor->m_actorDefinition->m_isStatic && !otherActor->m_actorDefinition->m_isStatic)
			{
				if (PushDiscOutOfFixedDisc2D(otherActorXY, otherActor->m_physicsCylinderRadius, currActorXY, currActor->m_physicsCylinderRadius))
				{
					collided = true;
				}
			}

			currActor->m_position = Vec3(currActorXY.x, currActorXY.y, currActor->m_position.z);
			otherActor->m_position = Vec3(otherActorXY.x, otherActorXY.y, otherActor->m_position.z);
			if (collided)
			{
				currActor->OnCollide(otherActor);
				otherActor->OnCollide(currActor);
			}
		}
	}
}

void Map::PushActorsOutOfWorld()
{
	for (size_t i = 0; i < m_allActors.size(); i++)
	{
		Actor* actorToPush = m_allActors[i];
		if (!IsActorAlive(actorToPush))
		{
			continue;
		}
		if (!actorToPush->m_actorDefinition->m_collidesWithWorld)
		{
			continue;
		}
		ConstrainActorToMapDimensions(actorToPush);
		IntVec2 actorCoords = GetCoordsFromPos(actorToPush->m_position);
		bool collidedWithWorld = false;

		//north
		PushActorOutOfTile(actorToPush, GetTileFromCoords(IntVec2(actorCoords.x, actorCoords.y + 1)), collidedWithWorld);
		//south
		PushActorOutOfTile(actorToPush, GetTileFromCoords(IntVec2(actorCoords.x, actorCoords.y - 1)), collidedWithWorld);
		//east
		PushActorOutOfTile(actorToPush, GetTileFromCoords(IntVec2(actorCoords.x + 1, actorCoords.y)), collidedWithWorld);
		//west
		PushActorOutOfTile(actorToPush, GetTileFromCoords(IntVec2(actorCoords.x - 1, actorCoords.y)), collidedWithWorld);
		//northeast
		PushActorOutOfTile(actorToPush, GetTileFromCoords(IntVec2(actorCoords.x + 1, actorCoords.y + 1)), collidedWithWorld);
		//northwest
		PushActorOutOfTile(actorToPush, GetTileFromCoords(IntVec2(actorCoords.x - 1, actorCoords.y + 1)), collidedWithWorld);
		//southeast
		PushActorOutOfTile(actorToPush, GetTileFromCoords(IntVec2(actorCoords.x + 1, actorCoords.y - 1)), collidedWithWorld);
		//southwest
		PushActorOutOfTile(actorToPush, GetTileFromCoords(IntVec2(actorCoords.x - 1, actorCoords.y - 1)), collidedWithWorld);
		

		//push out of ceiling
		if (actorToPush->GetPhysicsCylinderZMinMax().m_max > 1.f)
		{
			collidedWithWorld = true;
			actorToPush->m_position.z = 1.f - actorToPush->m_physicsCylinderHeight;
		}

		//push out of floor
		if (actorToPush->m_position.z < 0.f)
		{
			collidedWithWorld = true;
			actorToPush->m_position.z = 0.f;
		}

		if (collidedWithWorld)
		{
			if (actorToPush->m_actorDefinition->m_dieOnCollide)
			{
				actorToPush->KillActor();
			}
		}
	}
}

void Map::PushActorOutOfTile(Actor* actorToPush, Tile const& tile, bool& didCollide)
{
	if (tile.IsTileSolid())
	{
		Vec2 actorXY = actorToPush->GetPhysicsCylinderCenter();
		if (PushDiscOutOfFixedAABB2D(actorXY, actorToPush->m_physicsCylinderRadius, tile.GetBounds2D()))
		{
			didCollide = true;
		}
		actorToPush->m_position = Vec3(actorXY.x, actorXY.y, actorToPush->m_position.z);
	}
}

void Map::SpawnPlayer(int playerIdx)
{
	std::vector<SpawnInfo> spawnPoints;
	for (size_t i = 0; i < m_def.m_spawnInfos.size(); i++)
	{
		if (m_def.m_spawnInfos[i].m_actorDefinition->m_name == std::string("SpawnPoint"))
		{
			spawnPoints.push_back(m_def.m_spawnInfos[i]);
		}
	}

	int randomSpawnIdx = g_randGen->RollRandomIntInRange(0, (int)spawnPoints.size() - 1);
	SpawnInfo playerSpawnInfo = spawnPoints[randomSpawnIdx];
	playerSpawnInfo.m_actorDefinition = ActorDefinition::GetByName("Marine");
	Actor* playerActor = SpawnActor(playerSpawnInfo);
	g_theGame->m_players[playerIdx]->Possess(playerActor->GetActorUID());
}

void Map::RespawnBinkey(Actor* binkeyActor)
{
	std::vector<SpawnInfo> spawnPoints;
	for (size_t i = 0; i < m_def.m_spawnInfos.size(); i++)
	{
		if (m_def.m_spawnInfos[i].m_actorDefinition->m_name == std::string("BinkeySpawnPoint"))
		{
			spawnPoints.push_back(m_def.m_spawnInfos[i]);
		}
	}

	int bestSpawnInfo = 0;
	float bestDistance = GetDistance3D(spawnPoints[0].m_position, binkeyActor->m_position);
	for (int i = 0; i < (int)spawnPoints.size(); i++)
	{
		float currDistance = GetDistance3D(spawnPoints[i].m_position, binkeyActor->m_position);
		if (currDistance < bestDistance)
		{
			bestDistance = currDistance;
			bestSpawnInfo = i;
		}
	}

	SpawnInfo binkeySpawnInfo = spawnPoints[bestSpawnInfo];
	binkeyActor->m_position = binkeySpawnInfo.m_position;
	binkeySpawnInfo.m_orientation = binkeySpawnInfo.m_orientation;
	binkeyActor->m_currentHealth = binkeyActor->m_actorDefinition->m_health;
	binkeyActor->m_isDead = false;
	binkeyActor->TransitionAnimationState("Walk");
}

void Map::SpawnPlayer(Actor* playerActor)
{
	std::vector<SpawnInfo> spawnPoints;
	for (size_t i = 0; i < m_def.m_spawnInfos.size(); i++)
	{
		if (m_def.m_spawnInfos[i].m_actorDefinition->m_name == std::string("SpawnPoint"))
		{
			spawnPoints.push_back(m_def.m_spawnInfos[i]);
		}
	}

	int randomSpawnIdx = g_randGen->RollRandomIntInRange(0, (int)spawnPoints.size() - 1);
	SpawnInfo playerSpawnInfo = spawnPoints[randomSpawnIdx];
	playerActor->m_position = playerSpawnInfo.m_position;
	playerActor->m_orientation = playerSpawnInfo.m_orientation;
	SpawnActor(playerActor);
}

void Map::DeleteGarbageActors()
{
	for (size_t i = 0; i < m_allActors.size(); i++)
	{
		if (m_allActors[i] != nullptr && m_allActors[i]->m_isGarbage)
		{
			delete m_allActors[i];
			m_allActors[i] = nullptr;
		}
	}
}


bool Map::IsActorAlive(Actor* actor)
{
	return actor != nullptr && actor->IsAlive();
}

bool Map::IsNonGarbage(Actor* actor)
{
	return actor != nullptr && !actor->m_isGarbage;
}

void Map::HandlePlayerRespawn()
{
	for (size_t i = 0; i < g_theGame->m_players.size(); i++)
	{
		if (GetActorByUID(g_theGame->m_players[i]->m_currentlyPossesedActor) == nullptr)
		{
			SpawnPlayer((int)i);
		}
	}
}

void Map::ConstrainActorToMapDimensions(Actor* actor)
{
	actor->m_position.x = Clamp(actor->m_position.x, 1.f, (float)m_dimensions.x - 1.00001f);
	actor->m_position.y = Clamp(actor->m_position.y, 1.f, (float)m_dimensions.y - 1.00001f);
}

void Map::RaycastMapWalls(RaycastResultDoomenstein& result, Vec3 const& rayStartPos, Vec3 const& rayFwdNormal, float rayMaxLength)
{
	//set up curr raycast
	RaycastResultDoomenstein currResult;
	currResult.m_rayFwdNormal = rayFwdNormal;
	currResult.m_rayMaxLength = rayMaxLength;
	currResult.m_rayStartPos = rayStartPos;

	//check if start pos is a hit
	IntVec2 startTileCoords(RoundDownToInt(rayStartPos.x), RoundDownToInt(rayStartPos.y));
	if (IsInBounds(startTileCoords) && GetTileFromCoords(startTileCoords).IsTileSolid() && rayStartPos.z > 0.f && rayStartPos.z < 1.f)
	{
		result.m_impactPos = rayStartPos;
		result.m_impactDist = 0;
		result.m_didImpact = true;
		result.m_impactNormal = -rayFwdNormal;
		result.m_rayFwdNormal = rayFwdNormal;
		result.m_rayMaxLength = rayMaxLength;
		result.m_rayStartPos = rayStartPos;
		return;
	}

	int xStepDir = rayFwdNormal.x > 0 ? 1 : -1;
	int yStepDir = rayFwdNormal.y > 0 ? 1 : -1;
	float distanceLeft = rayMaxLength;
	Vec2 currPosXY = rayStartPos.GetXY();
	Vec2 rayFwdNormalXY = rayFwdNormal.GetXY();
	rayFwdNormalXY = rayFwdNormalXY.GetNormalized();
	float rayLengthXY = (rayFwdNormal * rayMaxLength).GetLengthXY();
	Vec3 rayEndPos = rayStartPos + rayFwdNormal * rayMaxLength;
	IntVec2 currTileCoords(RoundDownToInt(currPosXY.x), RoundDownToInt(currPosXY.y));

	while (distanceLeft > 0)
	{
		//figure out how long to travel along forward normal to reach next x and y
		int nextX = currTileCoords.x + (xStepDir + 1) / 2;
		float distToNextX = static_cast<float>(nextX) - currPosXY.x;
		float forwardLenToNextX = fabsf(distToNextX / rayFwdNormalXY.x);

		int nextY = currTileCoords.y + (yStepDir + 1) / 2;
		float distToNextY = static_cast<float>(nextY) - currPosXY.y;
		float forwardLenToNextY = fabsf(distToNextY / rayFwdNormalXY.y);

		Vec2 nextStepDisplacment;
		if (forwardLenToNextX < forwardLenToNextY)
		{
			nextStepDisplacment = forwardLenToNextX * rayFwdNormalXY;
			currTileCoords.x += xStepDir;
		}
		else
		{
			nextStepDisplacment = forwardLenToNextY * rayFwdNormalXY;
			currTileCoords.y += yStepDir;
		}
		distanceLeft -= nextStepDisplacment.GetLength();
		if (distanceLeft < 0)
		{
			break;
		}
		currPosXY += nextStepDisplacment;

		float currPosT = GetDistance2D(rayStartPos.GetXY(), currPosXY) / rayLengthXY;
		Vec3 currPosXYZ = Vec3::Lerp(rayStartPos, rayEndPos, currPosT);

		if (IsInBounds(currTileCoords) && GetTileFromCoords(currTileCoords).IsTileSolid() && currPosXYZ.z > 0.f && currPosXYZ.z < 1.f)
		{
			//crossing x
			if (forwardLenToNextX < forwardLenToNextY)
			{
				if (xStepDir > 0)
				{
					currResult.m_impactNormal = Vec3(-1.f, 0.f, 0.f);
				}
				else
				{
					currResult.m_impactNormal = Vec3(1.f, 0.f, 0.f);
				}
			}
			//crossing y
			else
			{
				if (yStepDir > 0)
				{
					currResult.m_impactNormal = Vec3(0.f, -1.f, 0.f);
				}
				else
				{
					currResult.m_impactNormal = Vec3(0.f, 1.f, 0.f);
				}
			}
			currResult.m_impactPos = currPosXYZ;
			currResult.m_impactDist = GetDistance3D(rayStartPos, currPosXYZ);
			currResult.m_didImpact = true;
			break;
		}
	}

	//miss
	if (!currResult.m_didImpact)
	{
		return;
	}

	if (currResult.m_didImpact && (currResult.m_impactDist < result.m_impactDist || !result.m_didImpact))
	{
		result = currResult;
	}
}

void Map::RaycastMapFloorAndCeiling(RaycastResultDoomenstein& result, Vec3 const& rayStartPos, Vec3 const& rayFwdNormal, float rayDistance)
{
	Vec3 rayEndPos = rayStartPos + rayFwdNormal * rayDistance;

	//determine if we are raycasting vs floor or ceiling
	Vec3 impactNormal = rayFwdNormal.z > 0 ? Vec3(0.f, 0.f, -1.f) : Vec3(0.f, 0.f, 1.f);
	float impactHeight = rayFwdNormal.z > 0 ? 1.f : 0.f;
	
	float impactT = GetFractionWithinRange(impactHeight, rayStartPos.z, rayEndPos.z);

	//miss
	if (impactT < 0 || impactT > 1)
	{
		return;
	}

	RaycastResultDoomenstein verticalResult;
	verticalResult.m_rayFwdNormal = rayFwdNormal;
	verticalResult.m_rayMaxLength = rayDistance;
	verticalResult.m_rayStartPos = rayStartPos;
	verticalResult.m_didImpact = true;
	verticalResult.m_impactDist = impactT * rayDistance;
	verticalResult.m_impactNormal = impactNormal;
	verticalResult.m_impactPos = rayStartPos + verticalResult.m_impactDist * rayFwdNormal;

	if (verticalResult.m_didImpact && (verticalResult.m_impactDist < result.m_impactDist || !result.m_didImpact))
	{
		result = verticalResult;
	}
}

void Map::RaycastWorldActors(RaycastResultDoomenstein& result, Vec3 const& rayStart, Vec3 const& rayFwdNormal, float rayDistance, ActorUID actorToIgnore)
{
	//raycast vs all actors
	for (size_t i = 0; i < m_allActors.size(); i++)
	{
		if (!IsActorAlive(m_allActors[i]))
		{
			continue;
		}
		if (m_allActors[i]->GetActorUID() == actorToIgnore)
		{
			continue;
		}
		RaycastResultDoomenstein currResult = m_allActors[i]->RaycastVsActor(rayStart, rayFwdNormal, rayDistance);
		if (currResult.m_actorHit == actorToIgnore)
		{
			continue;
		}
		if (currResult.m_didImpact && (currResult.m_impactDist < result.m_impactDist || !result.m_didImpact))
		{
			result = currResult;
		}
	}
}

void Map::Update(float deltaSeconds)
{
	AdjustLightingCommands();

	for (size_t i = 0; i < m_allActors.size(); i++)
	{
		if (IsNonGarbage(m_allActors[i]))
		{
			m_allActors[i]->Update(deltaSeconds);
		}
	}
	PushActorsOutOfEachother();
	PushActorsOutOfWorld();
	
	for (size_t i = 0; i < g_theGame->m_players.size(); i++)
	{
		g_theGame->m_players[i]->UpdateCamera();
	}
	DeleteGarbageActors();
	HandlePlayerRespawn();
}

void Map::Render()
{
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetModelConstants(Mat44(), Rgba8::WHITE);
	Actor* playerActor = g_theGame->GetCurrentlyRenderingPlayer()->GetActor();
	if (playerActor != nullptr)
	{
		g_theRenderer->SetLightingConstants(m_sunDirection.GetNormalized(), m_sunIntensity, m_ambientIntensity);
		g_theRenderer->SetPointLights(m_currNumPointLights, m_pointLights);
	}

	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(m_def.m_spriteSheet->GetTexture());
	g_theRenderer->BindShader(m_def.m_shader);
	g_theRenderer->DrawVertexBufferIndexed(m_vertexBuffer, m_indexBuffer, (int)m_indexes.size(), VertexType::VERTEX_TYPE_PCUTBN);

	for (size_t i = 0; i < m_allActors.size(); i++)
	{
		if (IsNonGarbage(m_allActors[i]))
		{
			m_allActors[i]->Render();
		}
	}
}

RaycastResultDoomenstein Map::RaycastVsMap(Vec3 const& startPos, Vec3 const& rayFwdNormal, float rayDistance, ActorUID actorToIgnore)
{
	RaycastResultDoomenstein closestRaycast;
	RaycastWorldActors(closestRaycast, startPos, rayFwdNormal, rayDistance, actorToIgnore);
	RaycastMapWalls(closestRaycast, startPos, rayFwdNormal, rayDistance);
	RaycastMapFloorAndCeiling(closestRaycast, startPos, rayFwdNormal, rayDistance);
	return closestRaycast;
}

void Map::AdjustLightingCommands()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_sunDirection.x--;
		DebugAddMessage(Stringf("Sun Direction x: %f", m_sunDirection.x), 15.f, 1.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_sunDirection.x++;
		DebugAddMessage(Stringf("Sun Direction x: %f", m_sunDirection.x), 15.f, 1.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_sunDirection.y--;
		DebugAddMessage(Stringf("Sun Direction y: %f", m_sunDirection.y), 15.f, 1.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
	{
		m_sunDirection.y++;
		DebugAddMessage(Stringf("Sun Direction y: %f", m_sunDirection.y), 15.f, 1.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_sunIntensity -= .05f;
		m_sunIntensity = Clamp(m_sunIntensity, 0.f, 1.f);
		DebugAddMessage(Stringf("Sun Intensity: %f", m_sunIntensity), 15.f, 1.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_sunIntensity += .05f;
		m_sunIntensity = Clamp(m_sunIntensity, 0.f, 1.f);
		DebugAddMessage(Stringf("Sun Intensity: %f", m_sunIntensity), 15.f, 1.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_ambientIntensity -= .05f;
		m_ambientIntensity = Clamp(m_ambientIntensity, 0.f, 1.f);
		DebugAddMessage(Stringf("Ambient Intensity: %f", m_ambientIntensity), 15.f, 1.f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
	{
		m_ambientIntensity += .05f;
		m_ambientIntensity = Clamp(m_ambientIntensity, 0.f, 1.f);
		DebugAddMessage(Stringf("Ambient Intensity: %f", m_ambientIntensity), 15.f, 1.f);
	}
}

IntVec2 Map::GetCoordsFromPos(Vec3 const& pos)
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

const Tile& Map::GetTileFromCoords(IntVec2 const& coords)
{
	if (!IsInBounds(coords))
	{
		ERROR_AND_DIE("Trying to get Idx of out of bounds tile");
	}
	int tileIdx = coords.x + m_dimensions.x * coords.y;
	return m_tiles[tileIdx];
}

bool Map::IsInBounds(IntVec2 const& coords)
{
	return (coords.x >= 0 && coords.x < m_dimensions.x && coords.y >= 0 && coords.y < m_dimensions.y);
}

void Map::DebugPossessNext()
{
	if (g_theGame->m_players.size() != 1)
	{
		return;
	}
	Actor* playerActor = g_theGame->m_players[0]->GetActor();
	int currPossesionIdx = 0;
	if (playerActor != nullptr)
	{
		currPossesionIdx = playerActor->GetActorUID().GetIndex();
	}

	//iterate circularly through list of entities
	for (int i = currPossesionIdx + 1; i != currPossesionIdx; i++)
	{
		if (i >= (int)m_allActors.size())
		{
			i = 0;
		}

		Actor* nextActorToPosses = m_allActors[i];
		if (!IsActorAlive(nextActorToPosses) || !nextActorToPosses->m_actorDefinition->m_canBePossesed)
		{
			continue;
		}
		g_theGame->m_players[0]->Possess(nextActorToPosses->GetActorUID());
		break;
	}
}

ActorUID Map::FindTargetForActor(Actor* searchingActor)
{
	ActorUID targetActor = ActorUID::INVALID;
	float nearestTargetDistance = 99999999.f;
	for (size_t i = 0; i < m_allActors.size(); i++)
	{
		if (!IsActorAlive(m_allActors[i]))
		{
			continue;
		}

		if ( searchingActor->IsOpposingFaction(m_allActors[i]) && ( searchingActor->IsInSightline(m_allActors[i]->GetActorUID()) || m_allActors[i]->m_actorDefinition->m_binkeyAI ) )
		{
			if (m_allActors[i]->m_actorDefinition->m_binkeyAI)
			{
				return m_allActors[i]->GetActorUID();
			}

			float currTargetDistance = GetDistance3D(m_allActors[i]->m_position, searchingActor->m_position);
			if (currTargetDistance < nearestTargetDistance)
			{
				targetActor = m_allActors[i]->GetActorUID();
				nearestTargetDistance = currTargetDistance;
			}
		}
	}

	return targetActor;
}

void Map::GetActorsInMeleeSwing(std::vector<Actor*>& actorsInSwing, Vec3 const& swingPos, Vec2 swingDirXY, float swingArc, float swingRange)
{
	for (size_t i = 0; i < m_allActors.size(); i++)
	{
		Actor* currActor = m_allActors[i];
		if (!IsActorAlive(currActor))
		{
			continue;
		}
		if (GetDistanceSquared2D(currActor->m_position.GetXY(), swingPos.GetXY()) > swingRange * swingRange)
		{
			continue;
		}
		Vec2 dispToCurrActorXY = currActor->m_position.GetXY() - swingPos.GetXY();
		float displacmentTheta = dispToCurrActorXY.GetOrientationDegrees();
		float swingTheta = swingDirXY.GetOrientationDegrees();
		if (GetShortestAngularDispDegrees(displacmentTheta, swingTheta) < swingArc * .5f)
		{
			actorsInSwing.push_back(currActor);
		}
	}
}

void Map::AddPointLight(PointLight pointLightToAdd)
{
	if (m_currNumPointLights >= MAX_NUM_POINT_LIGHTS)
	{
		return;
	}
	m_pointLights[m_currNumPointLights] = pointLightToAdd;
	m_currNumPointLights++;
}

void Map::RemovePointLight(int idxToRemoveAt)
{
	for (int i = idxToRemoveAt; i < m_currNumPointLights - 1; i++)
	{
		m_pointLights[i] = m_pointLights[i + 1];
	}
	m_currNumPointLights--;
}

void Map::UpdatePointLight(int idxToUpdate, PointLight updatedValues)
{
	if (idxToUpdate < 0 || idxToUpdate >= m_currNumPointLights)
	{
		return;
	}
	m_pointLights[idxToUpdate] = updatedValues;
}

void Map::SpawnEnemy()
{
	std::vector<SpawnInfo> spawnPoints;
	for (size_t i = 0; i < m_def.m_spawnInfos.size(); i++)
	{
		if (m_def.m_spawnInfos[i].m_actorDefinition->m_name == std::string("EnemySpawnPoint"))
		{
			spawnPoints.push_back(m_def.m_spawnInfos[i]);
		}
	}

	int randomSpawnIdx = g_randGen->RollRandomIntInRange(0, (int)spawnPoints.size() - 1);
	SpawnInfo enemySpawnInfo = spawnPoints[randomSpawnIdx];
	enemySpawnInfo.m_actorDefinition = ActorDefinition::GetByName("Demon");
	SpawnActor(enemySpawnInfo);
}

int Map::GetNumPointLights()
{
	int numPointLights = 0;
	for (int i = 0; i < MAX_NUM_POINT_LIGHTS; i++)
	{
		if (m_pointLights[i].intensity > 0.f)
		{
			numPointLights++;
		}
	}
	return numPointLights;
}

int Map::GetNumEnemies()
{
	int numEnemies = 0;
	for (int i = 0; i < (int)m_allActors.size(); i++)
	{
		if (IsActorAlive(m_allActors[i]) && m_allActors[i]->m_actorDefinition->m_name == "Demon")
		{
			numEnemies++;
		}
	}
	return numEnemies;
}

void Map::DestroyAllActorsWithName(std::string name)
{
	for (size_t i = 0; i < m_allActors.size(); i++)
	{
		if (IsActorAlive(m_allActors[i]) && m_allActors[i]->m_actorDefinition->m_name == name)
		{
			m_allActors[i]->DestroyActor();
		}
	}
}

void Map::DestroyLightTiles()
{
	m_vertexes.clear();
	m_indexes.clear();
	m_tiles.clear();

	m_tiles.reserve((size_t)m_dimensions.x * m_dimensions.y);

	size_t vertsSize = (size_t)16 * m_dimensions.x * m_dimensions.y;
	m_vertexes.reserve(vertsSize);

	size_t indexesSize = (size_t)6 * m_dimensions.x * m_dimensions.y;
	m_indexes.reserve(indexesSize);

	for (int y = 0; y < m_dimensions.y; y++)
	{
		for (int x = 0; x < m_dimensions.x; x++)
		{

			//spawn tiles
			TileDefinition const* tileDef = TileDefinition::GetTileDefFromColor(m_def.m_tilesImage.GetTexelColor(IntVec2(x, y)));
			AABB3 tileBounds(Vec3((float)x, (float)y, 0.f), Vec3((float)(x + 1), (float)(y + 1), 1.f));

			if (tileDef->m_isLit || tileDef->m_name == "Empty")
			{
				m_tiles.push_back(Tile(tileBounds, TileDefinition::GetTileDefFromName("Empty")));
				continue;
			}

			m_tiles.push_back(Tile(tileBounds, tileDef));

			Vec3& mins = tileBounds.m_mins;
			Vec3& maxs = tileBounds.m_maxs;
			Vec3 frontLeftBottom = Vec3(maxs.x, maxs.y, mins.z);
			Vec3 frontRightBottom = Vec3(maxs.x, mins.y, mins.z);
			Vec3 frontLeftTop = Vec3(maxs.x, maxs.y, maxs.z);
			Vec3 frontRightTop = Vec3(maxs.x, mins.y, maxs.z);
			Vec3 backLeftBottom = Vec3(mins.x, maxs.y, mins.z);
			Vec3 backRightBottom = Vec3(mins.x, mins.y, mins.z);
			Vec3 backLeftTop = Vec3(mins.x, maxs.y, maxs.z);
			Vec3 backRightTop = Vec3(mins.x, mins.y, maxs.z);

			if (tileDef->m_isSolid)
			{
				//front
				AddVertsForQuad3D(m_vertexes, m_indexes, frontRightBottom, frontLeftBottom, frontLeftTop, frontRightTop, Rgba8::WHITE, tileDef->GetWallUVs(*m_def.m_spriteSheet));

				//back
				AddVertsForQuad3D(m_vertexes, m_indexes, backLeftBottom, backRightBottom, backRightTop, backLeftTop, Rgba8::WHITE, tileDef->GetWallUVs(*m_def.m_spriteSheet));

				//left
				AddVertsForQuad3D(m_vertexes, m_indexes, frontLeftBottom, backLeftBottom, backLeftTop, frontLeftTop, Rgba8::WHITE, tileDef->GetWallUVs(*m_def.m_spriteSheet));

				//right
				AddVertsForQuad3D(m_vertexes, m_indexes, backRightBottom, frontRightBottom, frontRightTop, backRightTop, Rgba8::WHITE, tileDef->GetWallUVs(*m_def.m_spriteSheet));
			}
			else
			{
				//top
				AddVertsForQuad3D(m_vertexes, m_indexes, backRightTop, backLeftTop, frontLeftTop, frontRightTop, Rgba8::WHITE, tileDef->GetCeilingUVs(*m_def.m_spriteSheet));

				//bottom
				AddVertsForQuad3D(m_vertexes, m_indexes, backLeftBottom, backRightBottom, frontRightBottom, frontLeftBottom, Rgba8::WHITE, tileDef->GetFloorUVs(*m_def.m_spriteSheet));
			}
		}
	}

	if (m_vertexBuffer != nullptr)
	{
		delete m_vertexBuffer;
		m_vertexBuffer = nullptr;
	}
	size_t vertexBufferSize = sizeof(Vertex_PCUTBN) * m_vertexes.size();
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(vertexBufferSize);
	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), vertexBufferSize, m_vertexBuffer);


	if (m_indexBuffer != nullptr)
	{
		delete m_indexBuffer;
		m_indexBuffer = nullptr;
	}
	size_t indexBufferSize = sizeof(unsigned int) * m_indexes.size();
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(indexBufferSize);
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), indexBufferSize, m_indexBuffer);
}

Tile Map::GetTileFromPosition(Vec3 position)
{
	IntVec2 tileCoords((int)position.x, (int)position.y);
	return GetTileFromCoords(tileCoords);
}

void Map::RemoveActor(ActorUID actorToRemove)
{
	if (m_allActors[actorToRemove.GetIndex()] != nullptr && m_allActors[actorToRemove.GetIndex()]->GetActorUID() == actorToRemove)
	{
		m_allActors.erase(m_allActors.begin() + actorToRemove.GetIndex());
	}
}


