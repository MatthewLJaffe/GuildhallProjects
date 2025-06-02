#pragma once
#include "Game/Actor.hpp"

class Thrall : public Actor 
{
public:
	Thrall(SpawnInfo const& spawnInfo, ActorUID actorUID);
	void Update(float deltaSeconds) override;
	virtual void Die() override;
	//Vec3 ComputeChaseDirection(float deltaSeconds, float weight);
	//Vec3 ComputeSeperationDirection(float weight);
	//float m_acceleration = 20.f;
	//float m_maxSpeed = 10.5f;
	bool m_exploding = false;
	float m_maxExplodeDistance = 4.f;
	float m_maxChaseDistance = 3.5f;
	Timer m_explodeTimer;
	float m_explodeMidwayPoint = .5f;
};