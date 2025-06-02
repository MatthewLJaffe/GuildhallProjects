#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
class RandomNumberGenerator;

constexpr int NUM_ASTEROID_SIDES = 16;
constexpr int NUM_ASTEROID_TRIS = NUM_ASTEROID_SIDES;
constexpr int NUM_ASTEROID_VERTS = 3 * NUM_ASTEROID_TRIS;
constexpr float ASTEROID_MAX_ROTATION_SPEED = 100.0f;


class Asteroid : public Entity
{
public:
	Asteroid(Game* owner, const Vec2& pos, RandomNumberGenerator* randGen);
	~Asteroid();

	void Update(float deltaSeconds) override;
	void Render() const override;
	void Die() override;

protected:
	void InitializeLocalVerts() override;

private:
	RandomNumberGenerator* m_randomNumberGenerator;

};