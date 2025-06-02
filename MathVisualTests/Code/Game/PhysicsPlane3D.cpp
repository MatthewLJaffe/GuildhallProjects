#include "Game/PhysicsPlane3D.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Game/PhysicsSphere3D.hpp"
#include "Game/PhysicsCylinder3D.hpp"
#include "Game/PhysicsAABB3D.hpp"
#include "Game/PhysicsOBB3D.hpp"
#include "Engine/Core/Clock.hpp"

PhysicsPlane3D::PhysicsPlane3D(Plane3 const& plane, ShapeType shapeType, bool wireFrame, Texture* texture)
	: PhysicsShape3D(plane.m_distFromOriginAlongNormal * plane.m_normal, shapeType, wireFrame, texture)
	, m_plane(plane)
{
	
}

RaycastResult3D PhysicsPlane3D::RaycastVsShape(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist)
{
	return RaycastVsPlanes3D(startPos, fwdNormal, maxDist, m_plane);
}

Vec3 PhysicsPlane3D::GetNearestPointOnShape(Vec3 const& pointToQuery)
{
	float altitude = m_plane.GetAltitude(pointToQuery);
	return pointToQuery - altitude * m_plane.m_normal;
}

//#ToDo implement plane overlap check
void PhysicsPlane3D::CheckForOverlapWithOtherShapes(std::vector<std::vector<PhysicsShape3D*>> const& listOfShapesByType)
{
	for (size_t i = 0; i < listOfShapesByType[(int)ShapeType::OBB3].size(); i++)
	{
		PhysicsOBB3D* currOBB = dynamic_cast<PhysicsOBB3D*>(listOfShapesByType[(int)ShapeType::OBB3][i]);
		if (DoPlaneAndOBB3Overlap3D(m_plane, currOBB->m_obb3))
		{
			m_isOverlapping = true;
			return;
		}
	}
	m_isOverlapping = false;
}

//#ToDo implement sphere overlap check
bool PhysicsPlane3D::IsOverlappingSphere(Vec3 const& otherSpherePos, float otherSphereRadius)
{
	UNUSED(otherSpherePos);
	UNUSED(otherSphereRadius);
	return false;
}

//#ToDo implement cylinder overlap check
bool PhysicsPlane3D::IsOverlappingCylinder(Vec2 const& otherCylinderCenterXY, float otherCylinderRadius, FloatRange const& cylinderMinMaxZ)
{
	UNUSED(cylinderMinMaxZ);
	UNUSED(otherCylinderCenterXY);
	UNUSED(otherCylinderRadius);
	return false;
}

//#ToDo implement ABB3 overlap check
bool PhysicsPlane3D::IsOverlappingAABB3(AABB3 const& box)
{
	UNUSED(box);
	return false;
}

void PhysicsPlane3D::SetPosition(Vec3 const& newPosition)
{
	Vec3 displacment = newPosition - m_position;
	float moveDist = DotProduct3D(displacment, m_plane.m_normal);
	m_plane.m_distFromOriginAlongNormal += moveDist;
	m_position = m_plane.m_distFromOriginAlongNormal * m_plane.m_normal;
}

void PhysicsPlane3D::Render()
{
	std::vector<Vertex_PCU> planeVerts;

	AddVertsForSphere3D(planeVerts, Vec3::ZERO, .1f, Rgba8(150, 150, 150));
	AddVertsForArrow3D(planeVerts, Vec3::ZERO, m_plane.m_normal, .05f);
	Vec3 planeTangent;
	Vec3 planeBitangent;
	if (fabsf(DotProduct3D(m_plane.m_normal, Vec3(0.f, 0.f, 1.f))) != 1.f)
	{
		planeTangent = CrossProduct3D(m_plane.m_normal, Vec3(0.f, 0.f, 1.f));
	}
	else
	{
		planeTangent = CrossProduct3D(m_plane.m_normal, Vec3(0.f, 1.f, 0.f));
	}
	planeTangent = planeTangent.GetNormalized();
	planeBitangent = CrossProduct3D(m_plane.m_normal, planeTangent);
	planeBitangent = planeBitangent.GetNormalized();

	float gridSpacing = 1.f;
	int numGridLines = 11;

	float gridSize = (float)(numGridLines - 1) * gridSpacing;

	Rgba8 transparentWireFrameColor = Rgba8(c_wireframeDefaultColor.r, c_wireframeDefaultColor.g, c_wireframeDefaultColor.b, 0);
	Vec3 rowStartPos = -planeBitangent * gridSize;
	Vec3 rowEndPos = planeBitangent * gridSize;
	Vec3 rowOffset = -planeTangent * gridSize * .5f;

	for (int i = 0; i < numGridLines; i++)
	{
		AddVertsForLine3D(planeVerts, rowOffset, rowEndPos + rowOffset, .05f, c_wireframeDefaultColor, transparentWireFrameColor);
		AddVertsForLine3D(planeVerts, rowOffset, rowStartPos + rowOffset, .05f, c_wireframeDefaultColor, transparentWireFrameColor);
		rowOffset += planeTangent * gridSpacing;
	}

	Vec3 collStartPos = -planeTangent * gridSize;
	Vec3 collEndPos = planeTangent * gridSize;
	Vec3 collOffset = -planeBitangent * gridSize * .5f;

	for (int i = 0; i < numGridLines; i++)
	{
		AddVertsForLine3D(planeVerts, collOffset, collEndPos + collOffset, .05f, c_wireframeDefaultColor, transparentWireFrameColor);
		AddVertsForLine3D(planeVerts, collOffset, collStartPos + collOffset, .05f, c_wireframeDefaultColor, transparentWireFrameColor);
		collOffset += planeBitangent * gridSpacing;
	}

	AddVertsForCylinder3D(planeVerts, -m_position, Vec3::ZERO, .02f, Rgba8(125, 125, 125, 150));

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	m_modelMatrix.SetTranslation3D(m_position);
	g_theRenderer->BindTexture(m_texture);
	if (m_isOverlapping)
	{
		float normalizedAlpha = RangeMap(sinf(Clock::GetSystemClock().GetTotalSeconds() * 5.f), -1.f, 1.f, .2f, 1.f);
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
	g_theRenderer->DrawVertexArray(planeVerts.size(), planeVerts.data());
	std::vector<Vertex_PCU> nearestPointVerts;
	AddVertsForSphere3D(nearestPointVerts, m_nearestPoint, .05f, Rgba8(255, 150, 0));
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(nearestPointVerts.size(), nearestPointVerts.data());
}
