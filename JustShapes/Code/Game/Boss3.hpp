#pragma once
#include "Game/Entity.hpp"
#include "Game/BoxHazard.hpp"

class Boss3 : public Entity
{
public:
	Boss3(GameState* gameState, EntityType entityType, Vec2 const& startPos, EntityConfig const& config);
	void Update(float deltaSeconds) override;
	bool OverlapsPlayer(Player* player) override;
	void Render() override;
	void Wiggle();
	void Shake(float shakeTime = .96f, bool changeFaces = false, float shakeScale = 1500000.f);
	void ArmShake(bool endFaceOnSaw = false, float armShakeDuration = 1.5f);
	void WiggleFast();
	void UpdateWiggle();
	void UpdateWiggleFast();
	void UpdateShake();
	void UpdateArmsShake();
	void SetRandomSprite();
	void SpawnTearBullets();
	void AddArm(float rotationSpeed = 20.f);
	void RemoveArms();
	void StartSpeedUpArms();
	float m_shakeScale = 1500000.f;
	float m_wiggleScale = 7000000.f;
	float m_wiggleDeltaDegrees = 12.f;
	std::vector<BoxHazard*> m_arms;
	SpriteSheet* m_boss3Sheet = nullptr;
	Timer m_wiggleTimer;
	Timer m_wiggleFastTimer;
	Timer m_shakeTimer;
	Timer m_timeBetweenSpeedUpArms;
	Timer m_armsShakeTimer;
	int m_armSpeedUpIdx = 0;
	int m_timesToChangeFace = 0;
	Timer m_changeFaceTimer;
	bool m_speedUpArms = false;
	bool m_hidden = false;
	bool m_endFaceOnSaw = false;
private:
	void HandleSpeedUpArms();
};