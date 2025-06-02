#include "Game/PhysicsOBB3D.hpp"
#include "Game/PhysicsPlane3D.hpp"

PhysicsOBB3D::PhysicsOBB3D(Vec3 const& position, ShapeType shapeType, bool wireFrame, Texture* texture, Vec3 iBasis, Vec3 jBasis, Vec3 kBasis, Vec3 halfDimensions)
	: PhysicsShape3D(position, shapeType, wireFrame, texture)
	, m_obb3(iBasis, jBasis, kBasis, halfDimensions, position)
{
	OBB3 nonTransformedOBB3(iBasis, jBasis, kBasis, halfDimensions, Vec3::ZERO);
	AddVertsForOBB3(m_verts, nonTransformedOBB3);
}

void PhysicsOBB3D::CheckForOverlapWithOtherShapes(std::vector<std::vector<PhysicsShape3D*>> const& listOfShapesByType)
{
	for (size_t i = 0; i < listOfShapesByType[(int)ShapeType::PLANE].size(); i++)
	{
		PhysicsPlane3D* currPlane = dynamic_cast<PhysicsPlane3D*>(listOfShapesByType[(int)ShapeType::PLANE][i]);
		if (DoPlaneAndOBB3Overlap3D(currPlane->m_plane, m_obb3))
		{
			m_isOverlapping = true;
			return;
		}
	}
	m_isOverlapping = false;
}

RaycastResult3D PhysicsOBB3D::RaycastVsShape(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist)
{
	return RaycastVsOBB3(startPos, fwdNormal, maxDist, m_obb3);
}

Vec3 PhysicsOBB3D::GetNearestPointOnShape(Vec3 const& pointToQuery)
{
	return GetNearestPointOnOBB3(pointToQuery, m_obb3);
}

//#ToDo implement IsOverlappingSphere for OBB3
bool PhysicsOBB3D::IsOverlappingSphere(Vec3 const& otherSpherePos, float otherSphereRadius)
{
	UNUSED(otherSpherePos);
	UNUSED(otherSphereRadius);
	return false;
}

//#ToDo implement IsOverlappingCylinder for OBB3
bool PhysicsOBB3D::IsOverlappingCylinder(Vec2 const& otherCylinderCenterXY, float otherCylinderRadius, FloatRange const& cylinderMinMaxZ)
{
	UNUSED(otherCylinderCenterXY);
	UNUSED(otherCylinderRadius);
	UNUSED(cylinderMinMaxZ);
	return false;
}

//#ToDo implement IsOverlappingAABB3 for OBB3
bool PhysicsOBB3D::IsOverlappingAABB3(AABB3 const& box)
{
	UNUSED(box);
	return false;
}

void PhysicsOBB3D::SetPosition(Vec3 const& newPosition)
{
	m_position = newPosition;
	m_obb3.m_center = m_position;
}
