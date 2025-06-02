#include "Game/GuidedMissile.hpp"
#include "Game/Map.hpp"

GuidedMissile::GuidedMissile(Vec2 const& startPos, float startOrientation, EntityFaction factionType)
	: Entity(startPos, startOrientation)
{
	m_entityFaction = factionType;
	m_health = 1;
	m_maxSpeed = 2.f;
	m_texture = TEXTURE_MISSILE;
	m_entityType = ENTITY_TYPE_EVIL_MISSILE;
	m_physicsRadius = .1f;
	m_cosmeticRadius = .25f;
	m_velocity = GetForwardNormal() * m_maxSpeed;
	m_liveTime = 3.f;
	InitializeLocalVerts();
}

void GuidedMissile::Update(float deltaSeconds)
{
	if (m_target != nullptr)
	{
		Vec2 dirTowardsTarget = m_target->m_position - m_position;
		float targetOrientation = dirTowardsTarget.GetOrientationDegrees();
		m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, targetOrientation, deltaSeconds * m_turnDegreesPerSecond);
	}
	m_position += GetForwardNormal() * m_maxSpeed * deltaSeconds;
	m_liveTime -= deltaSeconds;
	if (m_liveTime <= 0)
	{
		Die();
	}
	if (m_map->IsPointInSolid(m_position, false))
	{
		Die();
	}
}

void GuidedMissile::InitializeLocalVerts()
{
	AddVertsForAABB2D(m_localVerts, Vec2(-.1f, -.075f), Vec2(.1f, .075f), Rgba8::WHITE);
}

void GuidedMissile::Render() const
{
	RenderVerts(m_localVerts, m_position, m_orientationDegrees, 1.f, m_texture);
}

void GuidedMissile::Die()
{
	g_theAudio->StartSound(SOUND_ID_BULLET_RICOCHET1);
	m_isDead = true;
	m_isGarbage = true;
}

