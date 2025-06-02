#include "Game/Aries.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Bullet.hpp"

Aries::Aries(Vec2 const& startPos, float orientation)
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
	m_entityType = EntityType::ENTITY_TYPE_EVIL_ARIES;
	m_entityFaction = EntityFaction::FACTION_EVIL;
	m_texture = TEXTURE_ARIES;
}

void Aries::Update(float deltaSeconds)
{
	UpdateTargetPos();
	ComputeWayPointPosImproved();
	MoveTowardsWaypointPos(deltaSeconds);
}

void Aries::Render() const
{
	RenderVerts(m_localVerts, m_position, m_orientationDegrees, 1.f, m_texture);
}

void Aries::HandleIncomingBullet(Bullet* bullet)
{
	Vec2 airesToBulletDisp = bullet->m_position - m_position;
	if (fabsf(GetAngleDegreesBetweenVectors2D(airesToBulletDisp, GetForwardNormal())) > 45.f)
	{
		TakeDamage(bullet->m_bulletDamage);
		bullet->Die();
	}
	else
	{
		bullet->ReflectBullet(airesToBulletDisp.GetNormalized(), true);
		PushDiscOutOfFixedDisc2D(bullet->m_position, bullet->m_physicsRadius, m_position, m_physicsRadius);
		bullet->TakeDamage(1.f);
	}
}

void Aries::RenderDebug() const
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

void Aries::TakeDamage(float damageAmount)
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

void Aries::Die()
{
	m_map->SpawnNewExplosion(m_position, .75f, .75);
	g_theAudio->StartSound(SOUND_ID_ENEMY_DIED);
	Entity::Die();
}

void Aries::InitializeLocalVerts()
{
	AddVertsForAABB2D(m_localVerts, Vec2(-.5f, -.5f), Vec2(.5f, .5f), Rgba8(255, 255, 255, 255));
}
