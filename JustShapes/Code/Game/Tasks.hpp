#pragma once
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/EngineCommon.hpp"

class Entity;
class Boss1;
class GameStateLevel;

class Task
{
public:
	Task(GameStateLevel* gameState, float numBeats);
	virtual void Tick();
	virtual void Begin();
	virtual void End();
	virtual void DebugSkip();
	float m_numBeats = 1.f;
	GameStateLevel* m_gameState = nullptr;
};

class T_PlayAnimation : public Task
{
public:
	T_PlayAnimation(GameStateLevel* gameState, float numBeats, Entity* entityToPlayAnimOn, std::string animName);
	void Begin() override;
	void End() override;
	void DebugSkip() override;
	std::string m_animName = "";
	Entity* m_entityToPlayAnimOn;
};

class T_B1_A1_StillSpiral : public Task 
{
public:
	T_B1_A1_StillSpiral(GameStateLevel* gameState, float numBeats);
	void Tick() override;
	void Begin() override;
	bool m_firedSecondVolley = false;
	bool m_firedThirdVolley = false;
	bool m_firedFourthVolley = false;
	bool m_firedFifthVolley = false;
};

class T_B1_A1_SecondSpiral : public Task
{
public:
	T_B1_A1_SecondSpiral(GameStateLevel* gameState, float numBeats, int rythmicRotationBeats);
	void Tick() override;
	void Begin() override;
	int m_rytmicRotationBeats;
};

class T_B1_A2 : public Task
{
public:
	T_B1_A2(GameStateLevel* gameState, float numBeats);
	void Tick() override;
	void Begin() override;
	Timer m_panToBossTime;
	Vec2 m_startSpriteDimensions = Vec2(32,32);
	Vec2 m_finalSpriteDimensions = Vec2(64, 64);
	Timer m_enlargeTimer;
	Timer m_attractSpawnTimer;
};

class T_B1_RotatingHazards : public Task 
{
public:
	T_B1_RotatingHazards(GameStateLevel* gameState, float numBeats);
	void Begin() override;
	void Tick() override;
	Timer m_panToBossTimer;
	Vec2 m_cameraStartPos;
};

class T_B1_Die : public Task 
{
public:
	void Begin() override;
	void Tick() override;
	T_B1_Die(GameStateLevel* gameState, float numBeats);
};

class T_B2_Intro : public Task 
{
public:
	T_B2_Intro(GameStateLevel* gameState, float numBeats);
	void Tick() override;
	void Begin() override;
	void End() override;
	Timer m_sprialBulletTimer;
	float m_sprialRotationSpeed = 360.f;
	float m_spriralTheta = 0.f;
};

class T_B2_BossSpawn : public Task
{
public:
	T_B2_BossSpawn(GameStateLevel* gameState, float numBeats);
	void Tick() override;
	void Begin() override;
	void DebugSkip() override;
};

class T_B2_TeleportingAttacks : public Task
{
public:
	T_B2_TeleportingAttacks(GameStateLevel* gameState, float numBeats);
	void Tick() override;
	void Begin() override;
	void SpawnBoopBoopBeep();
	std::vector<Vec2> m_teleportLocations;
	int m_currentTeleportCircleIdx = 0;
	int m_currentTeleportIdx = 0;
};

class T_B2_DevilAttacks : public Task 
{
public:
	T_B2_DevilAttacks(GameStateLevel* gameState, float numBeats);
	void Tick() override;
	void Begin() override;
	void DoDevilAttackAtPos(Vec2 const& pos);
	Vec2 m_nextRandomAttackPos;
};

class T_B2_CrazyAttack : public Task
{
public:
	T_B2_CrazyAttack(GameStateLevel* gameState, float numBeats);
	void Tick() override;
	void Begin() override;
	void End() override;
	void SpawnBobbingProjectiles();
	void SpawnArms();
};

class T2_B2_End : public Task 
{
public:
	T2_B2_End(GameStateLevel* gameState, float numBeats);
	void Begin() override;
	void Tick() override;
	void SpawnDevilClones();
	void SpawnDevilSpiralAtPos(Vec2 pos);
	Vec2 m_nextTeleportPos;
};

class T_B3_Intro : public Task
{
public:
	T_B3_Intro(GameStateLevel* gameState, float numBeats);
	void Begin() override;
	void Tick() override;
	Timer m_twoThridBeatTimer;
	Timer m_thirdBeatTimer;
	Timer m_quarterBeatTimer;
};

class T_B3_CryFast : public Task
{
public:
	T_B3_CryFast(GameStateLevel* gameState, float numBeats);
	void Begin() override;
	void Tick() override;
	Timer m_twoThridBeatTimer;
};

class T_B3_CryFaster : public Task
{
public:
	T_B3_CryFaster(GameStateLevel* gameState, float numBeats);
	void Begin() override;
	void Tick() override;
	Timer m_thirdBeatTimer;
};

class T_B3_CryReallyFast : public Task
{
public:
	T_B3_CryReallyFast(GameStateLevel* gameState, float numBeats);
	void Begin() override;
	void Tick() override;
	Timer m_quarterBeatTimer;
};

class T_B3_GearSpinAttack : public Task
{
public:
	T_B3_GearSpinAttack(GameStateLevel* gameState, float numBeats);
	void Begin() override;
	void Tick() override;
	bool m_spawnedGear = false;
};

class T_B3_SawAttack : public Task
{
public:
	T_B3_SawAttack(GameStateLevel* gameState, float numBeats);
	void Begin() override;
	void Tick() override;
};

class T_B3_Falldown : public Task
{
public:
	T_B3_Falldown(GameStateLevel* gameState, float numBeats, bool addArms = false, int beatToAddArms = 0);
	void Begin() override;
	void Tick() override;
	int m_beatToAddArms = 0;
	bool m_addArms = false;
};

class T_B3_Arms : public Task
{
public:
	T_B3_Arms(GameStateLevel* gameState, float numBeats, bool startImmediately = false);
	void Begin() override;
	void Tick() override;
	void End() override;
	bool m_startImmediately = false;
};

class T_B3_FlyIn : public Task
{
public:
	T_B3_FlyIn(GameStateLevel* gameState, float numBeats, bool shrink = false);
	void Begin() override;
	void Tick() override;
	void End() override;
	bool m_shrink = false;
	Timer m_scaleTimer;
	Vec2 m_startBossSize;
	bool m_playedOutroAnimation = false;
};

class T_B3_HeartbeatAttack : public Task 
{
public:
	T_B3_HeartbeatAttack(GameStateLevel* gameState, float numBeats);
	void Begin() override;
	void Tick() override;
};

class T_SadBoySequence : public Task
{
public:
	T_SadBoySequence(GameStateLevel* gameState, float numBeats);
	void Begin() override;
	void Tick() override;
};
