#pragma once
#include "Game/Entity.hpp"

class Player : public Entity
{
public:
	Player(const Vec2& startPos, float startOrientation);
	~Player() override;
	void Update(float deltaSeconds) override;
	void HandleControlsController(float deltaSeconds);
	void HandleControlsKeyboard(float deltaSeconds);
	void Render() const override;
	void RenderDebug() const override;
	void Die() override;
	void Respawn();
	void TakeDamage(float amount) override;
protected:
	void InitializeLocalVerts() override;
private:
	void UpdateMuzzleFlashes();
	void HandleBulletShoot(float deltaSeconds);
	void HandleFlameShoot(float deltaSeconds);
	std::vector<Vertex_PCU> m_localTurretVerts;
	float m_turretOrientation;
	float m_turretTargetOrientation;
	float m_targetOrientation;
	Texture* m_tankTexture;
	Texture* m_turretTexture;
	float m_bulletShootCooldown = .1f;
	float m_flameShootCooldown = .05f;
	float m_currShootCooldown = 0.f;
};
