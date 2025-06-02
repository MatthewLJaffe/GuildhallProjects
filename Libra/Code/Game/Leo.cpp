#include "Game/Leo.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"

Leo::Leo(Vec2 const& startPos, float orientation)
	: Entity(startPos, orientation)
{
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
	m_entityType = EntityType::ENTITY_TYPE_EVIL_LEO;
	m_entityFaction = EntityFaction::FACTION_EVIL;
	m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTank4.png");
}

void Leo::Update(float deltaSeconds)
{
	UpdateTargetPos();
	ComputeWayPointPosImproved();
	MoveTowardsWaypointPos(deltaSeconds);
	HandleShooting(deltaSeconds);
}

void Leo::Render() const
{
	RenderVerts(m_localVerts, m_position, m_orientationDegrees, 1.f, m_texture);
}

void Leo::RenderDebug() const
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

void Leo::TakeDamage(float damageAmount)
{
	m_health -= damageAmount;
	if (m_health <= 0)
	{
		Die();
	}
	else
	{
		g_theAudio->StartSound(SOUND_ID_ENEMY_HIT);
	}
}

void Leo::Die()
{
	m_map->SpawnNewExplosion(m_position, .75f, .75);
	g_theAudio->StartSound(SOUND_ID_ENEMY_DIED);
	Entity::Die();
}

void Leo::InitializeLocalVerts()
{
	AddVertsForAABB2D(m_localVerts, Vec2(-.5f, -.5f), Vec2(.5f, .5f), Rgba8(255, 255, 255, 255));
}

void Leo::HandleShooting(float deltaSeconds)
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
				g_theGame->m_currentMap->SpawnNewEntity(EntityType::ENTITY_TYPE_EVIL_BULLET, m_position + GetForwardNormal() * .5f, m_orientationDegrees);
			}
		}
	}
}


