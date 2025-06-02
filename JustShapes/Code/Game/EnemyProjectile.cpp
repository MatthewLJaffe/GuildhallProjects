#include "Game/EnemyProjectile.hpp"
#include "Game/Player.hpp"

EnemyProjectile::EnemyProjectile(GameState* gameState, EntityType entityType, Vec2 const& startPos, ProjectileConfig config)
	: Entity(gameState, entityType, startPos)
{
	m_config = config;
	m_radius = config.m_collisionRadius;
	m_spriteBounds = m_config.m_spriteBounds;
	m_texture = m_config.m_texture;
	m_uvs = m_config.m_uvs;
	m_velocity = m_config.m_velocity;
	m_simulatePhysics = true;
	m_color = Rgba8(255, 50, 100);
	m_orientationDegrees = m_config.m_startOrientaiton;
	m_liveTimer = Timer(m_config.m_liveTime, g_theApp->GetGameClock());
	m_hideTimer = Timer(m_config.m_hideTime, g_theApp->GetGameClock());
	m_acceleration = m_config.m_acceleration;
	if (m_hideTimer.m_period > 0.01f)
	{
		m_hideTimer.Start();
		m_liveTimer.Stop();
	}
	else
	{
		PlayAnimation(m_config.m_animation);
		m_liveTimer.Start();
	}
}

void EnemyProjectile::Update(float deltaSeconds)
{
	if (m_liveTimer.IsStopped())
	{
		if (m_hideTimer.HasPeriodElapsed())
		{
			PlayAnimation(m_config.m_animation);
			m_liveTimer.Start();
		}
		else
		{
			return;
		}
	}

	if (m_liveTimer.GetElapsedTime() > m_config.m_becomeHazardTime)
	{
		m_isHazard = true;
	}

	m_orientationDegrees += m_config.m_rotationSpeed * deltaSeconds;
	if (m_liveTimer.HasPeriodElapsed())
	{
		OnDestroy();
		DestroyEntity();
		return;
	}

	UpdateAnimation();
	m_velocity += m_acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
}

void EnemyProjectile::Render()
{
	if (m_liveTimer.IsStopped())
	{
		return;
	}
	Entity::Render();
}

void EnemyProjectile::OnDestroy()
{
}
