#include "Game/LaserBeam.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/Time.hpp"

LaserBeam::LaserBeam(Game* game)
	: m_game(game)
{
	m_color = Rgba8(12, 198, 240, 10);
}

void LaserBeam::Update(float deltaSeconds)
{
	//updating laser position
	PlayerShip* player = m_game->m_playerShip;
	Vec2 playerForward = player->GetForwardNormal();
	m_startPos = player->m_position + playerForward * m_thickness * 2.f;
	m_endPos = m_startPos + playerForward * m_length;

	//check for collision with entities
	m_game->ApplyLaserDamage(this, deltaSeconds);

	//decrement ammo
	m_ammo -= m_ammoPerSecond * deltaSeconds;
	if (m_ammo <= 0)
	{
		m_ammo = 0.f;
	}
}

void LaserBeam::Render() const
{
	DrawLaserBeam(m_startPos, m_endPos, m_thickness, m_color);
}

bool LaserBeam::IsInLaser(Entity* entity)
{
	Vec2 laserForwardNormal = (m_endPos - m_startPos).GetNormalized();
	Vec2 laserRightNormal = laserForwardNormal.GetRotatedMinus90Degrees();
	Vec2 dispFromLaserToEntity = entity->m_position - m_startPos;
	float iPos = DotProduct2D(dispFromLaserToEntity, laserRightNormal);
	float jPos = DotProduct2D(dispFromLaserToEntity, laserForwardNormal);
	bool overlappingHoriz = abs(iPos) - entity->m_physicsRadius < m_thickness;
	bool overlappingVert =  jPos + entity->m_physicsRadius > 0.f && jPos - entity->m_physicsRadius < m_length;
	return overlappingHoriz && overlappingVert;
}

void LaserBeam::DrawLaserBeam(Vec2 startPos, Vec2 endPos, float thickness, Rgba8 color) const
{
	constexpr int NUM_PULSE_BEAMS = 40;
	float t = static_cast<float>(GetCurrentTimeSeconds());
	float beamThickness = .75f * thickness;
	float pulseSpeed = 10.f;
	for (int i = 0; i < NUM_PULSE_BEAMS; i++)
	{
		float timeOffset = static_cast<float>(i);
		float maxBeams = static_cast<float>(NUM_PULSE_BEAMS);
		timeOffset = RangeMap(timeOffset, 0.f, maxBeams, 0.f, 2.f * PI);
		float horizOffset = sinf(t * pulseSpeed + timeOffset);
		horizOffset *= (thickness - beamThickness);
		Vec2 horizAxis = (endPos - startPos).GetNormalized().GetRotatedMinus90Degrees();
		Vec2 posOffset = horizOffset * horizAxis;
		DebugDrawLine(startPos + posOffset, endPos + posOffset, beamThickness, color);
	}
	constexpr int NUM_RANDOM_BEAMS = 10;
	for (int i = 0; i < NUM_RANDOM_BEAMS; i++)
	{
		float horizOffset = g_randGen->RollRandomFloatInRange(-1.f, 1.f);
		horizOffset *= (thickness - beamThickness);
		Vec2 horizAxis = (endPos - startPos).GetNormalized().GetRotatedMinus90Degrees();
		Vec2 posOffset = horizOffset * horizAxis;
		DebugDrawLine(startPos + posOffset, endPos + posOffset, beamThickness, color);
	}
}
