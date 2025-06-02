#include "Game/GameStateLevel3.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Game/EnemyProjectile.hpp"
#include "Game/Boss3.hpp"
#include "Game/Tasks.hpp"

GameStateLevel3::GameStateLevel3(GameStateType gameType, SoundID levelMusic)
	: GameStateLevel(gameType, levelMusic)
{
}

void GameStateLevel3::StartUp()
{
	Player* player = new Player(this, EntityType::PLAYER, Vec2(GetWorldScreenDimensions().x * .25f, GetWorldScreenDimensions().y * .5f));
	player->m_sortOrder = 6;
	AddEntity(player);
	AddEntity(new Boss3(this, EntityType::BOSS_3, GetWorldScreenDimensions() * .5f, EntityConfig::GetEntityConfigByName("Boss3")));

	m_levelTimer = Timer(146.f, g_theApp->GetGameClock());
	m_beatTime = 60.f / 124.f;

	//Add Tasks
	m_taskQueue.push_back(new T_B3_Intro(this, 19.f));
	m_taskQueue.push_back(new T_B3_CryFast(this, 13.f));
	m_taskQueue.push_back(new T_B3_CryFaster(this, 40.f));
	m_taskQueue.push_back(new T_B3_CryReallyFast(this, 52.f));
	m_taskQueue.push_back(new T_B3_HeartbeatAttack(this, 8.f));
	m_taskQueue.push_back(new T_B3_Falldown(this, 3.f));
	m_taskQueue.push_back(new T_B3_Arms(this, 4.f));
	m_taskQueue.push_back(new T_B3_GearSpinAttack(this, 11.f));
	m_taskQueue.push_back(new T_B3_FlyIn(this, 6.f));
	m_taskQueue.push_back(new T_B3_SawAttack(this, 7.f));
	m_taskQueue.push_back(new T_PlayAnimation(this, 4.f, GetBoss3(), "PreArms"));
	m_taskQueue.push_back(new T_B3_Arms(this, 6.f));
	m_taskQueue.push_back(new T_B3_HeartbeatAttack(this, 7.f));
	m_taskQueue.push_back(new T_B3_Falldown(this, 3.f));
	m_taskQueue.push_back(new T_B3_Arms(this, 8.f));

	m_taskQueue.push_back(new T_SadBoySequence(this, 32.f));
	m_taskQueue.push_back(new T_B3_Arms(this, 5.f));
	m_taskQueue.push_back(new T_B3_SawAttack(this, 7.f));
	m_taskQueue.push_back(new T_PlayAnimation(this, 3.f, GetBoss3(), "PreArms"));
	m_taskQueue.push_back(new T_B3_Arms(this, 6.f));
	m_taskQueue.push_back(new T_B3_GearSpinAttack(this, 10.f));
	m_taskQueue.push_back(new T_B3_FlyIn(this, 6.f));
	m_taskQueue.push_back(new T_B3_HeartbeatAttack(this, 7.f));
	m_taskQueue.push_back(new Task(this, 2.f));
	m_taskQueue.push_back(new T_B3_Falldown(this, 3.f, true, 2));
	m_taskQueue.push_back(new T_B3_Arms(this, 4.f, true));
	m_taskQueue.push_back(new T_B3_GearSpinAttack(this, 11.f));
	m_taskQueue.push_back(new T_B3_FlyIn(this, 14.f, true));
}

void GameStateLevel3::OnEnable()
{
	m_levelTimer.Start();
	m_beatsToSkipAhead = 0.f;
	m_doDebugPause = false;
	m_beatsToPauseAt = 0.0;
	g_theGame->PlaySound(m_levelMusic, SoundType::MUSIC, false, 1.f, .5f, 1.f, m_beatTime * m_beatsToSkipAhead);
	GameStateLevel::OnEnable();

	//Add Entities
	g_theGame->m_worldCamera.SetCameraPos(g_theGame->m_worldCamera.GetOrthoDimensions() * .5f);
}
