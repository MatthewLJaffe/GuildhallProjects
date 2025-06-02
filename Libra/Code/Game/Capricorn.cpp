#include "Game/Capricorn.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Bullet.hpp"
#include "Game/GuidedMissile.hpp"

Capricorn::Capricorn(Vec2 const& startPos, float orientation)
	: Entity(startPos, orientation)
{
	m_canSwim = true;
	m_isPushedByWalls = true;
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isHitByBullets = true;
	m_orientationDegrees = 0.f;
	m_physicsRadius = g_gameConfigBlackboard.GetValue("tankPhysicsRadius", .25f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("tankCosmeticRadius", .75f);
	m_maxHealth = g_gameConfigBlackboard.GetValue("defaultEnemyHealth", 3.f);
	m_health = m_maxHealth;
	m_hasHealthBar = true;
	m_maxMoveSpeed = g_gameConfigBlackboard.GetValue("defaultEnemyMoveSpeed", .5f);
	InitializeLocalVerts();
	m_entityType = EntityType::ENTITY_TYPE_EVIL_CAPRICORN;
	m_entityFaction = EntityFaction::FACTION_EVIL;
	m_texture = TEXTURE_CAPRICORN;
}

void Capricorn::Update(float deltaSeconds)
{
	UpdateTargetPos();
	ComputeWayPointPosImproved();
	MoveTowardsWaypointPos(deltaSeconds);
	HandleShooting(deltaSeconds);
}

void Capricorn::Render() const
{
	RenderVerts(m_localVerts, m_position, m_orientationDegrees, 1.f, m_texture);
}

void Capricorn::RenderDebug() const
{
	Entity::RenderDebug();
	if (m_targetPos != Vec2::ZERO)
	{
		std::vector<Vertex_PCU> debugVerts;
		debugVerts.reserve(150);
		AddVertsForDisc2D(debugVerts, m_position, .05f, Rgba8::BLACK);
		AddVertsForLine2D(debugVerts, m_position, m_targetPos, .015f, Rgba8(0, 0, 255, 150));
		AddVertsForDisc2D(debugVerts, m_targetPos, .05f, Rgba8::BLACK);

		AddVertsForLine2D(debugVerts, m_position, m_nextWayPointPos, .015f, Rgba8(255, 255, 255, 150));
		AddVertsForDisc2D(debugVerts, m_nextWayPointPos, .05f, Rgba8::WHITE);

		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(debugVerts.size(), debugVerts.data());
	}
}

void Capricorn::Die()
{
	m_map->SpawnNewExplosion(m_position, .75f, .75);
	g_theAudio->StartSound(SOUND_ID_ENEMY_DIED);
	Entity::Die();
}

void Capricorn::InitializeLocalVerts()
{
	AddVertsForAABB2D(m_localVerts, Vec2(-.5f, -.5f), Vec2(.5f, .5f), Rgba8(255, 255, 255, 255));
}

void Capricorn::HandleShooting(float deltaSeconds)
{
	Player* player = m_map->GetNearestPlayerAlive();
	bool trackingPlayer = false;
	if (player)
	{
		trackingPlayer = m_map->HasLineOfSight(this, m_visionRadius, player);
	}
	if (trackingPlayer)
	{
		//shooting
		float goalOrientation = (player->m_position - m_position).GetOrientationDegrees();
		if (fabsf(m_orientationDegrees - goalOrientation) < 45.f)
		{
			m_currShootCooldown -= deltaSeconds;
			if (m_currShootCooldown <= 0.f)
			{
				m_currShootCooldown = m_shootCooldown;
				g_theAudio->StartSound(SOUND_ID_ENEMY_SHOOT);
				Entity* guidedMissile = g_theGame->m_currentMap->SpawnNewEntity(EntityType::ENTITY_TYPE_EVIL_MISSILE, m_position + GetForwardNormal() * .5f, m_orientationDegrees);
				dynamic_cast<GuidedMissile*>(guidedMissile)->m_target = player;
			}
		}
	}
}
