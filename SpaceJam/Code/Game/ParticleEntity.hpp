#pragma once
#include "Game/Entity.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"

class ParticleEntity : public Entity
{
public:
	ParticleEntity(const Vec3& startPos, EulerAngles const& orientation, ParticleEffect* effect, float lifeTime = -1.f);
	ParticleEntity(const Vec3& startPos, Mat44 const& orientation, ParticleEffect* effect, float lifeTime = -1.f);
	ParticleEffect* m_effect;
	void Update(float deltaSeconds) override;
	void Render() const override;
	float m_currentLifeTime = -1.f;
	~ParticleEntity();
};