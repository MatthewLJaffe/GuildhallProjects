#pragma once
#include "Game/Entity.hpp"
#include "Game/EnemyProjectile.hpp"
#include "Engine/Math/MathUtils.hpp"


class Boss1 : public Entity
{
public:
	Boss1(GameState* gameState, EntityType entityType, Vec2 const& startPos);
	void Update(float deltaSeconds) override;
	void AttackInSpiral(float attackOrientation, float rotationDuration, float timePerAttack, int numAttacks);
	void Attack(int numberOfProjectilesInAttack, int numberOfAttacks, float timePerAttack);
	void StartRythmicRotation(int numberOfBeats, float secondsPerBeat);
	void StartRandomMovement(float moveTime);
	void StartChasePlayer(float moveTime);
	void MoveTowardsPlayer(float moveTime);
	void UpdateRandomMove(float deltaSeconds);
	void AddArm();
	void KillBoss();
private:
	ProjectileConfig m_spikeyBallConfig;
	//Spiral attack
	void UpdateSpiralAttack();
	void UpdateAttack();
	void UpdateRythmicRotation(float deltaSeconds);
	bool m_attackInSpiral = false;
	bool m_hornAttack = false;
	float m_startRotation = 0.f;
	float m_goalRotation = 0.f;
	int m_numHornAttacks = 0;
	float m_slowTurnRotationSpeed = 90.f;
	Vec2 m_fastTurnAngleRange = Vec2(90.f, 270.f);
	int m_rotationsLeft = 0;
	int m_rotationsComplete = 0;
	int m_projectilesPerAttack = 2;
	float m_moveSpeed = 60.f;
	Vec2 m_moveTarget;
	Timer m_randomMoveTimer;
	Timer m_moveTowardsPlayerTimer;
	Timer m_rotationTimer;
	Timer m_hornAttackTimer;
	Timer m_rythmicRotationTimer;
	std::vector<Entity*> m_arms;
};