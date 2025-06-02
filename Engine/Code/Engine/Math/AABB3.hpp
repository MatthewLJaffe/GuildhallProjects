#pragma once
#include "Engine/Math/Vec3.hpp"

class AABB3
{
public:
	Vec3 m_mins;
	Vec3 m_maxs;
	AABB3() = default;
	AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	AABB3(const Vec3 & mins, const Vec3 & maxs);
	AABB3(const AABB3 & copyFrom);
	Vec3 GetDimensions() const;
	bool IsPointInside(Vec3 const& point) const;
	AABB3 GetTranslated(Vec3 const& translation) const;
	Vec3 GetCenter() const;
};