#include "Game/PhysicsShape3D.hpp"
#include "Engine/Core/Clock.hpp"

PhysicsShape3D::PhysicsShape3D(Vec3 const& position, ShapeType shapeType, bool wireFrame, Texture* texture)
	: m_position(position)
	, m_shapeType(shapeType)
	, m_wireFrame(wireFrame)
	, m_texture(texture)
{
	if (m_wireFrame)
	{
		m_color = c_wireframeDefaultColor;
	}
	else
	{
		m_color = Rgba8::WHITE;
	}
}

void PhysicsShape3D::Render()
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	m_modelMatrix.SetTranslation3D(m_position);
	g_theRenderer->BindTexture(m_texture);
	if (m_isOverlapping)
	{
		float normalizedAlpha = RangeMap( sinf(Clock::GetSystemClock().GetTotalSeconds()*5.f ), -1.f, 1.f, .2f, 1.f );
		m_color.a = DenormalizeByte(normalizedAlpha);
	}
	else
	{
		m_color.a = 255;
	}
	g_theRenderer->SetModelConstants(m_modelMatrix, m_color);
	if (m_wireFrame)
	{
		g_theRenderer->SetRasterizeMode(RasterizeMode::WIREFRAME_CULL_NONE);
	}
	else
	{
		g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	}
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->DrawVertexArray(m_verts.size(), m_verts.data());
	std::vector<Vertex_PCU> nearestPointVerts;
	AddVertsForSphere3D(nearestPointVerts, m_nearestPoint, .05f, Rgba8(255, 150, 0));
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(nearestPointVerts.size(), nearestPointVerts.data());
}

void PhysicsShape3D::SetNearestPoint(Vec3 const& refrencePos)
{
	m_nearestPoint = GetNearestPointOnShape(refrencePos);
}

Vec3 PhysicsShape3D::GetPosition() const
{
	return m_position;
}

void PhysicsShape3D::SetPosition(Vec3 const& newPosition)
{
	m_position = newPosition;
}
