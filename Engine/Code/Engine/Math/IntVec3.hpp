#pragma once
struct Vec3;

struct IntVec3
{
public:
	static const IntVec3 ZERO;

	IntVec3() = default;
	IntVec3(int x, int y, int z);
	IntVec3(const IntVec3& copyFrom);
	int x = 0;
	int y = 0;
	int z = 0;
	float GetLength() const;
	int GetLengthSquared() const;
	int GetTaxicabLength() const;
	Vec3 GetVec3() const;

	//operators
	const IntVec3	operator-(const IntVec3& vecToSubtract) const;	// vec2 - vec2
	const IntVec3	operator+(const IntVec3& vecToAdd) const;		// vec2 + vec2
	bool			operator==(const IntVec3& compare) const;		// IntVec3 == IntVec3
	bool			operator!=(const IntVec3& compare) const;		// IntVec3 != IntVec3
	bool			operator<(const IntVec3& compare) const;		
	void SetFromText(char const* text, char delimeterToSplitOn = ',');
};


