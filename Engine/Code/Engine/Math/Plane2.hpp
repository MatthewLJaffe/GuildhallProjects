#pragma once
#include "Engine/Math/Vec2.hpp"

struct Plane2
{
	Plane2(Vec2 normal, float distAlongForward);
	Vec2 m_normal;
	float m_distFromOriginAlongNormal = 0.f;
	float GetAltitude(Vec2 const& point) const;
	bool IsPointInFrontOfPlane(Vec2 const& point) const;
};