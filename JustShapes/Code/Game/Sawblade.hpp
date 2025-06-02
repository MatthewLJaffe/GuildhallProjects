#pragma once
#include "Game/Entity.hpp"

class SawBlade : public Entity
{
public:
	SawBlade(GameState* gameState, EntityType entityType, Vec2 const& startPos, EntityConfig const& config, Vec2 direction);
	void Update(float deltaSeconds) override;
	Vec2 m_direction;
	Timer m_spawnInTimer;
	Timer m_shootBulletTimer;
};