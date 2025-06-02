#pragma once
#include "Game/GameState.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Boss2.hpp"
#include <deque>

class Boss1;
class Task;

class GameStateLevel : public GameState
{
public:
	GameStateLevel(GameStateType gameType, SoundID levelMusic);
	virtual void StartUp() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void Update(float deltaSeconds);
	void UpdateTaskQueue();
	virtual void Render() override;
	void AddArms(float armsRotationSpeed);
	void RenderLives();
	void RenderProgressUI();
	Timer m_levelTimer;
	SoundID m_levelMusic;
	bool m_winLevel = false;
	void ShowWinLevel();
	void TogglePause();
	void UpdateCollision();
	void SpawnProjectilesInCircle(Vec2 spawnPoint, int numProjectiles, EntityConfig& entityConfig, float speed = 32.f, float acceleration = 32.f, float thetaOffset = 0.f);
	void SpawnProjectilesInSpiral(Vec2 spawnPoint, int numProjectiles, float timeBetweenProjectiles, EntityConfig& bulletConfig, float speed = 32.f, float acceleration = 32.f, bool clockwise = true, float numCircles = 1);
	void SpawnEntityInRandomPosOnScreen(EntityConfig& config, EntityType type, Vec2 normalizedScreenPadding = Vec2::ZERO);
	Vec2 GetRandomPosBelowScreen(float padding = 32.f);
	void SpawnEntityAtPos(EntityConfig& config, EntityType type, Vec2 pos);
	void SpawnScytheAtPos(EntityConfig& config, EntityType type, Vec2 pos);
	Vec2 RollRandomPosOnScreen(Vec2 normalizedScreenPadding = Vec2::ZERO);
	void SpawnRandomBarHazard(bool alwaysVertical = false);
	void SpawnRandomDiscHazard();
	void SpawnDiscHazardOnPlayer();
	void SpawnWispOffscreen();
	void SpawnConeHazard();
	void SpawnHeartbeatIndicator();
	void SpawnBossExplosion();
	void SpawnExplodingProjectile();
	void BeatDropKillAllScythes();
	void DestroyBobbingProjectiles();
	void DestroyWisps();
	void RemoveAllArms();
	void RemoveAllButtons();
	void SpawnArcProjectiles(Vec2 coneOrigin, float coneOrientation, float coneMaxAngle);
	//Queries
	Boss1* GetBoss1();
	float GetTotalBeatsElapsed();
	float GetCurrentTaskBeats();
	void SetPause(bool isPaused) override;
	bool IsBeatThisFrame(int& out_beatNumber);
	float m_beatsToSkipAhead = 0.f;
	float m_beatsToPauseAt = 0.f;
	bool m_doDebugPause = false;
	std::deque<Task*> m_taskQueue;
	Task* m_currentTask = nullptr;
	float m_beatTime = 0.f;
	float m_taskBeatsElapsed = 0.f;
	int m_prevFrameBeats = 0;
	Vec2 m_progressPos;
};

bool Event_ResumeLevel(EventArgs& args);
bool Event_ResetLevel(EventArgs& args);
bool Event_NextLevelPressed(EventArgs& args);

