#pragma once
#include <math.h>

struct Vec2;
struct Vec3;

//-----------------------------------------------------------------------------------------------
struct Vec4
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.0f;
	float w = 0.0f;

public:
	// Construction/Destruction
	Vec4() = default;												// default constructor (do nothing)

	explicit Vec4( float initialX, float initialY, float initialZ, float initialW );		// explicit constructor (from x, y, z, w)
	bool		operator==(Vec4 const& compare) const;			//Vec4 == Vec4
	bool		operator!=(Vec4 const& compare) const;			//Vec4 != Vec4
	Vec4 const	operator+(Vec4 const& vecToAdd) const;			//Vec4 + Vec4
	Vec4 const	operator-(const Vec4& vecToSubtract) const;		// Vec4 - Vec4
	const Vec4	operator-() const;								// -Vec4, i.e. "unary negation"
	Vec4 const	operator*(float uniformScale) const;			// Vec4 * Vec4
	const Vec4	operator*(const Vec4& vecToMultiply) const;		// Vec4 * Vec4
	Vec4 const	operator/(float inverseScale) const;			// Vec4 / Vec4
	Vec3 GetXYZ() const;

	// Operators (self-mutating / non-const)
	void		operator+=(const Vec4& vecToAdd);			// Vec4 += Vec4
	void		operator-=(const Vec4& vecToSubtract);		// Vec4 -= Vec4
	void		operator*=(const float uniformScale);		// Vec4 *= float
	void		operator/=(const float uniformDivisor);		// Vec4 /= float
	void		operator=(const Vec4& copyFrom);			// Vec4 = Vec4
};