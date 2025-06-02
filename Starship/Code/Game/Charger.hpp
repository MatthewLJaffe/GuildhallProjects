#pragma once
#include "Game/Entity.hpp"

class Charger : public Entity
{
public:
	Charger(Game* game, const Vec2& pos);
	void Update(float deltaSeconds) override;
	virtual void Die() override;
private:
	void CheckForCollision();
	Vec2 GetMovementDir();
	void InitializeLocalVerts() override;
	void FireAtPlayer(float deltaSeconds);
	Vec2 m_randomMoveDir;
	bool m_chasing = true;
	float m_seperateWeight = .4f;
	float m_attackRadius = 50.f;
	float m_visionRadius = 100.f;
	float m_maxSpeed = 6.f;
	float m_acceleration = 5.f;
	float m_shootCooldown = .8f;
	float m_currShootCooldown = .8f;
	float m_preferedDistanceFromOffScreen = 20.f;
};
/*
* Patrol move in random direction separation
* When player in vision radius accelerate towards player and shoot at player continuously
*/