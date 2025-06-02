#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

struct Vertex_MeshParticle
{
public:
	Vec3 m_position = Vec3::ZERO;
	Rgba8 m_color = Rgba8::WHITE;
	Vec2 m_uvTexCoords;
	unsigned int m_index = 0;
	Vec2 m_quadUVs;
	float m_emissive = 0.f;
	float m_alphaObscurance = 1.f;
	float m_panTextureContribution = 1.f;
	float m_particleLifetime = 0.f;
	Vec3 m_normal;

	Vertex_MeshParticle();
};