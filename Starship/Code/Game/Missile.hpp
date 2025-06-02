#pragma once
#include "Game/Entity.hpp"

class Missile : public Entity
{
public:
	Missile(Game* game, Vec2 startPos, Entity* target, float travelTime);
	void Update(float deltaSeconds) override;
	void Render() const override;
	void Die() override;
	Entity* m_target;
protected:
	void InitializeLocalVerts() override;
private:
	float m_currTime = 0.f;
	float m_arriveTime = 0.f;
	Vec2 m_startPos;
	float m_controlPointDir = 1.f;
	int m_currTrailSegments = 1;
	float m_trailOpacityDecreaseRate = 160.f;
	float m_currentOpacityDecrease = 0.f;
	void UpdateTrailSegmentPosition(float normalizedTime);
	void UpdateTrailOpacity(float deltaSeconds);
	bool m_trailInitialized = false;
};