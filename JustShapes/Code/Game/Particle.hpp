#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/AABB2.hpp"

struct ParticleConfig
{
	Vec2 m_velocity = Vec2::ZERO;
	Texture* m_texture = nullptr;
	AABB2 m_spriteBounds = AABB2(Vec2::ZERO, Vec2::ONE*16.f);
	AABB2 m_uvs = AABB2::ZERO_TO_ONE;
	float m_liveTime = 1.f;
	std::string m_animation = "PlayerParticle";
};

class Particle : public Entity
{
public:
	Particle(GameState* gameState, EntityType entityType, Vec2 const& startPos, ParticleConfig config);
	void Update(float deltaSeconds) override;
	Timer m_liveTimer;
	ParticleConfig m_particleConfig;
};