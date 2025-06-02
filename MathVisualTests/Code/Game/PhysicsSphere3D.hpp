#pragma once
#include "Game/PhysicsShape3D.hpp"
#include "Engine/Math/FloatRange.hpp"

class PhysicsSphere3D : public PhysicsShape3D
{
public:
	PhysicsSphere3D(Vec3 const& position, ShapeType shapeType, bool wireFrame, Texture* texture, float radius);

	//queries
	void CheckForOverlapWithOtherShapes(std::vector<std::vector<PhysicsShape3D*>> const& listOfShapesByType) override;
	RaycastResult3D RaycastVsShape(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) override;
	Vec3 GetNearestPointOnShape(Vec3 const& pointToQuery) override;
	bool IsOverlappingSphere(Vec3 const& otherSpherePos, float otherSphereRadius) override;
	bool IsOverlappingCylinder(Vec2 const& otherCylinderCenterXY, float otherCylinderRadius, FloatRange const& cylinderMinMaxZ) override;
	bool IsOverlappingAABB3(AABB3 const& box) override;
public:
	float m_radius = 1.f;
};