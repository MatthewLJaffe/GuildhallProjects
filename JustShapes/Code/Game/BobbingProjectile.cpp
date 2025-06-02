#include "Game/BobbingProjectile.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/GameStateLevel.hpp"

BobbingProjectile::BobbingProjectile(GameState* gameState, EntityType entityType, Vec2 const& startPos, EntityConfig const& config)
	: Entity(gameState, entityType, startPos, config)
{
	m_spawnInTimer = Timer(.5f, g_theApp->GetGameClock());
	m_despawnTimer = Timer(.5f, g_theApp->GetGameClock());
	m_startY = GetPosition().y;
}

void BobbingProjectile::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (!m_hideTimer.IsStopped() && !m_hideTimer.HasPeriodElapsed())
	{
		return;
	}
	else if (m_hideTimer.HasPeriodElapsed() || m_hideTimer.m_period == 0.f)
	{
		m_hideTimer.Stop();
		if (m_liveTimer.m_period > 0)
		{
			m_liveTimer.Start();
			m_spawnInTimer.Start();
		}
		if (m_becomeHazardTimer.m_period > 0)
		{
			m_becomeHazardTimer.Start();
		}
		if (m_config.m_useConfig)
		{
			PlayAnimation(m_config.m_startAnimation);
		}
		m_startTime = g_theApp->GetGameClock()->GetTotalSeconds();
	}

	if (!m_spawnInTimer.IsStopped())
	{
		if (m_spawnInTimer.HasPeriodElapsed())
		{
			m_spawnInTimer.Stop();
			m_bobAnimation = true;
		}
	}
	UpdateAnimation();

	if (m_bobAnimation)
	{
		GameStateLevel* gameStateLevel = dynamic_cast<GameStateLevel*>(m_gameState);
		float bobFrequency = 360.f / gameStateLevel->m_beatTime;
		float bobHeight = 4.f;
		m_position.y = m_startY + bobHeight * SinDegrees((g_theApp->GetGameClock()->GetTotalSeconds() - m_startTime) * bobFrequency);
	}

	if (!m_despawnTimer.IsStopped())
	{
		if (m_despawnTimer.HasPeriodElapsed())
		{
			m_isGarbage = true;
		}
		else
		{
			m_scale = Vec2::Lerp(Vec2::ONE, Vec2::ZERO, SmoothStart4(m_despawnTimer.GetElapsedFraction()));
		}
	}
}

void BobbingProjectile::Render()
{
	Entity::Render();
}

void BobbingProjectile::DestroyEntity()
{
	m_despawnTimer.Start();
}
