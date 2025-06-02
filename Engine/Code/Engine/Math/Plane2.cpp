#include "Engine/Math/Plane2.hpp"
#include "Engine/Math/MathUtils.hpp"

Plane2::Plane2(Vec2 normal, float distAlongForward)
    : m_normal(normal)
    , m_distFromOriginAlongNormal(distAlongForward)
{
}

float Plane2::GetAltitude(Vec2 const& point) const
{
    return DotProduct2D(point, m_normal) - m_distFromOriginAlongNormal;
}

bool Plane2::IsPointInFrontOfPlane(Vec2 const& point) const
{
    return DotProduct2D(point, m_normal) > m_distFromOriginAlongNormal;
}
