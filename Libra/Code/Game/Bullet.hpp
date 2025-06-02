#pragma once
#include "Game/Entity.hpp"
#include "Game/Tile.hpp"
class SpriteAnimDefinition;

class Bullet : public Entity 
{
public:
	Bullet(Vec2 const& startPos, float startOrientation, EntityType entityType);
	void Update(float deltaSeconds) override;
	void Render() const override;
	void Die() override;
	void ReflectBullet(Vec2 const& surfaceNormal, bool ariesShield = false);
	Texture* m_texture;
	SpriteAnimDefinition* m_animDefinition = nullptr;
	float m_maxSpeed = 5.f;
	float m_currLiveTime = 0.f;
	float m_maxLifetime = 1.f;
	float m_bulletDamage = 1.f;
	float m_bounceRandomness = .4f;
	float m_rotationSpeed = 0.f;
protected:
	void InitializeLocalVerts() override;
private:
	Vec2 CheckForRebound();
	bool IsInReboundingTile(Vec2 const& point);
};