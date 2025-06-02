#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"

Vec2 const Vec2::ZERO = Vec2(0.f, 0.f);
Vec2 const Vec2::ONE = Vec2(1.f, 1.f);
Vec2 const Vec2::UP = Vec2(0.f, 1.f);
Vec2 const Vec2::DOWN = Vec2(0.f, -1.f);
Vec2 const Vec2::LEFT = Vec2(-1.f, 0.f);
Vec2 const Vec2::RIGHT = Vec2(1.f, 0.f);
Vec2 const Vec2::TOP_LEFT = Vec2(-0.7071f, 0.7071f);
Vec2 const Vec2::TOP_RIGHT = Vec2(0.7071f, 0.7071f);
Vec2 const Vec2::BOTTOM_LEFT = Vec2(-0.7071f, -0.7071f);
Vec2 const Vec2::BOTTOM_RIGHT = Vec2(0.7071f, -0.7071f);

//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
	: x( copy.x )
	, y( copy.y )
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}

Vec2 const Vec2::MakeFromPolarRadians(float orientationRadians, float length)
{
	return Vec2(length * cosf(orientationRadians),length * sinf(orientationRadians));
}

Vec2 const Vec2::MakeFromPolarDegrees(float orientationDegrees, float length)
{
	return Vec2(length * CosDegrees(orientationDegrees), length * SinDegrees(orientationDegrees));
}

Vec2 const Vec2::Lerp(Vec2 a, Vec2 b, float t)
{
	return a*(1.f - t) + b*t;
}

// Accessors (const methods)
float Vec2::GetLength() const
{
	return sqrtf(this->x * this->x + this->y * this->y);
}

float Vec2::GetLengthSquared() const
{
	return this->x * this->x + this->y * this->y;
}

float Vec2::GetOrientationRadians() const
{
	return atan2f(this->y, this->x );
}

float Vec2::GetOrientationDegrees() const
{
	return ConvertRadiansToDegrees( atan2f(this->y, this->x ) );
}

Vec2 const Vec2::GetRotated90Degrees() const
{
	return Vec2(-y, x);
}

Vec2 const Vec2::GetRotatedMinus90Degrees() const
{
	return Vec2(y, -x);
}

Vec2 const Vec2::GetRotatedRadians(float deltaRadians) const
{
	float theta = this->GetOrientationRadians();
	float r = this->GetLength();
	theta += deltaRadians;
	return Vec2(r * cosf(theta), r * sinf(theta));
}

Vec2 const Vec2::GetRotatedDegrees(float deltaDegrees) const
{
	float theta = this->GetOrientationDegrees();
	float r = this->GetLength();
	theta += deltaDegrees;
	return Vec2(r * CosDegrees(theta), r * SinDegrees(theta));
}

Vec2 const Vec2::Clamp(float maxLength) const 
{
	if (this->GetLength() <= maxLength)
		return Vec2(this->x, this->y);
	return this->GetNormalized() * maxLength;
}

Vec2 const Vec2::GetNewLength(float newLength) const
{
	return this->GetNormalized() * newLength;
}

Vec2 const Vec2::GetNormalized() const
{
	float length = this->GetLength();
	if (length == 0.f)
		return Vec2(0.f, 0.f);
	float inverseLength = 1.f / length;
	return Vec2(x * inverseLength, y * inverseLength);
}

Vec2 const Vec2::GetReflected(Vec2 const& impactSurfaceNormal) const
{
	Vec2 iBasis = -impactSurfaceNormal;
	Vec2 jBasis = iBasis.GetRotated90Degrees();
	float iComponent = DotProduct2D(iBasis, *this);
	float jComponent = DotProduct2D(jBasis, *this);
	return -iComponent * iBasis + jComponent * jBasis;
}

Vec3 const Vec2::GetXYZ() const
{
	return Vec3(this->x, this->y, 0);
}

IntVec2 const Vec2::GetIntVec2()
{
	return IntVec2((int)x, (int)y);
}

// Mutators (non-const methods)
void Vec2::SetOrientationRadians(float newOrientationRadians)
{
	float theta = newOrientationRadians;
	float r = GetLength();
	this->x = r * cosf(theta);
	this->y = r * sinf(theta);
}

void Vec2::SetOrientationDegrees(float newOrientationDegrees)
{
	float theta = newOrientationDegrees;
	float r = GetLength();
	this->x = r * CosDegrees(theta);
	this->y = r * SinDegrees(theta);
}

void Vec2::SetPolarRadians(float newOrientationRadians, float newLength)
{
	this->x = newLength * cosf( newOrientationRadians );
	this->y = newLength * sinf( newOrientationRadians );
}

void Vec2::SetPolarDegrees(float newOrientationDegrees, float newLength)
{
	this->x = newLength * CosDegrees(newOrientationDegrees);
	this->y = newLength * SinDegrees(newOrientationDegrees);
}

void Vec2::Rotate90Degrees()
{
	float tempX = x;
	this->x = -y;
	this->y = tempX;
}

void Vec2::RotateMinus90Degrees()
{
	float tempX = x;
	this->x = y;
	this->y = -tempX;
}
void Vec2::RotateRadians(float deltaRadians)
{
	float theta = this->GetOrientationRadians();
	float r = this->GetLength();
	theta += deltaRadians;
	this->x = r * cosf(theta);
	this->y = r * sinf(theta);
}

void Vec2::RotateDegrees(float deltaDegrees)
{
	float theta = this->GetOrientationDegrees();
	float r = this->GetLength();
	theta += deltaDegrees;
	this->x = r * CosDegrees(theta);
	this->y = r * SinDegrees(theta);
}

void Vec2::SetLength(float newLength)
{
	float length = GetLength();
	if (length == 0.f)
		return;
	float scale = newLength / length;
	this->x *= scale;
	this->y *= scale;
}

void Vec2::ClampLength(float maxLength)
{
	if (this->GetLength() <= maxLength) return;
	this->Normalize();
	this->x *= maxLength;
	this->y *= maxLength;
}

void Vec2::Normalize()
{
	float length = GetLength();
	if (length == 0) return;
	float scale = 1.f / length;
	this->x *= scale;
	this->y *= scale;
}

float Vec2::NormalizeAndGetPreviousLength()
{
	float length = GetLength();
	this->x /= length;
	this->y /= length;
	return length;
}

void Vec2::Reflect(Vec2 const& impactSurfaceNormal, float elasticity)
{
	Vec2 iBasis = -impactSurfaceNormal;
	Vec2 jBasis = iBasis.GetRotated90Degrees();
	float iComponent = DotProduct2D(iBasis, *this);
	float jComponent = DotProduct2D(jBasis, *this);
	*this = (-elasticity * iComponent * iBasis) + (jComponent * jBasis);
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator + ( const Vec2& vecToAdd ) const
{
	return Vec2( this->x + vecToAdd.x, this->y + vecToAdd.y );
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-( const Vec2& vecToSubtract ) const
{
	return Vec2( this->x - vecToSubtract.x, this->y - vecToSubtract.y );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2( -this->x, -this->y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2( this->x * uniformScale, this->y * uniformScale );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2( this->x * vecToMultiply.x, this->y	 * vecToMultiply.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	return Vec2( this->x / inverseScale, this->y / inverseScale );
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	this->x *= uniformScale;
	this->y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	this->x /= uniformDivisor;
	this->y /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	this->x = copyFrom.x;
	this->y = copyFrom.y;
}

void Vec2::SetFromText(char const* text, char delim)
{
	Strings values = SplitStringOnDelimiter(text, delim, true);
	GUARANTEE_OR_DIE(values.size() == 2, "Parsing Vec2 failed to get x and y values");
	this->x = (float)atof(values[0].c_str());
	this->y = (float)atof(values[1].c_str());
}



//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2(vecToScale.x * uniformScale, vecToScale.y * uniformScale);
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	return this->x == compare.x && this->y == compare.y;
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	return this->x != compare.x || this->y != compare.y;
}