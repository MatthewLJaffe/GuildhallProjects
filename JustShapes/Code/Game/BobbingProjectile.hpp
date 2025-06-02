#pragma once
#include "Game/Entity.hpp"

class BobbingProjectile : public Entity
{
public:
	BobbingProjectile(GameState* gameState, EntityType entityType, Vec2 const& startPos, EntityConfig const& config = EntityConfig());
	void Update(float deltaSeconds) override;
	void Render() override;
	void DestroyEntity() override;
	Timer m_spawnInTimer;
	Timer m_despawnTimer;
	bool m_bobAnimation = false;
	float m_startY = 0.f;
	float m_startTime = 0.f;
};