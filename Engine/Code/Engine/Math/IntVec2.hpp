#pragma once
struct Vec2;


struct IntVec2
{
public:
	static const IntVec2 ZERO;
	static const IntVec2 NORTH;
	static const IntVec2 SOUTH;
	static const IntVec2 WEST;
	static const IntVec2 EAST;
	static const IntVec2 NORTH_EAST;
	static const IntVec2 NORTH_WEST;
	static const IntVec2 SOUTH_EAST;
	static const IntVec2 SOUTH_WEST;

	IntVec2() = default;
	IntVec2(int x, int y);
	IntVec2(const IntVec2& copyFrom);
	int x = 0;
	int y = 0;
	float GetLength() const;
	int GetLengthSquared() const;
	int GetTaxicabLength() const;
	float GetOrientationRadians() const;
	float GetOrientationDegrees() const;
	IntVec2 GetRotated90Degrees() const;
	IntVec2 GetRotatedMinus90Degrees() const;
	void Rotate90Degrees();
	void RotateMinus90Degrees();
	Vec2 GetVec2();
	//operators
	const IntVec2	operator-(const IntVec2& vecToSubtract) const;	// vec2 - vec2
	const IntVec2	operator+(const IntVec2& vecToAdd) const;		// vec2 + vec2
	bool			operator==(const IntVec2& compare) const;		// IntVec2 == IntVec2
	bool			operator!=(const IntVec2& compare) const;		// IntVec2 != IntVec2
	bool			operator<(const IntVec2& compare) const;		// IntVec2 != IntVec2
	void SetFromText(char const* text);
};


