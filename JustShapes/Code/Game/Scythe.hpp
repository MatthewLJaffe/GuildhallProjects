#pragma once
#include "Game/Entity.hpp"

class Scythe : public Entity
{
public:
	Scythe(GameState* gameState, EntityType entityType, Vec2 const& startPos, EntityConfig const& config = EntityConfig());
	void Update(float deltaSeconds) override;
	void UpdateScytheMovement(float deltaSeconds);
	void DestroyScythe();
	Timer m_delayTimer;
	Timer m_destroyTimer;
	float m_maxAcc = 256.f;
	float m_maxSpeed = 128.f;
	float m_baseDestroyRotation = 0.f;
	bool m_goDown = false;
};