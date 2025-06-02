#include "Engine/Core/Vertex_PCU.hpp"

Vertex_PCU::Vertex_PCU() {}

Vertex_PCU::Vertex_PCU(Vec3 const& position, Rgba8 const& color)

	: m_position(position)
	, m_color(Rgba8(color.r, color.g, color.b, color.a))
	, m_uvTexCoords(Vec2(0.0f, 0.0f))
{}

Vertex_PCU::Vertex_PCU(Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords)
	: m_position(position)
	, m_color(color)
	, m_uvTexCoords(uvTexCoords)
{}
