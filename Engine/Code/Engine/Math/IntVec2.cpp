#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

IntVec2 const IntVec2::ZERO = IntVec2(0, 0);
IntVec2 const IntVec2::NORTH = IntVec2(0, 1);
IntVec2 const IntVec2::SOUTH = IntVec2(0, -1);
IntVec2 const IntVec2::WEST = IntVec2(-1, 0);
IntVec2 const IntVec2::EAST = IntVec2(1, 0);
IntVec2 const IntVec2::NORTH_WEST = IntVec2(-1, 1);
IntVec2 const IntVec2::NORTH_EAST = IntVec2(1, 1);
IntVec2 const IntVec2::SOUTH_WEST = IntVec2(-1, -1);
IntVec2 const IntVec2::SOUTH_EAST = IntVec2(1, -1);

IntVec2::IntVec2(int startX, int startY)
	: x(startX)
	, y(startY)
{}

IntVec2::IntVec2(const IntVec2& copyFrom)
	: x(copyFrom.x)
	, y(copyFrom.y)
{}

float IntVec2::GetLength() const
{
	return sqrtf(static_cast<float>(x*x + y*y));
}

int IntVec2::GetLengthSquared() const
{
	return x*x + y*y;
}

int IntVec2::GetTaxicabLength() const
{
	return abs(x) + abs(y);
}

float IntVec2::GetOrientationRadians() const
{
	return atan2f(static_cast<float>(y), static_cast<float>(x));
}

float IntVec2::GetOrientationDegrees() const
{
	return ConvertRadiansToDegrees(atan2f(static_cast<float>(y), static_cast<float>(x)));
}

IntVec2 IntVec2::GetRotated90Degrees() const
{
	return IntVec2(-y, x);
}

IntVec2 IntVec2::GetRotatedMinus90Degrees() const
{
	return IntVec2(y, -x);
}

void IntVec2::Rotate90Degrees()
{
	int oldX = x;
	x = -y;
	y = oldX;
}

void IntVec2::RotateMinus90Degrees()
{
	int oldX = x;
	x = y;
	y = -oldX;
}

Vec2 IntVec2::GetVec2()
{
	return Vec2((float)x, (float)y);
}

const IntVec2 IntVec2::operator-(const IntVec2& vecToSubtract) const
{
	return IntVec2(this->x - vecToSubtract.x, this->y - vecToSubtract.y);
}

const IntVec2 IntVec2::operator+(const IntVec2& vecToAdd) const
{
	return IntVec2(this->x + vecToAdd.x, this->y + vecToAdd.y);
}

bool IntVec2::operator==(const IntVec2& compare) const
{
	return this->x == compare.x && this->y == compare.y;
}

bool IntVec2::operator!=(const IntVec2& compare) const
{
	return this->x != compare.x || this->y != compare.y;
}

bool IntVec2::operator<(const IntVec2& compare) const
{
	if (this->y < compare.y)
	{
		return true;
	}
	else if (this->y == compare.y && this->x < compare.x)
	{
		return true;
	}
	return false;
}

void IntVec2::SetFromText(char const* text)
{
	Strings values = SplitStringOnDelimiter(text, ',');
	GUARANTEE_OR_DIE(values.size() == 2, "Parsing Vec2Int failed to get x and y values");
	this->x = atoi(values[0].c_str());
	this->y = atoi(values[1].c_str());
}
