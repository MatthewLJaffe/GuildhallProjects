#include "AttractProjectile.hpp"

AttractProjectile::AttractProjectile(GameState* gameState, EntityType entityType, Vec2 const& startPos, AttractProjectileConfig config)
	: Entity(gameState, entityType, startPos)
	, m_config(config)
	, m_startPos(startPos)
{
	m_isHazard = true;
	m_liveTimer = Timer(m_config.m_liveTime, g_theApp->GetGameClock());
	m_liveTimer.Start();
	m_texture = m_config.m_texture;
	m_spriteBounds = m_config.m_spriteBounds;
	m_radius = m_config.m_collisionRadius;
	m_uvs = m_config.m_uvs;
}

void AttractProjectile::Update(float deltaSeconds)
{
	UpdateAnimation();
	UNUSED(deltaSeconds);
	if (m_liveTimer.HasPeriodElapsed())
	{
		DestroyEntity();
	}
	if (m_liveTimer.GetElapsedFraction() > m_config.m_stillPercentage)
	{
		if (!m_animationPlayed)
		{
			PlayAnimation(m_config.m_animation);
			m_animationPlayed = true;
		}
		float t = GetFractionWithinRange(m_liveTimer.GetElapsedFraction(), m_config.m_stillPercentage, 1.f);
		m_position = Vec2::Lerp(m_startPos, m_config.m_targetPos, SmoothStep5(t));
	}
}

void AttractProjectile::Render()
{
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	std::vector<Vertex_PCU> lineVerts;
	AddVertsForLine2D(lineVerts, m_startPos, m_config.m_targetPos, .5f, Rgba8(236, 1, 106, 100));
	g_theRenderer->DrawVertexArray(lineVerts.size(), lineVerts.data());
	Entity::Render();
}
