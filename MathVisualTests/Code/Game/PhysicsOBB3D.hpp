#pragma once
#pragma once
#include "Game/PhysicsShape3D.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/OBB3.hpp"

class PhysicsOBB3D : public PhysicsShape3D
{
public:
	PhysicsOBB3D(Vec3 const& position, ShapeType shapeType, bool wireFrame, Texture* texture, Vec3 iBasis, Vec3 jBasis, Vec3 kBasis, Vec3 halfDimensions);

	//queries
	void CheckForOverlapWithOtherShapes(std::vector<std::vector<PhysicsShape3D*>> const& listOfShapesByType) override;
	RaycastResult3D RaycastVsShape(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) override;
	Vec3 GetNearestPointOnShape(Vec3 const& pointToQuery) override;
	bool IsOverlappingSphere(Vec3 const& otherSpherePos, float otherSphereRadius) override;
	bool IsOverlappingCylinder(Vec2 const& otherCylinderCenterXY, float otherCylinderRadius, FloatRange const& cylinderMinMaxZ) override;
	bool IsOverlappingAABB3(AABB3 const& box) override;
	void SetPosition(Vec3 const& newPosition) override;
public:
	OBB3 m_obb3;
};