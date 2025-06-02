#pragma once
#include "Game/EnemyProjectile.hpp"

class BoxHazard : public EnemyProjectile
{
public:
	BoxHazard(GameState* gameState, EntityType entityType, Vec2 const& startPos, ProjectileConfig config);
	bool OverlapsPlayer(Player* player) override;
};
