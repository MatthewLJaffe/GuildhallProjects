#include "Engine/Math/Plane3.hpp"
#include "Engine/Math/MathUtils.hpp"

float Plane3::GetAltitude(Vec3 const& point) const 
{
	return DotProduct3D(point, m_normal) - m_distFromOriginAlongNormal;
}

bool Plane3::IsPointInFrontOfPlane(Vec3 const& point) const
{
	return DotProduct3D(point, m_normal) > m_distFromOriginAlongNormal;
}
