#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Vec3.hpp"

Vec4::Vec4(float initialX, float initialY, float initialZ, float initialW)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
	, w(initialW)
{
}

//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator + (const Vec4& vecToAdd) const
{
	return Vec4(this->x + vecToAdd.x, this->y + vecToAdd.y, this->z + vecToAdd.z, this->w + vecToAdd.w);
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator-(const Vec4& vecToSubtract) const
{
	return Vec4(this->x - vecToSubtract.x, this->y - vecToSubtract.y, this->z - vecToSubtract.z, this->w - vecToSubtract.w);
}


//------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator-() const
{
	return Vec4(-this->x, -this->y, -this->z, -this->w);
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator*(float uniformScale) const
{
	return Vec4(this->x * uniformScale, this->y * uniformScale, this->z * uniformScale, this->w*uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator*(const Vec4& vecToMultiply) const
{
	return Vec4(this->x * vecToMultiply.x, this->y * vecToMultiply.y, this->z * vecToMultiply.z, this->w * vecToMultiply.w);
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator/(float inverseScale) const
{
	float oneOverScale = 1.f / inverseScale;
	return Vec4(this->x * oneOverScale, this->y * oneOverScale, this->z * oneOverScale, this->w * oneOverScale);
}

Vec3 Vec4::GetXYZ() const
{
	return Vec3(x, y, z);
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator+=(const Vec4& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
	w += vecToAdd.w;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator-=(const Vec4& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
	w -= vecToSubtract.w;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator/=(const float uniformDivisor)
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
	w /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator=(const Vec4& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}