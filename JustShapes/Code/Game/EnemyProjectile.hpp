#pragma once
#include "Game/Entity.hpp"

class Player;

struct ProjectileConfig
{
	Vec2 m_velocity = Vec2::ZERO;
	Texture* m_texture = nullptr;
	AABB2 m_spriteBounds = AABB2(Vec2::ZERO, Vec2::ONE * 16.f);
	AABB2 m_uvs = AABB2::ZERO_TO_ONE;
	Vec2 m_normalizedPivot = Vec2(.5f, .5f);
	float m_liveTime = 5.f;
	float m_becomeHazardTime = 0.f;
	std::string m_animation = "";
	float m_collisionRadius = 16.f;
	float m_rotationSpeed = 0.f;
	float m_startOrientaiton = 0.f;
	float m_maxConeAngle = 30.f;
	float m_hideTime = 0.f;
	Vec2 m_acceleration = Vec2::ZERO;
};

class EnemyProjectile : public Entity 
{
public:
	EnemyProjectile(GameState* gameState, EntityType entityType, Vec2 const& startPos, ProjectileConfig config);
	void Update(float deltaSeconds) override;
	void Render() override;
	virtual void OnDestroy();
	ProjectileConfig m_config;
	Timer m_liveTimer;
	Timer m_hideTimer;
};