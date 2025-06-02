#pragma once
#include "Game/Entity.hpp"

class DamageText : public Entity
{
public:
	DamageText(Game* game, Vec3 const& startPos, int damage);
	void Update(float deltaSeconds);
	void Render() const;
	float m_liveTime = 1.f;
	float m_currentLiveTime = 0.f;
	Vec3 m_startPos;
	Vec3 m_endOffset = Vec3(0.f, 0.f, 1.f);
	Rgba8 m_startColor = Rgba8(255, 100, 100, 255);
	Rgba8 m_endColor = Rgba8(255, 100, 100, 0);

};