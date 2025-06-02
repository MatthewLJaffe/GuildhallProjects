#include "Game/GameStateLevel2.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Game/EnemyProjectile.hpp"
#include "Game/Boss2.hpp"
#include "Game/Tasks.hpp"
#include "Game/Wisp.hpp"

GameStateLevel2::GameStateLevel2(GameStateType gameType, SoundID levelMusic)
	: GameStateLevel(gameType, levelMusic)
{
}

void GameStateLevel2::StartUp()
{
	m_levelTimer = Timer(123.f, g_theApp->GetGameClock());
	m_beatTime = 60.f / 128.f;

	//Add Tasks
	m_taskQueue.push_back(new T_B2_Intro(this, 34.f));
	m_taskQueue.push_back(new T_B2_BossSpawn(this, 34.f));
	m_taskQueue.push_back(new T_B2_TeleportingAttacks(this, 32.f));
	m_taskQueue.push_back(new T_B2_DevilAttacks(this, 33.f));
	m_taskQueue.push_back(new T_B2_CrazyAttack(this, 31.f));
	m_taskQueue.push_back(new T_B2_TeleportingAttacks(this, 32.f));
	m_taskQueue.push_back(new T2_B2_End(this, 52.f));
}

void GameStateLevel2::OnEnable()
{
	m_levelTimer.Start();
	m_beatsToSkipAhead = 0.f;
	m_doDebugPause = false;
	m_beatsToPauseAt = 0.0;
	g_theGame->PlaySound(m_levelMusic, SoundType::MUSIC, false, 1.f, .5f, 1.f, m_beatTime * m_beatsToSkipAhead);
	GameStateLevel::OnEnable();

	//Add Entities
	g_theGame->m_worldCamera.SetCameraPos(g_theGame->m_worldCamera.GetOrthoDimensions() * .5f);

	Player* player = new Player(this, EntityType::PLAYER, Vec2(GetWorldScreenDimensions().x * .25f, GetWorldScreenDimensions().y * .5f));
	player->m_sortOrder = 6;
	AddEntity(player);
	AddEntity(new Boss2(this, EntityType::BOSS_2, GetWorldScreenDimensions() * .5f));
}
