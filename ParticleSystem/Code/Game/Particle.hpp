#pragma once

#include "Game/GameCommon.hpp"
class ParticleEmitter;


class Particle
{
public:
	Particle(ParticleEmitter const* emitter);
	Vec3 m_position;
	float m_liveTime = 0.f;
	Vec3 m_velocity;
	float distance;
	Vec4 m_color;
	IntVec2 m_spriteSheetSize;
	int m_spriteStartIndex;
	int m_spriteEndIndex;
	Vec2 m_uvBottomLeft;
	Vec2 m_uvTopRight;
	void Update(float deltaSeconds);
	void AddVertsForParticle(std::vector<Vertex_PCU>& verts);
	void operator=(Particle const& particle);
};