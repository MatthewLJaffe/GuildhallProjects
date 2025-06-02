#include "Engine/Math/IntRange.hpp"

IntRange::IntRange(int min, int max)
	: m_min(min)
	, m_max(max)
{}

void IntRange::operator=(const IntRange& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}

bool IntRange::operator==(const IntRange& otherIntRange) const
{
	return m_min == otherIntRange.m_min && m_max == otherIntRange.m_max;
}

bool IntRange::operator!=(const IntRange& otherIntRange) const
{
	return m_min != otherIntRange.m_min || m_max != otherIntRange.m_max;
}

bool IntRange::IsOnRange(int value)  const
{
	return m_min <= value && m_max >= value;
}

bool IntRange::IsOverlappingWith(IntRange const& otherIntRange) const
{
	return IsOnRange(otherIntRange.m_min) || IsOnRange(otherIntRange.m_max);
}

