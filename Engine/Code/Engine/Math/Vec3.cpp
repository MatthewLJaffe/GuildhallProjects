#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"

Vec3 const Vec3::ZERO = Vec3(0.f, 0.f, 0.f);
//Vec3 const Vec3::LEFT = Vec3(0.f, 1.f, 0.f);
//Vec3 const Vec3::RIGHT = Vec3(0.f, -1.f, 0.f);
//Vec3 const Vec3::UP = Vec3(0.f, 0.f, 1.f);
//Vec3 const Vec3::DOWN = Vec3(0.f, 0.f, -1.f);
//Vec3 const Vec3::NEAR = Vec3(-1.f, 0.f, 0.f);
//Vec3 const Vec3::FAR = Vec3(1.f, 0.f, 0.f);
//Vec3 const Vec3::ONE = Vec3(1.f, 1.f, 1.f);

Vec3::Vec3(float initialX, float initialY, float initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{}

Vec3::Vec3(float* floats)
	: x(floats[0])
	, y(floats[1])
	, z(floats[2])
{}

// Accessors (const methods)
float Vec3::GetLength() const
{
	return sqrtf( this->x * this->x + this->y * this->y + this->z * this->z);
}

float Vec3::GetLengthXY() const
{
	return sqrtf( this->x * this->x + this->y * this->y);
}

float Vec3::GetLengthSquared() const
{
	return this->x * this->x + this->y * this->y + this->z * this->z;
}

float Vec3::GetLengthXYSquared() const
{
	return this->x * this->x + this->y * this->y;
}

float Vec3::GetAngleAboutZRadians() const
{
	return atan2f(this->y, this->x);
}

float Vec3::GetAngleAboutZDegrees() const
{
	return ConvertRadiansToDegrees(atan2f(this->y, this->x));
}

Vec3 const Vec3::GetRotatedAboutZRadians( float deltaRadians ) const
{
	float theta = GetAngleAboutZRadians();
	float r = GetLengthXY();
	theta += deltaRadians;
	return Vec3(r * cosf(theta), r * sinf(theta), this->z);
}

Vec3 const Vec3::GetRotatedAboutZDegrees(float deltaDegrees) const
{
	float theta = GetAngleAboutZDegrees();
	float r = GetLengthXY();
	theta += deltaDegrees;
	return Vec3(r * CosDegrees(theta), r * SinDegrees(theta), this->z);
}

Vec3 const Vec3::GetRotatedAboutZ90Degrees() const
{
	return Vec3(-y, x, z);
}

Vec3 const Vec3::Clamp(float maxLength) const
{
	if (GetLengthSquared() <= maxLength * maxLength)
	{
		return Vec3(x, y, z);

	}
	return GetNormalized() * maxLength;
}

Vec3 const Vec3::GetNewLength(float newLength) const
{
	return GetNormalized() * newLength;
}

Vec3 const Vec3::GetNormalized() const
{
	float length = GetLength();
	if (length > 0.f)
	{
		float scale = 1.f / length;
		return Vec3(x, y, z) * scale;

	}
	return Vec3::ZERO;

}

Vec2 const Vec3::GetXY() const
{
	return Vec2(this->x, this->y);
}

void Vec3::SetLength(float newLength)
{
	float length = GetLength();
	if (length == 0.f)
		return;
	float scale = newLength / length;
	this->x *= scale;
	this->y *= scale;
	this->z *= scale;
}

Vec3 const Vec3::Lerp(Vec3 a, Vec3 b, float t)
{
	return a * (1.f- t) + b * t; 
}

//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator + (const Vec3& vecToAdd) const
{
	return Vec3(this->x + vecToAdd.x, this->y + vecToAdd.y, this->z + vecToAdd.z);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-(const Vec3& vecToSubtract) const
{
	return Vec3(this->x - vecToSubtract.x, this->y - vecToSubtract.y, this->z - vecToSubtract.z);
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-() const
{
	return Vec3(-this->x, -this->y, -this->z);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*(float uniformScale) const
{
	return Vec3(this->x * uniformScale, this->y * uniformScale, this->z * uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*(const Vec3& vecToMultiply) const
{
	return Vec3(this->x * vecToMultiply.x, this->y * vecToMultiply.y, this->z * vecToMultiply.z);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator/(float inverseScale) const
{
	return Vec3(this->x / inverseScale, this->y / inverseScale, this->z / inverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator+=(const Vec3& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator-=(const Vec3& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator*=(const float uniformScale)
{
	this->x *= uniformScale;
	this->y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator/=(const float uniformDivisor)
{
	this->x /= uniformDivisor;
	this->y /= uniformDivisor;
	this->z /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator=(const Vec3& copyFrom)
{
	this->x = copyFrom.x;
	this->y = copyFrom.y;
	this->z = copyFrom.z;
}

void Vec3::SetFromText(char const* text, char delimToSplitOn)
{
	Strings values = SplitStringOnDelimiter(text, delimToSplitOn, true);
	GUARANTEE_OR_DIE(values.size() == 3, "Parsing Vec3 failed to get x y z values");
	this->x = (float)atof(values[0].c_str());
	this->y = (float)atof(values[1].c_str());
	this->z = (float)atof(values[2].c_str());
}

void Vec3::SetFromFloats(float const* floats)
{
	this->x = floats[0];
	this->y = floats[1];
	this->z = floats[2];
}

void Vec3::GetFromFloats(float* out_floats)
{
	out_floats[0] = x;
	out_floats[1] = y;
	out_floats[2] = z;

}

//-----------------------------------------------------------------------------------------------
const Vec3 operator*(float uniformScale, const Vec3& vecToScale)
{
	return Vec3(vecToScale.x * uniformScale, vecToScale.y * uniformScale, vecToScale.z * uniformScale);
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator==(const Vec3& compare) const
{
	return this->x == compare.x && this->y == compare.y && this->z == compare.z;
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator!=(const Vec3& compare) const
{
	return this->x != compare.x || this->y != compare.y || this->z != compare.z;
}