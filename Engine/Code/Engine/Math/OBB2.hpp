#pragma once
#include "Engine/Math/Vec2.hpp"

class OBB2
{
public:
	OBB2() = default;
	OBB2(Vec2 const& center, Vec2 const& halfDimensions, Vec2 const& iBasisNormal);
	OBB2(Vec2 const& center, Vec2 const& halfDimensions, float orientation);
	Vec2 m_center;
	Vec2 m_halfDimensions;
	Vec2 m_iBasisNormal;
};