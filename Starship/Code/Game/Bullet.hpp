#pragma once
#include "Game/Entity.hpp"

class Bullet : public Entity
{
public:
	Bullet(Game* game, Vec2 startPos, float rotation);
	Bullet(Game* game, Vec2 startPos, float rotation, float speed, bool targetPlayer, float scale, Rgba8 centerColor, Rgba8 tailColor);
	void Update(float deltaSeconds) override;
	void Render() const override;
	void Die() override;
	bool m_targetPlayer = false;
private:
	void DieWithoutParticles();
	float m_scale = 1.f;
	Rgba8 m_centerColor;
	Rgba8 m_tailColor;
	float m_currBulletLiveTime = 0.f;
	float m_bulletLiveTime = BULLET_LIFETIME_SECONDS;
	float m_bulletSpeed = BULLET_SPEED;
protected:
	void InitializeLocalVerts() override;
};