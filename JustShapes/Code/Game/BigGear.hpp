#pragma once
#include "Game/Entity.hpp"

class BigGear : public Entity
{
public:
	BigGear(GameState* gameState, EntityType type, Vec2 const& startPos, EntityConfig config);
	bool OverlapsPlayer(Player* player) override;
	void Update(float deltaSeconds) override;
private:
	float m_innerRadius = 16.f;
	float m_innerRingRadius = 90.f;
	float m_outerRingRadius = 115.f;
	bool m_spawnedGoAwayParticle = false;
};