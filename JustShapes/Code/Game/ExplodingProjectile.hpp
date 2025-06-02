#pragma once
#include "Game/EnemyProjectile.hpp"


class ExplodingProjectile : public EnemyProjectile
{
public:
	ExplodingProjectile(GameState* gameState, EntityType entityType, Vec2 const& startPos, ProjectileConfig config);
	void OnDestroy() override;
};