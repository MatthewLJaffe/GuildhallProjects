#pragma once

struct FloatRange
{
	static const FloatRange ZERO;
	static const FloatRange ONE;
	static const FloatRange ZERO_TO_ONE;
public:
	FloatRange() = default;
	FloatRange(float min, float max);
	float m_min = 0.f;
	float m_max = 0.f;
	void operator=(const FloatRange& copyFrom);
	bool operator==(const FloatRange& otherFloatRange) const;
	bool operator!=(const FloatRange& otherFloatRange) const;
	bool IsOnRange(float value) const;
	bool IsOverlappingWith(FloatRange const& otherFloatRange) const;
	void SetFromText(const char* text);
};