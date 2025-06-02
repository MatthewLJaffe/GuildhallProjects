#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/MathUtils.hpp"

OBB2::OBB2(Vec2 const& center, Vec2 const& halfDimensions, Vec2 const& iBasisNormal)
	: m_center(center)
	, m_halfDimensions(halfDimensions)
	, m_iBasisNormal(iBasisNormal)
{ }

OBB2::OBB2(Vec2 const& center, Vec2 const& halfDimensions, float orientation)
	: m_center(center)
	, m_halfDimensions(halfDimensions)
	, m_iBasisNormal(Vec2::MakeFromPolarDegrees(orientation))
{ }
