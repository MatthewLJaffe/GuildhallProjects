#pragma once
#include "Game/Entity.hpp"

struct AttractProjectileConfig
{
	Texture* m_texture = nullptr;
	AABB2 m_spriteBounds = AABB2(Vec2::ZERO, Vec2::ONE * 16.f);
	AABB2 m_uvs = AABB2::ZERO_TO_ONE;
	float m_liveTime = 5.f;
	std::string m_animation = "";
	float m_collisionRadius = 8.f;
	Vec2 m_targetPos;
	float m_stillPercentage = .3f;
};

class AttractProjectile : public Entity 
{
public:
	AttractProjectile(GameState* gameState, EntityType entityType, Vec2 const& startPos, AttractProjectileConfig config);
	void Update(float deltaSeconds) override;
	void Render() override;
	AttractProjectileConfig m_config;
	Timer m_liveTimer;
	Vec2 m_startPos;
	bool m_animationPlayed = false;
};