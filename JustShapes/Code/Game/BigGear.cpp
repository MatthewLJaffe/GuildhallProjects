#include "Game/BigGear.hpp"
#include "Game/Player.hpp"


BigGear::BigGear(GameState* gameState, EntityType type, Vec2 const& startPos, EntityConfig config)
	: Entity(gameState, type, startPos, config)
{
}

bool BigGear::OverlapsPlayer(Player* player)
{
	float distFromPlayer = GetDistance2D(player->GetPosition(), GetPosition());
	if (distFromPlayer < m_innerRadius)
	{
		return true;
	}
	else if (distFromPlayer > m_innerRingRadius && distFromPlayer < m_outerRingRadius)
	{
		return true;
	}
	return false;
}

void BigGear::Update(float deltaSeconds)
{
	if (m_liveTimer.GetElapsedTime() < .3 || m_liveTimer.GetElapsedTime() > 2.903)
	{
		m_velocity = Vec2::ZERO;
		m_rotationSpeed = 0.f;
	}
	else
	{
		m_velocity = Vec2(-180.f, 0.f);
		m_rotationSpeed = 90.f;
	}
	if (m_liveTimer.GetElapsedTime() > 2.903 && !m_spawnedGoAwayParticle)
	{
		m_spawnedGoAwayParticle = true;
		EntityConfig circleParticleConfig = EntityConfig::GetEntityConfigByName("BigCircleParticle");
		m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, GetPosition(), circleParticleConfig));
	}
	Entity::Update(deltaSeconds);
}
