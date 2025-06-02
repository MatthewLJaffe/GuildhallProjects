#include "Game/PhysicsAABB3D.hpp"
#include "Game/PhysicsCylinder3D.hpp"
#include "Game/PhysicsSphere3D.hpp"

PhysicsAABB3D::PhysicsAABB3D(Vec3 const& position, ShapeType shapeType, bool wireFrame, Texture* texture, Vec3 dimensions)
	: PhysicsShape3D(position, shapeType, wireFrame, texture)
	, m_bounds(AABB3(m_position, m_position + dimensions))
{
 	AddVertsForAABB3D(m_verts, AABB3(Vec3::ZERO, dimensions));
}

void PhysicsAABB3D::CheckForOverlapWithOtherShapes(std::vector<std::vector<PhysicsShape3D*>> const& listOfShapesByType)
{
	for (size_t i = 0; i < listOfShapesByType[(int)ShapeType::AABB3].size(); i++)
	{
		PhysicsAABB3D* currAABB = dynamic_cast<PhysicsAABB3D*>(listOfShapesByType[(int)ShapeType::AABB3][i]);
		if (currAABB == this)
		{
			continue;
		}
		if (DoAABBsOverlap3D(m_bounds, currAABB->m_bounds))
		{
			m_isOverlapping = true;
			return;
		}
	}
	for (size_t i = 0; i < listOfShapesByType[(int)ShapeType::CYLINDER].size(); i++)
	{
		PhysicsCylinder3D* currCylinder = dynamic_cast<PhysicsCylinder3D*>(listOfShapesByType[(int)ShapeType::CYLINDER][i]);
		if (DoZCylinderAndAABBOverlap3D(currCylinder->m_XYCenterPos, currCylinder->m_radius, currCylinder->m_minMaxZRange, m_bounds))
		{
			m_isOverlapping = true;
			return;
		}
	}
	for (size_t i = 0; i < listOfShapesByType[(int)ShapeType::SPHERE].size(); i++)
	{
		PhysicsSphere3D* currSphere = dynamic_cast<PhysicsSphere3D*>(listOfShapesByType[(int)ShapeType::SPHERE][i]);
		if (DoSphereAndAABBOverlap3D(currSphere->GetPosition(), currSphere->m_radius, m_bounds))
		{
			m_isOverlapping = true;
			return;
		}
	}

	m_isOverlapping = false;
}

RaycastResult3D PhysicsAABB3D::RaycastVsShape(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist)
{
	return RaycastVsAABB3D(startPos, fwdNormal, maxDist, m_bounds);
}

Vec3 PhysicsAABB3D::GetNearestPointOnShape(Vec3 const& pointToQuery)
{
	return GetNearestPointOnAABB3D(pointToQuery, m_bounds);
}

bool PhysicsAABB3D::IsOverlappingSphere(Vec3 const& otherSpherePos, float otherSphereRadius)
{
	return DoSphereAndAABBOverlap3D(otherSpherePos, otherSphereRadius, m_bounds);
}

bool PhysicsAABB3D::IsOverlappingCylinder(Vec2 const& otherCylinderCenterXY, float otherCylinderRadius, FloatRange const& cylinderMinMaxZ)
{
	return DoZCylinderAndAABBOverlap3D(otherCylinderCenterXY, otherCylinderRadius, cylinderMinMaxZ, m_bounds);
}

bool PhysicsAABB3D::IsOverlappingAABB3(AABB3 const& box)
{
	return DoAABBsOverlap3D(m_bounds, box);
}

void PhysicsAABB3D::SetPosition(Vec3 const& newPosition)
{
	m_position = newPosition;
	Vec3 dimensions = m_bounds.GetDimensions();
	m_bounds.m_mins = newPosition;
	m_bounds.m_maxs = m_bounds.m_mins + dimensions;
}
