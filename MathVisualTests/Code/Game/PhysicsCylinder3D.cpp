#include "Game/PhysicsCylinder3D.hpp"
#include "Game/PhysicsAABB3D.hpp"
#include "Game/PhysicsSphere3D.hpp"

PhysicsCylinder3D::PhysicsCylinder3D(Vec3 const& position, ShapeType shapeType, bool wireFrame, Texture* texture, float radius, float height)
	: PhysicsShape3D(position, shapeType, wireFrame, texture)
	, m_radius(radius)
	, m_XYCenterPos(Vec2(m_position.x, m_position.y))
	, m_minMaxZRange(FloatRange(m_position.z, m_position.z + height))
{
	int numSlices = g_randGen->RollRandomIntInRange(8, 32);
	AddVertsForCylinder3D(m_verts, Vec3::ZERO, Vec3(0.f, 0.f, height), 
		m_radius, Rgba8::WHITE, AABB2::ZERO_TO_ONE, numSlices);

}

void PhysicsCylinder3D::CheckForOverlapWithOtherShapes(std::vector<std::vector<PhysicsShape3D*>> const& listOfShapesByType)
{
	for (size_t i = 0; i < listOfShapesByType[(int)ShapeType::AABB3].size(); i++)
	{
		PhysicsAABB3D* currAABB = dynamic_cast<PhysicsAABB3D*>(listOfShapesByType[(int)ShapeType::AABB3][i]);
		if (DoZCylinderAndAABBOverlap3D(m_XYCenterPos, m_radius, m_minMaxZRange, currAABB->m_bounds))
		{
			m_isOverlapping = true;
			return;
		}
	}
	for (size_t i = 0; i < listOfShapesByType[(int)ShapeType::CYLINDER].size(); i++)
	{
		PhysicsCylinder3D* currCylinder = dynamic_cast<PhysicsCylinder3D*>(listOfShapesByType[(int)ShapeType::CYLINDER][i]);
		if (currCylinder == this)
		{
			continue;
		}
		if ( DoZCylindersOverlap(m_XYCenterPos, m_radius, m_minMaxZRange, currCylinder->m_XYCenterPos, currCylinder->m_radius, currCylinder->m_minMaxZRange) )
		{
			m_isOverlapping = true;
			return;
		}
	}
	for (size_t i = 0; i < listOfShapesByType[(int)ShapeType::SPHERE].size(); i++)
	{
		PhysicsSphere3D* currSphere = dynamic_cast<PhysicsSphere3D*>(listOfShapesByType[(int)ShapeType::SPHERE][i]);
		if (DoZCylinderAndSphereOverlap3D(m_XYCenterPos, m_radius, m_minMaxZRange, currSphere->GetPosition(), currSphere->m_radius))
		{
			m_isOverlapping = true;
			return;
		}
	}

	m_isOverlapping = false;
}

RaycastResult3D PhysicsCylinder3D::RaycastVsShape(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist)
{
	return RaycastVsCylinderZ3D(startPos, fwdNormal, maxDist, m_XYCenterPos, m_radius, m_minMaxZRange);
}

Vec3 PhysicsCylinder3D::GetNearestPointOnShape(Vec3 const& pointToQuery)
{
	return GetNearestPointOnCylinder3D(pointToQuery, m_XYCenterPos, m_radius, m_minMaxZRange);
}

bool PhysicsCylinder3D::IsOverlappingSphere(Vec3 const& otherSpherePos, float otherSphereRadius)
{
	return DoZCylinderAndSphereOverlap3D(m_XYCenterPos, m_radius, m_minMaxZRange, otherSpherePos, otherSphereRadius);
}

bool PhysicsCylinder3D::IsOverlappingCylinder(Vec2 const& otherCylinderCenterXY, float otherCylinderRadius, FloatRange const& cylinderMinMaxZ)
{
	return DoZCylindersOverlap(m_XYCenterPos, m_radius, m_minMaxZRange, otherCylinderCenterXY, otherCylinderRadius, cylinderMinMaxZ);
}

bool PhysicsCylinder3D::IsOverlappingAABB3(AABB3 const& box)
{
	return DoZCylinderAndAABBOverlap3D(m_XYCenterPos, m_radius, m_minMaxZRange, box);
}

void PhysicsCylinder3D::SetPosition(Vec3 const& newPosition)
{
	m_position = newPosition;
	m_XYCenterPos = Vec2(m_position.x, m_position.y);
	float height = m_minMaxZRange.m_max - m_minMaxZRange.m_min;
	m_minMaxZRange.m_min = m_position.z;
	m_minMaxZRange.m_max = m_position.z + height;
}
