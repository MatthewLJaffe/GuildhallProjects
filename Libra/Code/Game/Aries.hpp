#pragma once
#include "Game/Entity.hpp"

class Aries : public Entity
{
public:
	Aries(Vec2 const& startPos, float startOrientation);
	void Update(float deltaSeconds) override;
	void Render() const override;
	void HandleIncomingBullet(Bullet* bullet) override;
	void RenderDebug() const override;
	void TakeDamage(float damageAmount) override;
	void Die() override;
	Texture* m_texture;
protected:
	void InitializeLocalVerts() override;
};