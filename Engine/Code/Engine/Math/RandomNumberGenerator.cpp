#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"

int RandomNumberGenerator::RollRandomIntLessThan(int maxNotInclusive)
{
	return rand() % maxNotInclusive;
}

int RandomNumberGenerator::RollRandomIntInRange(int minInclusive, int maxInclusive)
{
	return (rand() % (maxInclusive + 1 - minInclusive)) + minInclusive;
}

float RandomNumberGenerator::RollRandomFloatZeroToOne()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float RandomNumberGenerator::RollRandomFloatInRange(float minInclusive, float maxInclusive)
{
	float random = RollRandomFloatZeroToOne();
	return Lerp(minInclusive, maxInclusive, random);
}

float RandomNumberGenerator::RollRandomFloatInRange(FloatRange range)
{
	float random = RollRandomFloatZeroToOne();
	return Lerp(range.m_min, range.m_max, random);
}

Vec2 RandomNumberGenerator::RollRandomNormalizedVec2()
{
	return Vec2(RollRandomFloatInRange(-1.f, 1.f), RollRandomFloatInRange(-1.f, 1.f)).GetNormalized();
}

Vec3 RandomNumberGenerator::RollRandomNormalizedVec3()
{
	return Vec3(RollRandomFloatInRange(-1.f, 1.f), RollRandomFloatInRange(-1.f, 1.f), RollRandomFloatInRange(-1.f, 1.f)).GetNormalized();
}

Vec2 RandomNumberGenerator::RollRandomVec2InRange(Vec2 const& mins, Vec2 const& maxs)
{
	Vec2 randomVec2;
	randomVec2.x = RollRandomFloatInRange(mins.x, maxs.x);
	randomVec2.y = RollRandomFloatInRange(mins.y, maxs.y);
	return randomVec2;
}