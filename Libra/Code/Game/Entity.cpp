#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Game/Bullet.hpp"
#include "Game/Map.hpp"
#include "Game/GuidedMissile.hpp"

Entity::Entity(Vec2 const& startPos)
	: m_position(startPos)
{}

Entity::Entity(Vec2 const& startPos, float orientationDegrees)
	: m_position(startPos)
	, m_orientationDegrees(orientationDegrees)
{

}

Entity::~Entity()
{
}

void Entity::Render() const
{
	RenderVerts(m_localVerts, m_position, m_orientationDegrees, 1.f, nullptr);
}

void Entity::RenderHealthBar() const
{
	if (m_hasHealthBar == false) return;

	Vec2 healthBarMin = m_position + Vec2(-.4f, .5f);
	Vec2 healthBarMax = m_position + Vec2(.4f, .575f);
	float healthPrecentage = m_health / m_maxHealth;
	float greenMaxX = Lerp(healthBarMin.x, healthBarMax.x, healthPrecentage);
	std::vector<Vertex_PCU> healthBarVerts;
	AddVertsForAABB2D(healthBarVerts, AABB2(healthBarMin, healthBarMax), Rgba8::RED);
	AddVertsForAABB2D(healthBarVerts, AABB2(healthBarMin, Vec2(greenMaxX, healthBarMax.y)), Rgba8::GREEN);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(healthBarVerts.size(), healthBarVerts.data());
}

void Entity::RenderDebug() const
{
	std::vector<Vertex_PCU> debugVerts;

	//radiuses
	AddVertsForRing2D(debugVerts, m_position, m_physicsRadius, .025f, Rgba8(0, 255, 255, 255));
	AddVertsForRing2D(debugVerts, m_position, m_cosmeticRadius, .025f, Rgba8(255, 0, 255, 255));

	Vec2 forwardDir = GetForwardNormal() * m_cosmeticRadius;
	//orientation
	AddVertsForLine2D(debugVerts, m_position, m_position + forwardDir, .025f, Rgba8(255, 0, 0, 255));
	AddVertsForLine2D(debugVerts, m_position, m_position + forwardDir.GetRotated90Degrees(), .025f, Rgba8(0, 255, 0, 255));

	//velocity
	AddVertsForLine2D(debugVerts, m_position, m_position + m_velocity, .015f, Rgba8(255, 255, 0, 255));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(debugVerts.size(), debugVerts.data());
}

void Entity::Die()
{
	m_isGarbage = true;
	m_isDead = true;
}

void Entity::HandleIncomingBullet(Bullet* bullet)
{
	TakeDamage(bullet->m_bulletDamage);
	bullet->Die();
}

void Entity::HandleIncomingMissile(GuidedMissile* missile)
{
	TakeDamage(missile->m_missileDamage);
	missile->Die();
}

void Entity::TakeDamage(float damageAmount)
{
	m_health -= damageAmount;
	if (m_health <= 0)
	{
		Die();
	}
}

bool Entity::IsAlive() const
{
	return !m_isGarbage && !m_isDead;
}

void Entity::MoveTankTowardsPoint(Vec2 const& point, float deltaSeconds)
{
	float targetOrientation = (point - m_position).GetOrientationDegrees();
	m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, targetOrientation, deltaSeconds * m_maxRotateSpeed);
	m_position += GetForwardNormal() * m_maxMoveSpeed * deltaSeconds;
}

void Entity::ComputeWayPointPos()
{
	if (m_heatMapOfTargetPos == nullptr)
	{
		m_heatMapOfTargetPos = new TileHeatMap(m_map->m_dimensions);
	}
	IntVec2 startCoords((int)m_targetPos.x, (int)m_targetPos.y);
	m_map->PopulateDistanceField(*m_heatMapOfTargetPos, startCoords, SPECIAL_TILE_HEAT, !m_canSwim);

	//handle case where we are in the best tile already
	IntVec2 currCoords((int)m_position.x, (int)m_position.y);
	if (m_heatMapOfTargetPos->GetHeatValue(currCoords) == 0.f)
	{
		m_nextWayPointPos = m_targetPos;
		return;
	}
	//find cheapest coordinate to travel to
	IntVec2 bestCoords = currCoords + IntVec2::NORTH;
	float bestCost = SPECIAL_TILE_HEAT;
	float currCost = bestCost;
	if (m_map->IsInteriorPos(bestCoords))
	{
		bestCost = m_heatMapOfTargetPos->GetHeatValue(currCoords + IntVec2::NORTH);
	}

	if (m_map->IsInteriorPos(currCoords + IntVec2::EAST))
	{
		currCost = m_heatMapOfTargetPos->GetHeatValue(currCoords + IntVec2::EAST);
		if (currCost < bestCost)
		{
			bestCost = currCost;
			bestCoords = currCoords + IntVec2::EAST;
		}
	}

	if (m_map->IsInteriorPos(currCoords + IntVec2::SOUTH))
	{
		currCost = m_heatMapOfTargetPos->GetHeatValue(currCoords + IntVec2::SOUTH);
		if (currCost < bestCost)
		{
			bestCost = currCost;
			bestCoords = currCoords + IntVec2::SOUTH;
		}
	}

	if (m_map->IsInteriorPos(currCoords + IntVec2::WEST))
	{
		currCost = m_heatMapOfTargetPos->GetHeatValue(currCoords + IntVec2::WEST);
		if (currCost < bestCost)
		{
			bestCost = currCost;
			bestCoords = currCoords + IntVec2::WEST;
		}
	}

	//no valid moves
	if (bestCost == SPECIAL_TILE_HEAT)
	{
		return;
	}
	m_nextWayPointPos = Vec2((float)bestCoords.x + .5f, (float)bestCoords.y + .5f);
}

void Entity::RenderVerts(std::vector<Vertex_PCU> const& verts, Vec2 const& position, float orientationDegrees, float scaleXY, Texture* tex) const
{
	//local copy
	std::vector<Vertex_PCU> tempVerts(verts);
	//compute i, j basis
	Vec2 iBasis = Vec2(CosDegrees(orientationDegrees), SinDegrees(orientationDegrees)) * scaleXY;
	Vec2 jBasis = iBasis.GetRotated90Degrees();
	//transform copy verts
	for (int i = 0; i < static_cast<int>(tempVerts.size()); i++)
	{
		TransformPositionXY3D(tempVerts[i].m_position, iBasis, jBasis, position);
	}
	//render
	g_theRenderer->BindTexture(tex);
	g_theRenderer->DrawVertexArray(static_cast<int>(tempVerts.size()), tempVerts.data());
}

void Entity::Wander(float deltaSeconds)
{
	m_currentDirectionTime += deltaSeconds;
	if (m_currentDirectionTime > m_changeDirectionTime)
	{
		m_currentDirectionTime = 0.f;
		m_goalOrientation = g_randGen->RollRandomFloatInRange(0.f, 360.f);
	}

	m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, m_goalOrientation, m_maxRotateSpeed * deltaSeconds);
	m_velocity = GetForwardNormal() * m_maxMoveSpeed;
	m_position += m_velocity * deltaSeconds;
}

void Entity::UpdateTargetPos()
{
	if (m_heatMapOfTargetPos == nullptr)
	{
		m_heatMapOfTargetPos = new TileHeatMap(m_map->m_dimensions);
	}
	Player* player = m_map->GetNearestPlayerAlive();

	if (player && m_map->HasLineOfSight(this, m_visionRadius, player))
	{
		if (!m_inPursuitOfPlayer)
		{
			m_map->PlayDiscoverySound();
			m_inPursuitOfPlayer = true;
		}
		IntVec2 currTargetCoords = IntVec2((int)m_targetPos.x, (int)m_targetPos.y);
		IntVec2 playerCoords = IntVec2((int)player->m_position.x, (int)player->m_position.y);
		if (currTargetCoords != playerCoords)
		{
			m_pathPoints.clear();
			m_map->PopulateDistanceField(*m_heatMapOfTargetPos, IntVec2((int)playerCoords.x, (int)playerCoords.y), SPECIAL_TILE_HEAT, !m_canSwim);
			m_map->GenerateEntityPathToGoal(m_heatMapOfTargetPos, m_pathPoints, playerCoords, IntVec2((int)m_position.x, (int)m_position.y));
		}
		m_targetPos = player->m_position;
	}
	else
	{
		if (m_pathPoints.size() == 0)
		{
			m_targetPos = m_map->GetRandomReachableTargetPos(IntVec2((int)m_position.x, (int)m_position.y), m_heatMapOfTargetPos, m_canSwim);
			m_pathPoints.clear();
			m_map->GenerateEntityPathToGoal(m_heatMapOfTargetPos, m_pathPoints, IntVec2((int)m_targetPos.x, (int)m_targetPos.y), IntVec2((int)m_position.x, (int)m_position.y));
			m_inPursuitOfPlayer = false;
		}
	}
}

void Entity::ComputeWayPointPosImproved()
{
	if (m_pathPoints.size() >= 2)
	{
		Vec2 raycastDir = (m_pathPoints[m_pathPoints.size() - 2] - m_position);
		RaycastResult2D raycastResult;
		if (m_canSwim)
		{
			raycastResult = m_map->m_solidMapForAmphibians.ImprovedRaycastVsSpecialHeat(m_position, raycastDir.GetNormalized(), raycastDir.GetLength(), SPECIAL_TILE_HEAT);
		}
		else
		{
			raycastResult = m_map->m_solidMapForLandbased.ImprovedRaycastVsSpecialHeat(m_position, raycastDir.GetNormalized(), raycastDir.GetLength(), SPECIAL_TILE_HEAT);
		}
		if (!raycastResult.m_didImpact)
		{
			m_pathPoints.pop_back();
		}
	}
	//use size check here to handle case where we try to get the .back() element of an empty vector and crash
	if (m_pathPoints.size() != 0)
	{
		if (IsPointInsideDisc2D(m_pathPoints.back(), m_position, m_physicsRadius))
		{
			m_pathPoints.pop_back();
		}
	}
	if (m_pathPoints.size() == 0)
	{
		m_nextWayPointPos = m_targetPos;
	}
	else
	{
		m_nextWayPointPos = m_pathPoints.back();
	}
}

void Entity::MoveTowardsWaypointPos(float deltaSeconds)
{
	MoveTankTowardsPoint(m_nextWayPointPos, deltaSeconds);
}

Vec2 Entity::GetForwardNormal() const
{
	return Vec2::MakeFromPolarDegrees(m_orientationDegrees);
}
