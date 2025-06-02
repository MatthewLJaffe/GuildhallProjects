#pragma once
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/FloatRange.hpp"

class Texture;

enum class ShapeType
{
	SPHERE,
	CYLINDER,
	AABB3,
	PLANE,
	OBB3,
	COUNT
};

class PhysicsShape3D
{
public:
	PhysicsShape3D(Vec3 const& position, ShapeType shapeType, bool wireFrame, Texture* texture);
	virtual void Render();

	//Queries
	virtual RaycastResult3D RaycastVsShape(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist) = 0;
	void SetNearestPoint(Vec3 const& refrencePos);

	virtual Vec3 GetNearestPointOnShape(Vec3 const& pointToQuery) = 0;
	virtual void CheckForOverlapWithOtherShapes(std::vector<std::vector<PhysicsShape3D*>> const& listOfShapesByType) = 0;
	virtual bool IsOverlappingSphere(Vec3 const& otherSpherePos, float otherSphereRadius) = 0;
	virtual bool IsOverlappingCylinder(Vec2 const& otherCylinderCenterXY, float otherCylinderRadius, FloatRange const& cylinderMinMaxZ) = 0;
	virtual bool IsOverlappingAABB3(AABB3 const& box) = 0;
	Vec3 GetPosition() const;
	virtual void SetPosition(Vec3 const& newPosition);

public:
	std::vector<Vertex_PCU> m_verts;
	ShapeType m_shapeType;
	bool m_wireFrame = true;
	bool m_isOverlapping = false;
	bool m_isHeld = false;
	Mat44 m_modelMatrix = Mat44();
	Rgba8 m_color = Rgba8::WHITE;
	Texture* m_texture = nullptr;
	Vec3 m_nearestPoint = Vec3(0, 0, 0);
	const Rgba8 c_wireframeDefaultColor = Rgba8(100, 150, 255);
	const Rgba8 c_grabbedColor = Rgba8(255, 0 ,0);
	const Rgba8 c_hitColor = Rgba8(0, 0, 255);

protected:
	Vec3 m_position;
	
};
