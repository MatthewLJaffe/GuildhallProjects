#include "Game/Tasks.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include "Game/Weapon.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/BinkeyController.hpp"

Task::Task(std::string taskName)
	: m_name(taskName)
{
}

Task::~Task()
{
}

//Task 1
Task1_Start::Task1_Start(std::string name)
	: Task(name)
{
}

void Task1_Start::OnStart()
{
}

bool Task1_Start::Tick()
{
	//wait until player picks up flamethrower
	if (g_theGame->m_players[0]->GetActor()->m_weapons.size() > 0)
	{
		return true;
	}
	return false;
}

void Task1_Start::OnEnd()
{
}


//Task 2
Task2_SpawnDemons::Task2_SpawnDemons(std::string name)
	: Task(name)
	, m_demonSpawnTimer(Timer(5.f, g_theApp->GetGameClock()))
{
}

void Task2_SpawnDemons::OnStart()
{
	m_demonSpawnTimer.Stop();
}

bool Task2_SpawnDemons::Tick()
{
	if (g_theGame->m_currentMap->GetNumEnemies() == 0)
	{
		if (m_demonSpawnTimer.IsStopped())
		{
			m_demonSpawnTimer.Start();
		}
	}
	if (m_demonSpawnTimer.HasPeriodElapsed())
	{
		m_demonSpawnTimer.Stop();
		g_theGame->m_currentMap->SpawnEnemy();
	}

	int numPointLights = g_theGame->m_currentMap->GetNumPointLights();
	if (!m_pointlightMessage1Started && numPointLights >= m_pointlightMessage1Threshold)
	{
		g_dialogSystem->StartNewDialogue(pointLightMessage1);
		m_pointlightMessage1Started = true;
		g_theApp->SetArtificialLag(2);
	}
	if (!m_pointlightMessage2Started && numPointLights >= m_pointlightMessage2Threshold)
	{
		g_dialogSystem->StartNewDialogue(pointLightMessage2);
		m_pointlightMessage2Started = true;
		g_theApp->SetArtificialLag(10);
	}
	if (!m_pointlightMessage3Started && numPointLights >= m_pointlightMessage3Threshold)
	{
		g_dialogSystem->StartNewDialogue(pointLightMessage3);
		m_pointlightMessage3Started = true;
		g_theApp->SetArtificialLag(50);
	}
	if (!m_pointlightMessage4Started && numPointLights >= m_pointlightMessage4Threshold)
	{
		g_theApp->SetArtificialLag(100);
		g_theGame->m_players[0]->ReduceGrade(10, "Low Performance -10", 1.f);
		g_dialogSystem->StartNewDialogue(pointLightMessage4);
		m_pointlightMessage4Started = true;
		return true;
	}
	return false;
}

void Task2_SpawnDemons::OnEnd()
{
	m_demonSpawnTimer.Stop();
}


//TASK 3
Task3_RemovePointLights::Task3_RemovePointLights(std::string name)
	: Task(name)
{
}

void Task3_RemovePointLights::OnStart()
{
}

bool Task3_RemovePointLights::Tick()
{
	if (g_dialogSystem->GetCurrentDialogSequence() == nullptr)
	{
		g_theGame->m_currentMap->DestroyAllActorsWithName("Flame");
		g_theGame->m_currentMap->m_currNumPointLights = 0;
		g_theApp->SetArtificialLag(0);

		g_theGame->m_currentMap->DestroyLightTiles();
		g_dialogSystem->StartNewDialogue("wallsDestroyed");

		Actor* player = g_theGame->m_players[0]->GetActor();
		for (size_t i = 0; i < player->m_weapons.size(); i++)
		{
			delete player->m_weapons[i];
		}
		player->m_weapons.clear();
		player->m_weapons.push_back( new Weapon(WeaponDefinition::GetByName("Pistol"), player->GetActorUID()) );


		return true;
	}
	return false;
}

void Task3_RemovePointLights::OnEnd()
{
	
}


//TASK 4
Task4_OutOfBoundsError::Task4_OutOfBoundsError(std::string name)
	: Task(name)
{
}

void Task4_OutOfBoundsError::OnStart()
{
}

bool Task4_OutOfBoundsError::Tick()
{
	Map* currentMap = g_theGame->m_currentMap;
	Actor* playerActor = g_theGame->GetFirsPlayerActor();
	if (currentMap->GetTileFromPosition(playerActor->m_position).m_def->m_name == "Empty" && !m_errorDialogStarted)
	{
		g_theGame->m_players[0]->m_showErrorMessage = true;
		g_theAudio->StartSound(SOUND_ID_ERROR_MESSAGE);
		g_theGame->m_players[0]->ReduceGrade(10, "Crash -10", 1.f);
		g_dialogSystem->StartNewDialogue("indexOutOfBounds");
		m_errorDialogStarted = true;
	}

	if (m_errorDialogStarted && g_dialogSystem->GetCurrentDialogSequence() == nullptr)
	{
		return true;
	}
	return false;
}

void Task4_OutOfBoundsError::OnEnd()
{
	g_theGame->m_players[0]->m_showErrorMessage = false;
	g_theGame->AdvanceMap();
}

Task5_CheckForBinkeyAlerted::Task5_CheckForBinkeyAlerted(std::string name)
	: Task(name)
{
}

void Task5_CheckForBinkeyAlerted::OnStart()
{
}

bool Task5_CheckForBinkeyAlerted::Tick()
{
	//check for dialog initiation
	if (!m_binkeyAlerted && !m_binkeyDialogStarted)
	{
		Actor* binkey = g_theGame->m_currentMap->GetFirstActorWithName("Binkey");
		if (GetDistance3D(binkey->m_position, g_theGame->m_players[0]->GetActor()->m_position) < 2.5f)
		{
			Vec2 dispFromBinkeyToPlayer = g_theGame->m_players[0]->GetActor()->m_position.GetXY() - binkey->m_position.GetXY();
			binkey->m_orientation.m_yaw = dispFromBinkeyToPlayer.GetOrientationDegrees();

			g_dialogSystem->StartNewDialogue("binkeyAlerted");
			m_binkeyDialogStarted = true;
		}

		else if (m_binkeyHurt && !m_binkeyDialogStarted)
		{
			binkey = g_theGame->m_currentMap->GetFirstActorWithName("Binkey");
			Vec2 dispFromBinkeyToPlayer = g_theGame->m_players[0]->GetActor()->m_position.GetXY() - binkey->m_position.GetXY();
			binkey->m_orientation.m_yaw = dispFromBinkeyToPlayer.GetOrientationDegrees();

			g_dialogSystem->StartNewDialogue("binkeyHurt");
			m_binkeyDialogStarted = true;
		}
	}

	//check for conversation completed and end task
	if (m_binkeyDialogStarted && g_dialogSystem->GetCurrentDialogSequence() == nullptr)
	{
		return true;
	}
	return false;
}

void Task5_CheckForBinkeyAlerted::OnEnd()
{
}

Task6_BinkeyRunIntoWall::Task6_BinkeyRunIntoWall(std::string name)
	: Task(name)
	, m_convo1Timer(Timer(3.f, g_theApp->GetGameClock()))
{
}

void Task6_BinkeyRunIntoWall::OnStart()
{
	m_holdW = true;
	m_convo1Timer.Start();
}

bool Task6_BinkeyRunIntoWall::Tick()
{
	if (m_holdW)
	{
		Actor* binkey = g_theGame->m_currentMap->GetFirstActorWithName("Binkey");
		binkey->TurnTorwardsDirection(Vec2(0.f, 1.f));
		binkey->MoveInDirection(Vec3(0.f, 1.f, 0.f), binkey->m_actorDefinition->m_runSpeed);
	}
	if (m_convo1Timer.HasPeriodElapsed())
	{
		m_convo1Timer.Stop();
		g_dialogSystem->StartNewDialogue("binkeyRunIntoWall");
	}
	if (m_convo1Timer.IsStopped() && g_dialogSystem->GetCurrentDialogSequence() == nullptr)
	{
		return true;
	}
	return false;
}

void Task6_BinkeyRunIntoWall::OnEnd()
{
	g_theGame->m_currentMap->GetFirstActorWithName("Binkey")->KillActor();
	g_theGame->m_players[0]->ReduceGrade(10, "Bad AI", 3.f);
}

Task7_BinkeyRespawn::Task7_BinkeyRespawn(std::string name)
	: Task(name)
{
}

void Task7_BinkeyRespawn::OnStart()
{
}

bool Task7_BinkeyRespawn::Tick()
{
	Actor* binkey = g_theGame->m_currentMap->GetFirstActorWithName("Binkey");
	if (binkey->IsAlive() && GetDistance3D(binkey->m_position, g_theGame->m_players[0]->GetActor()->m_position) < 3.f && !m_respawnConvoStared)
	{
		m_respawnConvoStared = true;
		g_dialogSystem->StartNewDialogue("binkeyRespawn");
	}
	if (m_respawnConvoStared && g_dialogSystem->GetCurrentDialogSequence() == nullptr)
	{
		return true;
	}
	return false;
}

void Task7_BinkeyRespawn::OnEnd()
{
}

Task8_BinkeyCombat::Task8_BinkeyCombat(std::string name)
	: Task(name)
	, m_messageTimer(Timer(3.f, g_theApp->GetGameClock()))
{
}

void Task8_BinkeyCombat::OnStart()
{
	for (int i = 0; i < 3; i++)
	{
		g_theGame->m_currentMap->SpawnEnemy();
	}
	Actor* binkey = g_theGame->m_currentMap->GetFirstActorWithName("Binkey");
	BinkeyController* binkeyAI = dynamic_cast<BinkeyController*>(binkey->m_controller);
	binkeyAI->m_currentState = BinkeyState::BINKEY_STATE_ATTACK;
	m_messageTimer.Start();
}

bool Task8_BinkeyCombat::Tick()
{
	if (g_theGame->m_currentMap->GetNumEnemies() == 0)
	{
		return true;
	}
	if (m_messageTimer.HasPeriodElapsed())
	{
		m_messageTimer.Start();
		int rand = g_randGen->RollRandomIntInRange(0, 2);
		if (rand == 0)
		{
			g_dialogSystem->StartNewDialogue("binkeyHelp");
		}
		else if (rand == 1)
		{
			g_dialogSystem->StartNewDialogue("binkeyAgony");
		}
		else if (rand == 2)
		{
			g_dialogSystem->StartNewDialogue("binkeyStop");
		}
	}

	return false;
}

void Task8_BinkeyCombat::OnEnd()
{
	Actor* binkey = g_theGame->m_currentMap->GetFirstActorWithName("Binkey");
	BinkeyController* binkeyAI = dynamic_cast<BinkeyController*>(binkey->m_controller);
	binkeyAI->m_currentState = BinkeyState::BINKEY_STATE_IDLE;
}

Task9_BinkeyRap::Task9_BinkeyRap(std::string name)
	: Task(name)
	, m_rapTimer(216, g_theApp->GetGameClock())
{
}

void Task9_BinkeyRap::OnStart()
{
	g_dialogSystem->StartNewDialogue("binkeyRap");
}

bool Task9_BinkeyRap::Tick()
{
	Actor* binkey = g_theGame->m_currentMap->GetFirstActorWithName("Binkey");
	if (g_dialogSystem->GetCurrentDialogSequence() == nullptr && !m_rapStarted)
	{
		m_rapStarted = true;
		m_rap = g_theAudio->StartSoundAt(g_theAudio->CreateOrGetSound("Data/Audio/Music/Amesbury Dr.mp3"), binkey->m_position);
		m_rapTimer.Start();
	}
	if (m_rapStarted)
	{
		g_theAudio->SetSoundPosition(m_rap, binkey->m_position);
		if ( m_rapTimer.HasPeriodElapsed() )
		{
			g_theGame->m_players[0]->ReduceGrade(10, "Rapping", 4.f);
			return true;
		}
	}
	if (m_endRap)
	{
		g_theGame->m_players[0]->ReduceGrade(10, "Rapping", 4.f);
		return true;
	}
	return false;
}

void Task9_BinkeyRap::OnEnd()
{
	g_theAudio->StopSound(m_rap);
}

Task10_ShrinkRayIntro::Task10_ShrinkRayIntro(std::string name)
	: Task(name)
{
}

void Task10_ShrinkRayIntro::OnStart()
{
	g_dialogSystem->StartNewDialogue("shrinkRayIntro");
}

bool Task10_ShrinkRayIntro::Tick()
{
	if (g_dialogSystem->GetCurrentDialogSequence() == nullptr)
	{
		return true;
	}
	return false;
}

void Task10_ShrinkRayIntro::OnEnd()
{
	g_theGame->AdvanceMap();
	delete g_theGame->m_players[0]->GetActor()->m_weapons[0];
	g_theGame->m_players[0]->GetActor()->m_weapons.clear();
}

Task11_ShrinkRayEnd::Task11_ShrinkRayEnd(std::string name)
	: Task(name)
{
}

void Task11_ShrinkRayEnd::OnStart()
{
	g_dialogSystem->StartNewDialogue("shrinkRay2");
}

bool Task11_ShrinkRayEnd::Tick()
{
	Actor* shrinkRay = g_theGame->m_currentMap->GetFirstActorWithName("ShrinkrayPickup");
	if (shrinkRay != nullptr && GetDistance3D(shrinkRay->m_position, g_theGame->m_players[0]->GetActor()->m_position) < 4.f && !m_warningConvoStarted)
	{
		m_warningConvoStarted = true;
		g_dialogSystem->StartNewDialogue("shrinkRay3");
	}
	if (m_shrinkRayEquipped && !m_lastShrinkRayConvoStarted)
	{
		m_lastShrinkRayConvoStarted = true;
		g_dialogSystem->StartNewDialogue("shrinkRay4");
	}
	if (m_lastShrinkRayConvoStarted && g_dialogSystem->GetCurrentDialogSequence() == nullptr)
	{
		return true;
	}
	return false;
}

void Task11_ShrinkRayEnd::OnEnd()
{
}

Task12_FOOM::Task12_FOOM(std::string name)
	: Task(name)
	, m_failTimer(Timer(5.f, g_theApp->GetGameClock()))
{
}

void Task12_FOOM::OnStart()
{
	g_theGame->m_players[0]->ReduceGrade(50, "Failure", 5.f);
	m_failTimer.Start();
}

bool Task12_FOOM::Tick()
{
	if (m_failTimer.HasPeriodElapsed())
	{
		m_failTimer.Stop();
		g_dialogSystem->StartNewDialogue("fail");
	}
	if (m_failTimer.IsStopped() && g_dialogSystem->GetCurrentDialogSequence() == nullptr)
	{
		return true;
	}
	return false;
}

void Task12_FOOM::OnEnd()
{
	g_theGame->m_currentGameMode = GameMode::FOOM;
}
