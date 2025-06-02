#pragma once
#include "Game/PhysicsShape3D.hpp"
#include "Engine/Math/Plane3.hpp"

class PhysicsPlane3D : public PhysicsShape3D
{
public:
	PhysicsPlane3D(Plane3 const& plane, ShapeType shapeType, bool wireFrame, Texture* texture);

	//Queries
	virtual RaycastResult3D RaycastVsShape(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) override;

	Vec3 GetNearestPointOnShape(Vec3 const& pointToQuery) override;
	virtual void CheckForOverlapWithOtherShapes(std::vector<std::vector<PhysicsShape3D*>> const& listOfShapesByType) override;
	virtual bool IsOverlappingSphere(Vec3 const& otherSpherePos, float otherSphereRadius) override;
	virtual bool IsOverlappingCylinder(Vec2 const& otherCylinderCenterXY, float otherCylinderRadius, FloatRange const& cylinderMinMaxZ) override;
	virtual bool IsOverlappingAABB3(AABB3 const& box) override;
	virtual void SetPosition(Vec3 const& newPosition) override;
	virtual void Render() override;
	Plane3 m_plane;
};