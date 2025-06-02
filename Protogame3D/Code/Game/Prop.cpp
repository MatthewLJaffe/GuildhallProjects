#include "Game/Prop.hpp"

Prop::Prop(Game* game, Vec3 const& startPos)
	: Entity(game, startPos)
{
}

void Prop::Update(float deltaSeconds)
{
	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees.m_yaw += m_angularVelocity.m_yaw * deltaSeconds; 
	m_orientationDegrees.m_pitch += m_angularVelocity.m_pitch * deltaSeconds;
	m_orientationDegrees.m_roll += m_angularVelocity.m_roll * deltaSeconds;
}

void Prop::Render() const
{
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray(m_vertexes.size(), m_vertexes.data());
}
