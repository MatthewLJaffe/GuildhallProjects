#pragma once
#include "Game/Entity.hpp"
#include "Game/Map.hpp"

class Scorpio : public Entity 
{
public:
	Scorpio(Vec2 const& startPos, float startOrientation);
	void Update(float deltaSeconds) override;
	void Render() const override;
	void TakeDamage(float damageAmount) override;
	void Die() override;
	float m_maxRotateSpeed = 45.f;
	Texture* m_texture;
	Texture* m_turretTexture;
	std::vector<Vertex_PCU> m_localTurretVerts;
protected:
	void InitializeLocalVerts() override;
private:
	bool CanShootAtPlayer();
	float m_visionRadius = 10.f;
	bool m_raycastHit;
	float m_currentShootCooldown = 0.f;
	float m_shootCooldown = .5f;
	RaycastResult2D m_turretRaycastResult;
};