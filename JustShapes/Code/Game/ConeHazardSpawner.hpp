#pragma once
#include"Game/EnemyProjectile.hpp"

class ConeHazardSpawner : public EnemyProjectile
{
public:
	ConeHazardSpawner(GameState* gameState, EntityType entityType, Vec2 const& startPos, ProjectileConfig config, bool fireProjectiles = true);
	void Update(float deltaSeconds) override;
	void Render() override;
	void OnDestroy() override;
	float m_coneAngle = 10.f;
	float m_minConeAngle = 0.f;
	float m_maxConeAngle = 0.f;
	Timer m_coneExpandTimer;
	float m_coneLength = 600.f;
	bool m_fireProjectiles = true;

};