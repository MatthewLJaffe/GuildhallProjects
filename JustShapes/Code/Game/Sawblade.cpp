#include "Game/Sawblade.hpp"

SawBlade::SawBlade(GameState* gameState, EntityType entityType, Vec2 const& startPos, EntityConfig const& config, Vec2 direction)
	: Entity(gameState, entityType, startPos, config)
	, m_direction(direction)
{
	if (m_direction == Vec2::LEFT)
	{
		PlayAnimation("SawBladeSpawnLeft");
	}
	else if (m_direction == Vec2::RIGHT)
	{
		PlayAnimation("SawBladeSpawnRight");
	}
	else if (m_direction == Vec2::UP)
	{
		PlayAnimation("SawBladeSpawnUp");
	}
	else if (m_direction == Vec2::DOWN)
	{
		PlayAnimation("SawBladeSpawnDown");
	}
	m_shootBulletTimer = Timer(.02f, g_theApp->GetGameClock());
	m_spawnInTimer = Timer(1.45161f, g_theApp->GetGameClock());
	m_spawnInTimer.Start();
}

void SawBlade::Update(float deltaSeconds)
{
	if (!m_spawnInTimer.IsStopped() && m_spawnInTimer.HasPeriodElapsed())
	{
		m_spawnInTimer.Stop();
		m_velocity = m_direction.GetRotatedMinus90Degrees() * 128.f;
		if (fabsf(m_direction.y) < .1f)
		{
			m_velocity *= .5f;
		}
		m_rotationSpeed = 500.f;
		m_shootBulletTimer.Start();
	}

	if (!m_shootBulletTimer.IsStopped() && m_shootBulletTimer.HasPeriodElapsed())
	{
		EntityConfig bulletConfig = EntityConfig::GetEntityConfigByName("CircleBullet");
		bulletConfig.m_startAcceleration = m_direction * 600.f;
		float velocityDir = m_direction.GetOrientationDegrees() + g_randGen->RollRandomFloatInRange(90.f, 130.f);
		bulletConfig.m_startVelocity = Vec2::MakeFromPolarDegrees(velocityDir) * g_randGen->RollRandomFloatInRange(200.f, 400.f);
		m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, GetPosition(), bulletConfig));
		m_shootBulletTimer.Start();
	}

	Entity::Update(deltaSeconds);
}
