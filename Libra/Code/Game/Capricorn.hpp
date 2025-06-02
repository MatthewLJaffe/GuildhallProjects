#pragma once
#include "Game/Entity.hpp"

class Capricorn : public Entity
{
public:
	Capricorn(Vec2 const& startPos, float startOrientation);
	void Update(float deltaSeconds) override;
	void Render() const override;
	void RenderDebug() const override;
	void Die() override;
	Texture* m_texture;
protected:
	void InitializeLocalVerts() override;
	void HandleShooting(float deltaSeconds);
	float m_currShootCooldown = 0.f;
	float m_shootCooldown = 1.5f;
};