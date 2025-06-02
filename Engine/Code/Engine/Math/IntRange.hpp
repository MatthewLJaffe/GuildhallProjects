#pragma once

struct IntRange
{
public:
	IntRange() = default;
	IntRange(int min, int max);
	int m_min = 0;
	int m_max = 0;
	void operator=(const IntRange& copyFrom);
	bool operator==(const IntRange& otherIntRange) const;
	bool operator!=(const IntRange& otherIntRange) const;
	bool IsOnRange(int value) const;
	bool IsOverlappingWith(IntRange const& otherIntRange) const;
};