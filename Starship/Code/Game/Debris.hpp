#pragma once
#include "Game/Entity.hpp"

class RandomNumberGenerator;
constexpr float DEBRIS_MAX_ROTATION_SPEED = 200.0f;
constexpr float DEBRIS_MAX_COSMETIC_RADIS = 2.f;
constexpr float DEBRIS_MAX_PHYSICS_RADIUS = 1.f;
constexpr float DEBRIS_LIVE_TIME = 2.f;

constexpr int NUM_DEBRIS_SIDES = 16;
constexpr int NUM_DEBRIS_TRIS = NUM_DEBRIS_SIDES;
constexpr int NUM_DEBRIS_VERTS = 3 * NUM_DEBRIS_TRIS;


class Debris : public Entity
{
public:
	Debris(Game* owner, const Vec2& pos, const Vec2& startingVelocity, float minSize, float maxSize, const Rgba8& color, RandomNumberGenerator* randGen);
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;
private:
	Rgba8 m_startColor;
	float m_age = 0.f;
	RandomNumberGenerator* m_randomNumberGenerator;
protected:
	void InitializeLocalVerts() override;
};