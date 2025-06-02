#include "Engine/Math/AABB3.hpp"

AABB3::AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
	: m_mins(minX, minY, minZ)
	, m_maxs(maxX, maxY, maxZ)
{

}

AABB3::AABB3(const Vec3& mins, const Vec3& maxs)
	: m_mins(mins)
	, m_maxs(maxs)
{
}

AABB3::AABB3(const AABB3& copyFrom)
	: m_mins(copyFrom.m_mins)
	, m_maxs(copyFrom.m_maxs)
{
}

Vec3 AABB3::GetDimensions() const
{
	return m_maxs - m_mins;
}

bool AABB3::IsPointInside(Vec3 const& point) const
{
	return	point.x > m_mins.x && point.x < m_maxs.x &&
			point.y > m_mins.y && point.y < m_maxs.y && 
			point.z > m_mins.z && point.z < m_maxs.z;
}

AABB3 AABB3::GetTranslated(Vec3 const& translation) const
{
	return AABB3(m_mins + translation, m_maxs + translation);
}

Vec3 AABB3::GetCenter() const
{
	return Vec3::Lerp(m_mins, m_maxs, .5f);
}
