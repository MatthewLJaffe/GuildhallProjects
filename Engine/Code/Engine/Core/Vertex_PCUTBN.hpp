#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"


struct Vertex_PCUTBN
{
	Vertex_PCUTBN() = default;
	Vertex_PCUTBN(Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords,
		Vec3 const& tangent, Vec3 const& bitangent, Vec3 const& normal)
		: m_position(position)
		, m_color(color)
		, m_uvTexCoords(uvTexCoords)
		, m_tangent(tangent)
		, m_bitangent(bitangent)
		, m_normal(normal)
	{ }

	Vertex_PCUTBN(float px, float py, float pz,
		unsigned char r, unsigned char b, unsigned char g, unsigned char a,
		float u, float v,
		float tx, float ty, float tz,
		float bx, float by, float bz,
		float nx, float ny, float nz)
		: m_position(Vec3(px, py, pz))
		,  m_color(Rgba8(r, g, b, a))
		, m_uvTexCoords(Vec2(u, v))
		, m_tangent(Vec3(tx, ty, tz))
		, m_bitangent(Vec3(bx, by, bz))
		, m_normal(Vec3(nx, ny, nz))
	{ }

	Vertex_PCUTBN(Vertex_PCUTBN const& copy)
		: m_position(copy.m_position)
		, m_color(copy.m_color)
		, m_uvTexCoords(copy.m_uvTexCoords)
		, m_tangent(copy.m_tangent)
		, m_bitangent(copy.m_bitangent)
		, m_normal(copy.m_normal)
	{

	}

	Vec3 m_position;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;
	Vec3 m_tangent;
	Vec3 m_bitangent;
	Vec3 m_normal;
};