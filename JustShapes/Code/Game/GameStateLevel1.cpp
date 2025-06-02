#include "GameStateLevel1.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Game/EnemyProjectile.hpp"
#include "Game/Boss1.hpp"
#include "Game/Tasks.hpp"
#include "Game/Particle.hpp"

GameStateLevel1::GameStateLevel1(GameStateType gameType, SoundID levelMusic)
	: GameStateLevel(gameType, levelMusic)
{
}

void GameStateLevel1::StartUp()
{
	g_theGame->m_worldCamera.SetCameraPos(g_theGame->m_worldCamera.GetOrthoDimensions() * .5f);

	Player* player = new Player(this, EntityType::PLAYER, Vec2(GetWorldScreenDimensions().x * .25f, GetWorldScreenDimensions().y * .5f));
	player->m_sortOrder = 6;
	AddEntity(player);

	AddEntity(new Boss1(this, EntityType::BOSS_1, GetWorldScreenDimensions() * .5f));

	//Add Tasks
	m_taskQueue.push_back(new T_PlayAnimation(this, 2.f, GetBoss1(), "Boss1Spawn"));
	m_taskQueue.push_back(new T_B1_A1_StillSpiral(this, 15.f));
	m_taskQueue.push_back(new T_PlayAnimation(this, 0.f, GetBoss1(), "Boss1EyeOpen"));
	m_taskQueue.push_back(new T_B1_A1_SecondSpiral(this, 32.f, 30));
	m_taskQueue.push_back(new T_B1_A2(this, 32.f));
	m_taskQueue.push_back(new T_PlayAnimation(this, 0.f, GetBoss1(), "Boss1HeadBop"));
	m_taskQueue.push_back(new T_B1_RotatingHazards(this, 103.f));
	m_taskQueue.push_back(new Task(this, 10.f));
	m_taskQueue.push_back(new T_PlayAnimation(this, 0.f, GetBoss1(), "Boss1EyeOpen"));
	m_taskQueue.push_back(new T_B1_A1_StillSpiral(this, 15.f));
	m_taskQueue.push_back(new T_B1_A1_SecondSpiral(this, 40.f, 40));
	m_taskQueue.push_back(new T_PlayAnimation(this, 8.f, GetBoss1(), "Boss1Shocked"));
	m_taskQueue.push_back(new T_B1_Die(this, 10.f));
}

void GameStateLevel1::OnEnable()
{
	m_levelTimer = Timer(130.f, g_theApp->GetGameClock());
	m_beatTime = 60.f / 127.f;

	m_levelTimer.Start();
	m_beatsToSkipAhead = 0.f;
	m_doDebugPause = false;
	m_beatsToPauseAt = 0.0;
	g_theGame->PlaySound(m_levelMusic, SoundType::MUSIC, false, 1.f, .5f, 1.f, m_beatTime * m_beatsToSkipAhead);
	GameStateLevel::OnEnable();
}
