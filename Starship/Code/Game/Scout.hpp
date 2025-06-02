#pragma once
#include "Game/Entity.hpp"

class Scout : public Entity
{
public:
	Scout(Game* game, const Vec2& pos);
	void Update(float deltaSeconds) override;
	virtual void Die() override;
private:
	void CheckForCollision();
	void InitializeLocalVerts() override;
	Vec2 GetMovement();
	void FireAtPlayer(float deltaSeconds);
	float m_isFleeing = false;
	float m_seperateWeight = .2f;
	float m_visionRadius = 10.f;
	float m_maxSpeed = 6.f;
	float m_shootCooldown = 2.5f;
	float m_currShootCooldown = 2.5f;
	float m_preferedDistanceFromOffScreen = 20.f;
};