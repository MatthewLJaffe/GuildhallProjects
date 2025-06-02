#pragma once
#include "Vec2.hpp"
#include <string>

class RandomNumberGenerator;

class AABB2
{
public:
	static const AABB2 ZERO_TO_ONE;

	Vec2 m_mins;
	Vec2 m_maxs;
	AABB2() = default;
	AABB2(float minX, float minY, float maxX, float maxY);
	AABB2(const Vec2& mins, const Vec2& maxs);
	AABB2(const AABB2& copyFrom);
	bool IsPointInside(const Vec2& point) const;
	bool IsAABB2Inside(const AABB2& otherAABB2);
	Vec2 GetCenter() const;
	Vec2 GetDimensions() const;
	Vec2 GetPointAtUV(const Vec2& uv) const;
	Vec2 GetUVForPoint(const Vec2& point) const;
	Vec2 GetNearestPoint(const Vec2& point) const;
	Vec2 GetNearestPointOnPerimeter(const Vec2& point) const;
	void Translate(const Vec2& displacment);
	void SetCenter(const Vec2& newCenter);
	void SetDimensions(const Vec2& newDimensions);
	void StretchToIncludePoint(const Vec2& point);
	AABB2 GetFractionOfBox(Vec2 const& fractionMins, Vec2 const& fractionMaxs);
	Vec2 GetRandomPointOutsideBox(float distanceOutsideBox, RandomNumberGenerator* randGen);
	Vec2 GetRandomPointInsideBox(RandomNumberGenerator* randGen, Vec2 padding = Vec2::ZERO);
	void SetFromText(std::string const& text);
};