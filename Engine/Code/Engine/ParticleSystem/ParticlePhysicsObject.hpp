#pragma once
#include "Engine/Math/MathUtils.hpp"

struct ParticlePhysicsObjectGPU
{
	Vec3 m_position;
	float m_forceMagnitude = 0.f;

	int m_attract = 0;
	float m_radius = 0.f;
	float m_falloffExponent = 1.5f;
	int m_isActive = 0;
};


struct ParticlePhysicsAABB3GPU
{
	Vec3 m_mins;
	int m_isActive = 0;
	Vec3 m_maxs;
	float padding;
};

class ParticlePhysicsObject
{
	friend class ParticleSystem;
public:
	~ParticlePhysicsObject();
	void SetPosition(Vec3 const& position);
	void SetForceMagnitude(float forceMagnitude);
	void SetRadius(float radius);
	void SetFalloff(float falloff);

private:
	ParticlePhysicsObject(int physicsObjectGPUIndex);
	int m_physicsObjectGPUIndex = -1;
};

class ParticlePhysicsAABB3
{
	friend class ParticleSystem;
public:
	~ParticlePhysicsAABB3();
	void SetCenterPosition(Vec3 const& position);
	void SetScale(Vec3 const& scale);
	void SetMins(Vec3 const& mins);
	void SetMaxs(Vec3 const& maxs);

private:
	ParticlePhysicsAABB3(ParticlePhysicsAABB3GPU& phsicsAABB3GPU);
	ParticlePhysicsAABB3GPU& m_physicsAABB3GPU;
};