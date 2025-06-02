#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/EngineCommon.hpp"

FloatRange const FloatRange::ZERO = FloatRange(0.f, 0.f);
FloatRange const FloatRange::ONE = FloatRange(1.f, 1.f);
FloatRange const FloatRange::ZERO_TO_ONE = FloatRange(0.f, 1.f);

FloatRange::FloatRange(float min, float max)
	: m_min(min)
	, m_max(max)
{}

void FloatRange::operator=(const FloatRange& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}

bool FloatRange::operator==(const FloatRange& otherFloatRange) const
{
	return m_min == otherFloatRange.m_min && m_max == otherFloatRange.m_max;
}

bool FloatRange::operator!=(const FloatRange& otherFloatRange) const
{
	return m_min != otherFloatRange.m_min || m_max != otherFloatRange.m_max;
}

bool FloatRange::IsOnRange(float value)  const
{
	return m_min <= value && m_max >= value;
}

bool FloatRange::IsOverlappingWith(FloatRange const& otherFloatRange) const
{
	return	IsOnRange(otherFloatRange.m_min) || IsOnRange(otherFloatRange.m_max) ||
			otherFloatRange.IsOnRange(m_min) || otherFloatRange.IsOnRange(m_max);
}

void FloatRange::SetFromText(const char* text)
{
	Strings values = SplitStringOnDelimiter(text, '~');
	GUARANTEE_OR_DIE(values.size() == 2, "Parsing FloatRange failed to get min and max values");
	this->m_min = (float)atof(values[0].c_str());
	this->m_max = (float)atof(values[1].c_str());
}

