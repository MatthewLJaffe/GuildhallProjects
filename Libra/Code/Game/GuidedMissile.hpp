#pragma once
#include "Game/Entity.hpp"

class GuidedMissile : public Entity
{
public:
	GuidedMissile(Vec2 const& startPos, float startOrientation, EntityFaction factionType);
	void Update(float deltaSeconds) override;
	void Render() const override;
	void Die() override;
	Texture* m_texture;
	Entity* m_target;
	float m_maxSpeed = .75f;
	float m_liveTime;
	float m_missileDamage = 1.f;
protected:
	void InitializeLocalVerts() override;
	float m_turnDegreesPerSecond = 30.f;
};