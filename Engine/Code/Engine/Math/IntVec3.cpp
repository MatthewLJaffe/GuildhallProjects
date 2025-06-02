#include "Engine/Math/IntVec3.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"

IntVec3 const IntVec3::ZERO = IntVec3(0, 0, 0);

IntVec3::IntVec3(int startX, int startY, int startZ)
	: x(startX)
	, y(startY)
	, z(startZ)
{}

IntVec3::IntVec3(const IntVec3& copyFrom)
	: x(copyFrom.x)
	, y(copyFrom.y)
	, z(copyFrom.z)
{}

float IntVec3::GetLength() const
{
	return sqrtf(static_cast<float>(x*x + y*y + z*z));
}

int IntVec3::GetLengthSquared() const
{
	return x*x + y*y + z*z;
}

int IntVec3::GetTaxicabLength() const
{
	return abs(x) + abs(y) + abs(z);
}

Vec3 IntVec3::GetVec3() const
{
	return Vec3( (float)x, (float)y, (float)z );
}

const IntVec3 IntVec3::operator-(const IntVec3& vecToSubtract) const
{
	return IntVec3(this->x - vecToSubtract.x, this->y - vecToSubtract.y, this->z - vecToSubtract.z);
}

const IntVec3 IntVec3::operator+(const IntVec3& vecToAdd) const
{
	return IntVec3(this->x + vecToAdd.x, this->y + vecToAdd.y, this->z + vecToAdd.z);
}

bool IntVec3::operator==(const IntVec3& compare) const
{
	return this->x == compare.x && this->y == compare.y && this->z == compare.z;
}

bool IntVec3::operator!=(const IntVec3& compare) const
{
	return this->x != compare.x || this->y != compare.y || this->z != compare.z;
}

bool IntVec3::operator<(const IntVec3& compare) const
{
	if (this->z < compare.z)
	{
		return true;
	}
	else if (this->z == compare.z && this->y < compare.y)
	{
		return true;
	}
	else if (this->z == compare.z && this->y == compare.y && this->x < compare.x)
	{
		return true;
	}
	return false;
}

void IntVec3::SetFromText(char const* text, char delimeterToSplitOn)
{
	Strings values = SplitStringOnDelimiter(text, delimeterToSplitOn);
	GUARANTEE_OR_DIE(values.size() == 3, "Parsing IntVec3 failed to get x, y, z values");
	if (values[0] == "" && delimeterToSplitOn == '/')
	{
		this->x = 0;
	}
	else
	{
		this->x = atoi(values[0].c_str());
	}
	if (values[1] == "" && delimeterToSplitOn == '/')
	{
		this->y = 0;
	}
	else
	{
		this->y = atoi(values[1].c_str());
	}
	if (values[2] == "" && delimeterToSplitOn == '/')
	{
		this->z = 0;
	}
	else
	{
		this->z = atoi(values[2].c_str());
	}
}
