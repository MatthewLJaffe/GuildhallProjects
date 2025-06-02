#pragma once
#include "Game/Entity.hpp"

class Leo : public Entity
{
public:
	Leo(Vec2 const& startPos, float startOrientation);
	void Update(float deltaSeconds) override;
	void Render() const override;
	void RenderDebug() const override;
	void TakeDamage(float damageAmount) override;
	void Die() override;
	Texture* m_texture;
protected:
	void InitializeLocalVerts() override;
private:
	void HandleShooting(float deltaSeconds);
	float m_shootCooldown = 1.f;
	float m_currShootCooldown = 0.f;
};