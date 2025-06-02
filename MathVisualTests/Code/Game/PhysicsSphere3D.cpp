#include "Game/PhysicsSphere3D.hpp"
#include "Game/PhysicsCylinder3D.hpp"
#include "Game/PhysicsAABB3D.hpp"
PhysicsSphere3D::PhysicsSphere3D(Vec3 const& position, ShapeType shapeType, bool wireFrame, Texture* texture, float radius)
	: PhysicsShape3D(position, shapeType, wireFrame, texture)
	, m_radius(radius)
{
	int numSlices = g_randGen->RollRandomIntInRange(4, 32);
	int numStacks = g_randGen->RollRandomIntInRange(4, 16);
	AddVertsForSphere3D(m_verts, Vec3::ZERO, m_radius, Rgba8::WHITE, AABB2::ZERO_TO_ONE, numSlices, numStacks);
}

void PhysicsSphere3D::CheckForOverlapWithOtherShapes(std::vector<std::vector<PhysicsShape3D*>> const& listOfShapesByType)
{
	for (size_t i = 0; i < listOfShapesByType[(int)ShapeType::AABB3].size(); i++)
	{
		PhysicsAABB3D* currAABB = dynamic_cast<PhysicsAABB3D*>(listOfShapesByType[(int)ShapeType::AABB3][i]);
		if (DoSphereAndAABBOverlap3D(m_position, m_radius, currAABB->m_bounds))
		{
			m_isOverlapping = true;
			return;
		}
	}
	for (size_t i = 0; i < listOfShapesByType[(int)ShapeType::CYLINDER].size(); i++)
	{
		PhysicsCylinder3D* currCylinder = dynamic_cast<PhysicsCylinder3D*>(listOfShapesByType[(int)ShapeType::CYLINDER][i]);
		if (DoZCylinderAndSphereOverlap3D(currCylinder->m_XYCenterPos, currCylinder->m_radius, currCylinder->m_minMaxZRange, m_position, m_radius))
		{
			m_isOverlapping = true;
			return;
		}
	}
	for (size_t i = 0; i < listOfShapesByType[(int)ShapeType::SPHERE].size(); i++)
	{
		PhysicsSphere3D* currSphere = dynamic_cast<PhysicsSphere3D*>(listOfShapesByType[(int)ShapeType::SPHERE][i]);
		if (currSphere == this)
		{
			continue;
		}
		if (DoSpheresOverlap3D(m_position, m_radius, currSphere->GetPosition(), currSphere->m_radius))
		{
			m_isOverlapping = true;
			return;
		}
	}

	m_isOverlapping = false;
}

RaycastResult3D PhysicsSphere3D::RaycastVsShape(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist)
{
	return RaycastVsSphere3D(startPos, fwdNormal, maxDist, m_position, m_radius);
}

Vec3 PhysicsSphere3D::GetNearestPointOnShape(Vec3 const& pointToQuery)
{
	return GetNearestPointOnSphere3D(pointToQuery, m_position, m_radius);
}

bool PhysicsSphere3D::IsOverlappingSphere(Vec3 const& otherSpherePos, float otherSphereRadius)
{
	return DoSpheresOverlap3D(m_position, m_radius, otherSpherePos, otherSphereRadius);
}

bool PhysicsSphere3D::IsOverlappingCylinder(Vec2 const& otherCylinderCenterXY, float otherCylinderRadius, FloatRange const& cylinderMinMaxZ)
{
	return DoZCylinderAndSphereOverlap3D(otherCylinderCenterXY, otherCylinderRadius, cylinderMinMaxZ, m_position, m_radius);
}

bool PhysicsSphere3D::IsOverlappingAABB3(AABB3 const& box)
{
	return DoSphereAndAABBOverlap3D(m_position, m_radius, box);
}
