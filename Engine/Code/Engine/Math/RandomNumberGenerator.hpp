#pragma once
#include <cstdlib>
#include <math.h>
#include "Engine/Math/Vec2.hpp"

struct FloatRange;

class RandomNumberGenerator
{
public:
	int RollRandomIntLessThan( int maxNotInclusive );
	int RollRandomIntInRange( int minInclusive, int maxInclusive );
	float RollRandomFloatZeroToOne();
	float RollRandomFloatInRange( float minInclusive, float maxInclusive );
	float RollRandomFloatInRange(FloatRange range);
	Vec2 RollRandomNormalizedVec2();
	Vec3 RollRandomNormalizedVec3();
	Vec2 RollRandomVec2InRange(Vec2 const& mins, Vec2 const& maxs);
private:
//	unsigned int m_seed = 0; //we will use these later on...
//	int m_position = 0; // ...when we replace rand() with noise
};