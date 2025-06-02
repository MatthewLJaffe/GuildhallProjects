#include "Engine/Core/Vertex_Particle.hpp"

Vertex_Particle::Vertex_Particle()
{
}

Vertex_Particle::Vertex_Particle(Vec3 const& position, Rgba8 const& color)
	: m_position(position)
	, m_color(color)
{
}

Vertex_Particle::Vertex_Particle(Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords, unsigned int index, Vec2 quadUVs)
	: m_position(position)
	, m_color(color)
	, m_uvTexCoords(uvTexCoords)
	, m_index(index)
	, m_quadUVs(quadUVs)
{
}
