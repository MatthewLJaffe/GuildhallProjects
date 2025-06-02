#pragma once
#include "Game/Entity.hpp"

class Shotgunner : public Entity
{
public:
	Shotgunner(Game* game, const Vec2& pos);
	void Update(float deltaSeconds) override;
	virtual void Die() override;
private:
	void CheckForCollision();
	void InitializeLocalVerts() override;
	Vec2 GetMovement();
	void FireAtPlayer(float deltaSeconds);
	Vec2 m_randomMoveDir;
	bool m_chasing = true;
	float m_acceleration = 5.f;
	float m_seperateWeight = .2f;
	float m_visionRadius = 10.f;
	float m_maxSpeed = 6.f;
	float m_shootCooldown = 4.f;
	float m_currShootCooldown = 2.f;
	float m_preferedDistanceFromOffScreen = 20.f;
};