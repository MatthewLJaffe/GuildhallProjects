#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/FloatCurve.hpp"

class Player : public Entity 
{
public:
	Player(GameState* gameState, EntityType entityType, Vec2 const& startPos);
	void Update(float deltaSeconds) override;
	void Render() override;
	void UpdatePhysics(float deltaSeconds) override;
	void UpdateDash(float deltaSeconds);
	void LoseLife();
	int GetLives();
	float m_radius = 4.f;
	int m_maxLives = 8;
	int m_lives = m_maxLives;
private:
	void HandlePlayerControls();
	void HandleControlsController(XboxController const& controller);
	void HandleControlsKeyboard();
	void SpawnPlayerParticle(Vec2 const& dir);
	void UpdateInvincibility();
	float m_moveSpeed;
	bool m_isDashing = false;
	float m_currentDashTime = 0.f;
	float m_totalDashTime = .25f;
	float m_dashInvincibilityTime = .32f;
	float m_dashTopSpeed = 450.f;
	float m_moveTurnSpeed = 720.f;
	Vec2 m_dashDir = Vec2::ZERO;
	Timer m_spawnParticleTimer;
	FloatCurve const* m_dashCurve = nullptr;
	bool m_moving = false;
	Timer m_invincibilityTimer;
	Timer m_dashInvincibilityTimer;
	bool m_isInvincible = false;
};
