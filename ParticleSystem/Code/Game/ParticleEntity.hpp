#pragma once
#include "Game/Entity.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"

class ParticleEntity : public Entity
{
public:
	ParticleEntity(Game* game, const Vec3& startPos, EulerAngles const& orientation, ParticleEffect* effect, ParticlePhysicsObject* physObject = nullptr, float lifeTime = -1.f);
	ParticleEffect* m_effect;
	ParticlePhysicsObject* m_physicsObject = nullptr;
	void Update(float deltaSeconds) override;
	void Render() const override;
	float m_currentLifeTime = -1.f;
	~ParticleEntity();
};