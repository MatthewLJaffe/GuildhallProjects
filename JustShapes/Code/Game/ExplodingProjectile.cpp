#include "Game/ExplodingProjectile.hpp"

ExplodingProjectile::ExplodingProjectile(GameState* gameState, EntityType entityType, Vec2 const& startPos, ProjectileConfig config)
	: EnemyProjectile(gameState, entityType, startPos, config)
{
}

void ExplodingProjectile::OnDestroy()
{
	int tinyProjectilesToSpawn = 5;

	float tinyProjectileAngleTheta = 360.f / (float)tinyProjectilesToSpawn;
	float startAngle = g_randGen->RollRandomFloatInRange(0.f, 360.f);

	for (int i = 0; i < 5; i++)
	{
		float angle = startAngle + (float)i * tinyProjectileAngleTheta;
		ProjectileConfig tinyProjectileConfig;
		tinyProjectileConfig.m_animation = "ProjectileSpawn";
		tinyProjectileConfig.m_collisionRadius = 2.f;
		tinyProjectileConfig.m_liveTime = 2.f;
		tinyProjectileConfig.m_rotationSpeed = 60.f;
		tinyProjectileConfig.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/SpikeyBall.png");
		tinyProjectileConfig.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(4.f, 4.f));
		tinyProjectileConfig.m_velocity = Vec2::MakeFromPolarDegrees(angle, 128.f);

		Vec2 startPos = m_position + Vec2::MakeFromPolarDegrees(angle, 4.f);
		m_gameState->AddEntity(new EnemyProjectile(m_gameState, EntityType::ENEMY_PROJECTILE, startPos, tinyProjectileConfig));
	}
}
