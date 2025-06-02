#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
class ParticleEmitter;


class Particle
{
public:
	Particle() = default;

	Vec3 m_position;
	float m_liveTime = 0.f;

	Vec3 m_velocity;
	unsigned int m_configIndex = 0;

	Vec2 m_uvBottomLeft;
	Vec2 m_uvTopRight;

	Vec3 m_emissionPosition;
	float m_currentDelayReturnTime;

	Vec3 m_lifetimeVelocity;
	unsigned int m_idInEmitter = 0;

	Vec3 m_reflectionVector;
	float padding;
};