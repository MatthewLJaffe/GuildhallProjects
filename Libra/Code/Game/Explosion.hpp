#pragma once
#include "Game/Entity.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

class Explosion : public Entity
{
public:
	Explosion(Vec2 const& startPos, float orientationDegrees, SpriteAnimDefinition const& animDefinition, bool isMuzzleFlash);
	void Update(float deltaSeconds) override;
	void Render() const override;
	float m_scale = 1.f;
	float m_duration = 0.f;
protected:
	void InitializeLocalVerts() override;
private:
	float m_currTime = 0.f;
	SpriteAnimDefinition m_animDefinition;
	AABB2 m_explosionBounds;

};