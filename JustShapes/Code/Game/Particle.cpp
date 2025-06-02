#include "Game/Particle.hpp"

Particle::Particle(GameState* gameState, EntityType entityType, Vec2 const& startPos, ParticleConfig config)
	: Entity(gameState, entityType, startPos)
{
	m_particleConfig = config;
	m_velocity = config.m_velocity;
	m_uvs = config.m_uvs;
	m_spriteBounds = config.m_spriteBounds;
	m_liveTimer = Timer(m_particleConfig.m_liveTime, g_theApp->GetGameClock());
	m_texture = config.m_texture;
	m_liveTimer.Start();
	PlayAnimation(config.m_animation);
}

void Particle::Update(float deltaSeconds)
{
	UpdateAnimation();
	m_position += m_velocity * deltaSeconds;
	if (m_liveTimer.HasPeriodElapsed())
	{
		DestroyEntity();
		return;
	}
}
