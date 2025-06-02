#pragma once
#include <math.h>

struct Vec2;

//-----------------------------------------------------------------------------------------------
struct Vec3
{
public:
	static const Vec3 ZERO;
	//static const Vec3 LEFT;
	//static const Vec3 RIGHT;
	//static const Vec3 UP;
	//static const Vec3 DOWN;
	//static const Vec3 NEAR;
	//static const Vec3 FAR;
	//static const Vec3 ONE;

public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.0f;

public:
	// Construction/Destruction
	Vec3() = default;												// default constructor (do nothing)
	explicit Vec3( float initialX, float initialY, float initialZ );		// explicit constructor (from x, y, z)
	Vec3(float* floats);

	// Accessors (const methods)
	float		GetLength() const;
	float		GetLengthXY() const;
	float		GetLengthSquared() const;
	float		GetLengthXYSquared() const;
	float		GetAngleAboutZRadians() const;
	float		GetAngleAboutZDegrees() const;
	Vec3 const	GetRotatedAboutZRadians( float deltaRadians ) const;
	Vec3 const	GetRotatedAboutZDegrees( float deltaDegrees ) const;
	Vec3 const	GetRotatedAboutZ90Degrees() const;
	Vec3 const	Clamp( float maxLength ) const;
	Vec3 const	GetNewLength(float newLength) const;
	Vec3 const	GetNormalized() const;
	Vec2 const	GetXY() const;
	void		SetLength(float newLength);
	static Vec3 const	Lerp(Vec3 a, Vec3 b, float t);


	// Operators (const)
	bool		operator==(Vec3 const& compare) const;			//vec3 == vec3
	bool		operator!=(Vec3 const& compare) const;			//vec3 != vec3
	Vec3 const	operator+(Vec3 const& vecToAdd) const;			//vec3 + vec3
	Vec3 const	operator-(const Vec3& vecToSubtract) const;		// vec3 - vec3
	const Vec3	operator-() const;								// -vec3, i.e. "unary negation"
	Vec3 const	operator*(float uniformScale) const;			// vec3 * vec3
	const Vec3	operator*(const Vec3& vecToMultiply) const;		// vec3 * vec3
	Vec3 const	operator/(float inverseScale) const;			// vec3 / vec3

	// Operators (self-mutating / non-const)
	void		operator+=(const Vec3& vecToAdd);			// vec3 += vec3
	void		operator-=(const Vec3& vecToSubtract);		// vec3 -= vec3
	void		operator*=(const float uniformScale);		// vec3 *= float
	void		operator/=(const float uniformDivisor);		// vec3 /= float
	void		operator=(const Vec3& copyFrom);			// vec3 = vec3

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec3::
	friend const Vec3 operator*(float uniformScale, const Vec3& vecToScale);	// float * vec3
	void SetFromText(char const* text, char delimToSplitOn = ',');
	void SetFromFloats(float const* floats);
	void GetFromFloats(float* out_floats);
};