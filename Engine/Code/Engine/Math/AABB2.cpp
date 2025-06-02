#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"

AABB2 const AABB2::ZERO_TO_ONE = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f));

AABB2::AABB2(float minX, float minY, float maxX, float maxY)
{
	m_mins = Vec2(minX, minY);
	m_maxs = Vec2(maxX, maxY);
}

AABB2::AABB2(const Vec2& mins, const Vec2& maxs)
{
	m_mins = Vec2(mins);
	m_maxs = Vec2(maxs);
}

AABB2::AABB2(const AABB2& copyFrom)
{
	m_mins = copyFrom.m_mins;
	m_maxs = copyFrom.m_maxs;
}

bool AABB2::IsPointInside(const Vec2& point) const
{
	return point.x >= m_mins.x && point.x <= m_maxs.x && point.y >= m_mins.y && point.y <= m_maxs.y;
}

bool AABB2::IsAABB2Inside(const AABB2& otherAABB2)
{
	return IsPointInside(otherAABB2.m_mins) && IsPointInside(otherAABB2.m_maxs);
}

Vec2 AABB2::GetCenter() const
{
	return Vec2((m_mins.x + m_maxs.x) / 2.f, (m_mins.y + m_maxs.y) / 2.f);
}

Vec2 AABB2::GetDimensions() const
{
	return Vec2(m_maxs.x - m_mins.x, m_maxs.y - m_mins.y);
}

Vec2 AABB2::GetPointAtUV(const Vec2& uv) const
{
	return Vec2(Lerp(m_mins.x, m_maxs.x, uv.x), Lerp(m_mins.y, m_maxs.y, uv.y));
}

Vec2 AABB2::GetUVForPoint(const Vec2& point) const
{
	Vec2 dimensions = GetDimensions();
	return Vec2((point.x - m_mins.x) / dimensions.x, (point.y - m_mins.y) / dimensions.y);
}

void AABB2::Translate(const Vec2& displacment)
{
	m_mins += displacment;
	m_maxs += displacment;
}

void AABB2::SetCenter(const Vec2& newCenter)
{
	Vec2 halfDimensions = GetDimensions() * .5f;
	m_mins = newCenter - halfDimensions;
	m_maxs = newCenter + halfDimensions;
}

void AABB2::SetDimensions(const Vec2& newDimensions)
{
	Vec2 halfDimensions = newDimensions * .5f;
	Vec2 center = GetCenter();
	m_mins = center - halfDimensions;
	m_maxs = center + halfDimensions;
}

void AABB2::StretchToIncludePoint(const Vec2& point)
{
	if (point.x > m_maxs.x)
		m_maxs.x = point.x;
	else if (point.x < m_mins.x)
		m_mins.x = point.x;
	if (point.y > m_maxs.y)
		m_maxs.y = point.y;
	else if (point.y < m_mins.y)
		m_mins.y = point.y;
}

AABB2 AABB2::GetFractionOfBox(Vec2 const& fractionMins, Vec2 const& fractionMaxs)
{
	Vec2 dimensions = GetDimensions();
	Vec2 boundsMins(m_mins.x + (fractionMins.x*dimensions.x), m_mins.y + (fractionMins.y*dimensions.y));
	Vec2 boundsMaxs(m_mins.x + (fractionMaxs.x * dimensions.x), m_mins.y + (fractionMaxs.y * dimensions.y));
	return AABB2(boundsMins, boundsMaxs);
}

Vec2 AABB2::GetNearestPoint(const Vec2& point) const
{
	Vec2 nearestPoint;
	nearestPoint.x = Clamp(point.x, m_mins.x, m_maxs.x);
	nearestPoint.y = Clamp(point.y, m_mins.y, m_maxs.y);
	return nearestPoint;
}

Vec2 AABB2::GetNearestPointOnPerimeter(const Vec2& point) const
{
	if (!IsPointInside(point))
	{
		return GetNearestPoint(point);
	}
	float distToMinX = point.x - m_mins.x;
	float distToMaxX = m_maxs.x - point.x;

	float distToMinY = point.y - m_mins.y;
	float distToMaxY = m_maxs.y - point.y;

	if (distToMinX < distToMaxX && distToMinX < distToMinY && distToMinX < distToMaxY)
	{
		return Vec2(m_mins.x, point.y);
	}
	else if (distToMaxX < distToMinX && distToMaxX < distToMinY && distToMaxX < distToMaxY)
	{
		return Vec2(m_maxs.x, point.y);
	}
	else if (distToMinY < distToMaxX && distToMinY < distToMinX && distToMinY < distToMaxY)
	{
		return Vec2(point.x, m_mins.y);
	}
	else
	{
		return Vec2(point.x, m_maxs.y);
	}
}

Vec2  AABB2::GetRandomPointOutsideBox(float distanceOutsideBox, RandomNumberGenerator* randGen)
{
	Vec2 dimensions = GetDimensions();
	float sphereRadius = dimensions.x;
	if (sphereRadius < dimensions.y)
	{
		sphereRadius = dimensions.y;
	}
	Vec2 randomDir = randGen->RollRandomNormalizedVec2();
	Vec2 pointOutsideBox = randomDir * sphereRadius + GetCenter();
	Vec2 randomPointOnBox = GetNearestPoint(pointOutsideBox);
	return randomPointOnBox + randomDir * distanceOutsideBox;
}

Vec2 AABB2::GetRandomPointInsideBox(RandomNumberGenerator* randGen, Vec2 padding)
{
	float minX = m_mins.x + padding.x;
	float minY = m_mins.y + padding.y;

	float maxX = m_maxs.x - padding.x;
	float maxY = m_maxs.y - padding.y;

	Vec2 randomPoint;
	randomPoint.x = randGen->RollRandomFloatInRange(minX, maxX);
	randomPoint.y = randGen->RollRandomFloatInRange(minY, maxY);
	return randomPoint;
}


void AABB2::SetFromText(std::string const& text)
{
	Strings values = SplitStringOnDelimiter(text, ',', true);
	GUARANTEE_OR_DIE(values.size() == 4, "Parsing AABB2 failed");
	m_mins.x = (float)atof(values[0].c_str());
	m_mins.y = (float)atof(values[1].c_str());
	m_maxs.x = (float)atof(values[0].c_str());
	m_maxs.y = (float)atof(values[1].c_str());
}

