#include "Game/Scythe.hpp"
#include "Game/Player.hpp"

Scythe::Scythe(GameState* gameState, EntityType entityType, Vec2 const& startPos, EntityConfig const& config)
	: Entity(gameState, entityType, startPos, config)
{
	m_delayTimer = Timer(1.f, g_theApp->GetGameClock());
	m_destroyTimer = Timer(1.25f, g_theApp->GetGameClock());
	m_acceleration = (m_gameState->GetPlayer()->GetPosition() - m_position).GetNormalized() * m_maxAcc;
	if (m_config.m_hideTime <= 0.f)
	{
		m_delayTimer.Start();
	}
}

void Scythe::Update(float deltaSeconds)
{
	if (!m_hideTimer.IsStopped() && !m_hideTimer.HasPeriodElapsed())
	{
		return;
	}
	else if (m_hideTimer.HasPeriodElapsed())
	{
		m_hideTimer.Stop();
		m_delayTimer.Start();
		if (m_liveTimer.m_period > 0)
		{
			m_liveTimer.Start();
		}
		if (m_becomeHazardTimer.m_period > 0)
		{
			m_becomeHazardTimer.Start();
		}
		if (m_config.m_useConfig)
		{
			PlayAnimation(m_config.m_startAnimation);
		}
	}
	if (!m_becomeHazardTimer.IsStopped() && m_becomeHazardTimer.HasPeriodElapsed())
	{
		m_becomeHazardTimer.Stop();
		m_isHazard = true;
	}
	if (!m_liveTimer.IsStopped() && m_liveTimer.HasPeriodElapsed())
	{
		DestroyEntity();
		return;
	}

	UpdateAnimation();
	if (!m_delayTimer.IsStopped() && !m_delayTimer.HasPeriodElapsed())
	{
		return;
	}
	else if (m_delayTimer.HasPeriodElapsed())
	{
		m_delayTimer.Stop();
	}

	if (!m_destroyTimer.IsStopped() && !m_destroyTimer.HasPeriodElapsed())
	{
		float wiggleFraction = m_destroyTimer.GetElapsedFraction();
		float wiggleLeftFraction = .1f;
		float wiggleRightFraction = .2f;
		if (wiggleFraction < wiggleLeftFraction)
		{
			m_orientationDegrees = Lerp(m_baseDestroyRotation, m_baseDestroyRotation - 5.f, SmoothStep5(wiggleFraction / wiggleLeftFraction));
		}
		else if (wiggleFraction < wiggleRightFraction)
		{
			m_orientationDegrees = Lerp(m_baseDestroyRotation - 5.f, m_baseDestroyRotation + 3.f, RangeMap(wiggleFraction, wiggleLeftFraction, wiggleRightFraction, 0.f, 1.f));
		}
		return;
	}
	else if (m_destroyTimer.HasPeriodElapsed())
	{
		DestroyEntity();
		return;
	}

	UpdateScytheMovement(deltaSeconds);
}

void Scythe::UpdateScytheMovement(float deltaSeconds)
{
	if (m_goDown)
	{
		m_acceleration = Vec2::DOWN * 256.f;
	}

	else
	{
		AABB2 cameraBounds(g_theGame->m_worldCamera.GetOrthoBottomLeft(), g_theGame->m_worldCamera.GetOrthoTopRight());
		Vec2 nearestPointOnCameraPerimeter = cameraBounds.GetNearestPointOnPerimeter(GetPosition());
		if (IsPointInsideDisc2D(nearestPointOnCameraPerimeter, m_position, m_radius))
		{
			Vec2 edgeNormal = Vec2::RIGHT;
			if (nearestPointOnCameraPerimeter.x == cameraBounds.m_maxs.x)
			{
				edgeNormal = Vec2::LEFT;
			}
			else if (nearestPointOnCameraPerimeter.y == cameraBounds.m_mins.y)
			{
				edgeNormal = Vec2::UP;
			}
			else if (nearestPointOnCameraPerimeter.y == cameraBounds.m_maxs.y)
			{
				edgeNormal = Vec2::DOWN;
			}

			//roll new acceleration vector if we are still going off screen
			if (DotProduct2D(edgeNormal, m_acceleration.GetNormalized()) < .5f)
			{
				float accAngle = edgeNormal.GetOrientationDegrees() + g_randGen->RollRandomFloatInRange(-30.f, 30.f);
				m_acceleration = m_maxAcc * Vec2::MakeFromPolarDegrees(accAngle);
			}
		}
	}

	m_velocity += m_acceleration * deltaSeconds;
	if (m_velocity.GetLength() > m_maxSpeed)
	{
		m_velocity.SetLength(m_maxSpeed);
	}
	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees += m_rotationSpeed * deltaSeconds;
}

void Scythe::DestroyScythe()
{
	m_baseDestroyRotation = m_orientationDegrees;
	m_destroyTimer.Start();
	PlayAnimation("ScytheFadeOut");
}
