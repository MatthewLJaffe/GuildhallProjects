#pragma once
#include <math.h>

struct Vec3;
struct IntVec2;

//-----------------------------------------------------------------------------------------------
struct Vec2
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
public:
	static const Vec2 ZERO;
	static const Vec2 ONE;
	static const Vec2 UP;
	static const Vec2 DOWN;
	static const Vec2 LEFT;
	static const Vec2 RIGHT;
	static const Vec2 TOP_RIGHT;
	static const Vec2 TOP_LEFT;
	static const Vec2 BOTTOM_RIGHT;
	static const Vec2 BOTTOM_LEFT;

public:
	// Construction/Destruction
	~Vec2() {}												// destructor (do nothing)
	Vec2() {}												// default constructor (do nothing)
	Vec2( const Vec2& copyFrom );							// copy constructor (from another vec2)
	explicit Vec2( float initialX, float initialY );		// explicit constructor (from x, y)

	// Static methods (e.g. creation funcitons)
	static Vec2 const	MakeFromPolarRadians( float orientationRadians, float length = 1.f );
	static Vec2 const	MakeFromPolarDegrees(float orientationDegrees, float length = 1.f);
	static Vec2 const	Lerp(Vec2 a, Vec2 b, float t);


	// Accessors (const methods)
	float		GetLength() const;
	float		GetLengthSquared() const;
	float		GetOrientationRadians() const;
	float		GetOrientationDegrees() const;
	Vec2 const	GetRotated90Degrees() const;
	Vec2 const	GetRotatedMinus90Degrees() const;
	Vec2 const	GetRotatedRadians(float deltaRadians) const;
	Vec2 const	GetRotatedDegrees(float deltaDegrees) const;
	Vec2 const	Clamp( float maxLength ) const;
	Vec2 const	GetNewLength(float newLength) const;
	Vec2 const	GetNormalized() const;
	Vec2 const	GetReflected( Vec2 const& impactSurfaceNormal ) const;
	Vec3 const GetXYZ() const;
	IntVec2 const GetIntVec2();

	// Mutators (non-const methods)
	void	SetOrientationRadians( float newOrientationRadians );
	void	SetOrientationDegrees(float newOrientationDegrees);
	void	SetPolarRadians( float newOrientationRadians, float newLength );
	void	SetPolarDegrees(float newOrientationDegrees, float newLength);
	void	Rotate90Degrees();
	void	RotateMinus90Degrees();
	void	RotateRadians( float deltaRadians );
	void	RotateDegrees(float deltaDegrees);
	void	SetLength( float newLength );
	void	ClampLength( float maxLength );
	void	Normalize();
	float	NormalizeAndGetPreviousLength();
	void	Reflect(Vec2 const& impactSurfaceNormal, float elasticity = 1.f);

	// Operators (const)
	bool		operator==( const Vec2& compare ) const;		// vec2 == vec2
	bool		operator!=( const Vec2& compare ) const;		// vec2 != vec2
	const Vec2	operator+( const Vec2& vecToAdd ) const;		// vec2 + vec2
	const Vec2	operator-( const Vec2& vecToSubtract ) const;	// vec2 - vec2
	const Vec2	operator-() const;								// -vec2, i.e. "unary negation"
	const Vec2	operator*( float uniformScale ) const;			// vec2 * float
	const Vec2	operator*( const Vec2& vecToMultiply ) const;	// vec2 * vec2
	const Vec2	operator/( float inverseScale ) const;			// vec2 / float

	// Operators (self-mutating / non-const)
	void		operator+=( const Vec2& vecToAdd );				// vec2 += vec2
	void		operator-=( const Vec2& vecToSubtract );		// vec2 -= vec2
	void		operator*=( const float uniformScale );			// vec2 *= float
	void		operator/=( const float uniformDivisor );		// vec2 /= float
	void		operator=( const Vec2& copyFrom );				// vec2 = vec2



	// Standalone "friend" functions that are conceptually, but not actually, part of Vec2::
	friend const Vec2 operator*( float uniformScale, const Vec2& vecToScale );	// float * vec2

	void SetFromText(char const* text, char delim = ',');
};


