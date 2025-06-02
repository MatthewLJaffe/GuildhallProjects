#pragma once
#include "Engine/Math/Vec3.hpp"

struct Plane3
{
	Vec3 m_normal = Vec3(0.f, 0.f, 1.f);
	float m_distFromOriginAlongNormal = 0.f;
	float GetAltitude(Vec3 const& point) const;
	bool IsPointInFrontOfPlane(Vec3 const& point) const;
};