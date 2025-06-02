#pragma once
#include "Game/Entity.hpp"

class Wisp : public Entity
{
public:
	Wisp(GameState* gameState, EntityType entityType, Vec2 const& startPos, EntityConfig const& config = EntityConfig());
	void Update(float deltaSeconds) override;
	Timer m_flameParticleTimer;
	Timer m_baseParticleTimer;
	float m_circleAngle = 0.f;
	bool m_moveAroundCircle = false;
	Vec2 m_targetPos;
	float m_accelerationLength = 6.f;
};