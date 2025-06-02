#include "Game/Wisp.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Boss2.hpp"

Wisp::Wisp(GameState* gameState, EntityType entityType, Vec2 const& startPos, EntityConfig const& config)
	: Entity(gameState, entityType, startPos, config)
{
	m_flameParticleTimer = Timer(.1f, g_theApp->GetGameClock());
	m_baseParticleTimer = Timer(.75f, g_theApp->GetGameClock());
	m_flameParticleTimer.Start();
	m_baseParticleTimer.Start();
	m_targetPos = m_gameState->GetBoss2()->GetPosition();
	Vec2 dirTowardsTarget = (m_targetPos - GetPosition()).GetNormalized();
	m_circleAngle = dirTowardsTarget.GetOrientationDegrees();
}

void Wisp::Update(float deltaSeconds)
{

	if (!m_moveAroundCircle)
	{
		//accelerate towards boss
		Vec2 dirTowardsBoss = (m_targetPos - GetPosition()).GetNormalized();
		m_acceleration = dirTowardsBoss * m_accelerationLength;
		m_velocity += m_acceleration * deltaSeconds;
		m_velocity = dirTowardsBoss * m_velocity.GetLength();
		m_position += m_velocity * deltaSeconds;

		//move in circle
		m_circleAngle += deltaSeconds * 120.f;
		Vec2 circleDir = Vec2::MakeFromPolarDegrees(m_circleAngle);
		float circleSpeed = 64.f;
		m_position += circleDir * circleSpeed * deltaSeconds;

		if (GetDistance2D(GetPosition(), m_targetPos) <= 16.f)
		{
			m_circleAngle = (GetNearestPointOnRing2D(GetPosition(), m_targetPos, 16.f) - m_targetPos).GetOrientationDegrees() + 90.f;;
			m_moveAroundCircle = true;
		}
	}

	else if (m_moveAroundCircle)
	{
		m_circleAngle += deltaSeconds * 90.f;
		Vec2 circleDir = Vec2::MakeFromPolarDegrees(m_circleAngle);
		float circleSpeed = 16.f;

		m_position += circleDir * circleSpeed * deltaSeconds;
	}

	if (m_flameParticleTimer.HasPeriodElapsed())
	{
		m_flameParticleTimer.Start();
		float flameEmmiterCone = 45.f;
		float oscilationRate = 180.f;
		float directionRandomness = 10.f;
		float emitAngle = 90.f + SinDegrees(g_theApp->GetGameClock()->GetTotalSeconds() * oscilationRate) * flameEmmiterCone * .5f;
		emitAngle = emitAngle + g_randGen->RollRandomFloatInRange(-1.f, 1.f) * directionRandomness;
		Vec2 emitDir = Vec2::MakeFromPolarDegrees(emitAngle, 4.f);
		EntityConfig config = EntityConfig::GetEntityConfigByName("WispFlame");
		m_gameState->AddEntity(new Entity(m_gameState, m_entityType, m_position + emitDir, config));
	}

	Entity::Update(deltaSeconds);
}
