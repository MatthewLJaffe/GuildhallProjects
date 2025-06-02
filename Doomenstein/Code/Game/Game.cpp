#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/Tasks.hpp"
#include "Game/Actor.hpp"

Game::Game(App* app)
	: m_theApp(app)
{}

Game::~Game() {}

void Game::StartUp()
{
	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
	g_bitMapFont = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Fonts/SquirrelFixedFont");
	m_entireScreenCamera.SetOrthographicView(Vec2::ZERO, GetScreenDimensions());
	m_entireScreenCamera.m_mode = Camera::eMode_Orthographic;
	m_currentGameMode = GameMode::ATTRACT;

	LoadAssets();
	g_dialogSystem = new DialogSystem();
	m_menuMusicPlayback = g_theAudio->StartSound(SOUND_ID_MENU_MUSIC, true);
}

void Game::StartGame()
{
	g_theAudio->StartSound(SOUND_ID_BUTTON_CLICK);
	m_currentGameMode = GameMode::PLAYING;
	DebugAddWorldBasis(Mat44(), -1.f, DebugRenderMode::USE_DEPTH);

	Map* map1 = new Map(*MapDefinition::GetByName(g_gameConfigBlackboard.GetValue("defaultMap", "No default map")));
	Map* map2 = new Map(*MapDefinition::GetByName("Map2"));
	Map* map3 = new Map(*MapDefinition::GetByName("Map3"));

	map1->InitializeMap();
	m_maps.push_back(map1);
	map2->InitializeMap();
	m_maps.push_back(map2);
	map3->InitializeMap();
	m_maps.push_back(map3);

	m_currentMapIdx = 0;
	m_currentMap =  m_maps[m_currentMapIdx];

	for (size_t i = 0; i < m_players.size(); i++)
	{
		m_players[i]->m_map = m_currentMap;
	}
	for (size_t i = 0; i < g_theGame->m_players.size(); i++)
	{
		m_currentMap->SpawnPlayer((int)i);
	}

	g_theAudio->SetNumListeners((int)m_players.size());
	g_theAudio->StopSound(m_menuMusicPlayback);
	//m_gameMusicPlayback = g_theAudio->StartSound(SOUND_ID_GAME_MUSIC, true);
	g_dialogSystem->StartNewDialogue("intro");

	InitializeTasks();
}

Player* Game::GetControllerPlayer(int controllerIdx)
{
	for (size_t i = 0; i < m_players.size(); i++)
	{
		if (m_players[i]->m_controllerIndex == controllerIdx)
		{
			return m_players[i];
		}
	}
	return nullptr;
}

Actor* Game::GetFirsPlayerActor()
{
	return m_players[0]->GetActor();
}

Player* Game::GetCurrentlyRenderingPlayer()
{
	return m_currentlyRenderingPlayer;
}

Player* Game::GetCurrentlyUpdatingPlayer()
{
	return m_currentlyUpdatingPlayer;
}

void Game::RenderAttract()
{
	g_theRenderer->BeginCamera(m_entireScreenCamera);
	std::vector<Vertex_PCU> attractScreenText;
	Vec2 screenDimensions = GetScreenDimensions();
	AABB2 instructionsTextBox(screenDimensions.x * .2f, screenDimensions.y * .05f, screenDimensions.x * .7f, screenDimensions.y * .6f);
	g_bitMapFont->AddVertsForTextInBox2D(attractScreenText, instructionsTextBox, 20.f, 
		"Press space to join with mouse and keyboard\nPress start to join with controller\nPress Escape or Back to exit", Rgba8::WHITE,1.f, Vec2(.5f, 0.f));
	g_theRenderer->BindTexture(g_bitMapFont->GetTexture());
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(attractScreenText.size(), attractScreenText.data());
	g_theRenderer->EndCamera(m_entireScreenCamera);
}

void Game::RenderLobby()
{
	for (size_t i = 0; i < m_players.size(); i++)
	{
		float textScalingFactor = m_players.size() == 1 ? 1.f : 2.f;
		g_theRenderer->BeginCamera(m_players[i]->m_screenCamera);

		std::vector<Vertex_PCU> playerText;
		float aspectRatio = g_gameConfigBlackboard.GetValue("aspectRatio", 2.f);
		Vec2 screenDimensions = GetScreenDimensions();
		float defaultAspectRatio = screenDimensions.x / screenDimensions.y;

		g_bitMapFont->AddVertsForTextInBox2D(playerText, AABB2(screenDimensions.x*.35f, screenDimensions.y*.45f, screenDimensions.x*.7f, screenDimensions.y*.7f), 50.f * textScalingFactor, 
			Stringf("Player %d", i + 1), Rgba8::WHITE, defaultAspectRatio / aspectRatio);

		std::string inputDevice = "Controller";
		std::string controls = "Press START to start game\nPress BACK to leave game";
		if (m_players[i]->m_controllerIndex == -1)
		{
			inputDevice = "Mouse and Keyboard";
			controls = "Press SPACE to start game\nPress ESCAPE to leave game";

			if (m_players.size() < MAX_PLAYER_COUNT)
			{
				controls += "\nPress START to join player";
			}

		}
		else if (m_players.size() < MAX_PLAYER_COUNT)
		{
			controls += "\nPress START/SPACE to join player";
		}

		g_bitMapFont->AddVertsForTextInBox2D(playerText, AABB2(screenDimensions.x * .35f, screenDimensions.y * .3f, screenDimensions.x * .7f, screenDimensions.y * .5f), 20.f * textScalingFactor,
			inputDevice, Rgba8::WHITE, defaultAspectRatio / aspectRatio);

		g_bitMapFont->AddVertsForTextInBox2D(playerText, AABB2(screenDimensions.x * .35f, screenDimensions.y * .1f, screenDimensions.x * .7f, screenDimensions.y * .3f), 14.f * textScalingFactor,
			controls, Rgba8::WHITE, defaultAspectRatio / aspectRatio, Vec2(.5f, 1.f));

		g_theRenderer->BindTexture(g_bitMapFont->GetTexture());
		g_theRenderer->DrawVertexArray(playerText.size(), playerText.data());
		
		g_theRenderer->EndCamera(m_players[i]->m_screenCamera);
	}
}

void Game::RenderGame()
{
	for (size_t i = 0; i < m_players.size(); i++)
	{
		g_theRenderer->BeginCamera(m_players[i]->m_worldCamera);
		m_currentlyRenderingPlayer = m_players[i];
		m_currentMap->Render();
		g_theRenderer->EndCamera(m_players[i]->m_worldCamera);

		g_theRenderer->BeginCamera(m_players[i]->m_screenCamera);
		m_currentlyRenderingPlayer = m_players[i];
		m_currentlyRenderingPlayer->RenderScreen();
		g_dialogSystem->Render();
		g_theRenderer->EndCamera(m_players[i]->m_screenCamera);
		DebugRenderWorld(m_players[i]->m_worldCamera);
	}

	//ScreenSpace Rendering
	DebugRenderScreen(m_entireScreenCamera);
}

void Game::CheckForDebugCommands()
{
	
}

bool Game::ReadyToStartGame()
{
	for (size_t i = 0; i < m_players.size(); i++)
	{
		if (m_players[i]->m_controllerIndex == -1)
		{
			if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
			{
				return true;
			}
		}
		else
		{
			XboxController const& controller = g_theInput->GetController(m_players[i]->m_controllerIndex);
			if (controller.WasButtonJustPressed(XboxController::START_BUTTON))
			{
				return true;
			}
		}
	}
	return false;
}

void Game::AdvanceMap()
{
	Actor* playerActor = m_players[0]->GetActor();
	ActorUID playerUID = m_players[0]->GetActor()->GetActorUID();
	m_currentMap->RemoveActor(playerUID);
	m_currentMapIdx++;
	if (m_currentMapIdx >= (int)m_maps.size())
	{
		m_currentMapIdx = 0;
	}
	m_currentMap = m_maps[m_currentMapIdx];
	m_currentMap->SpawnPlayer(playerActor);
	m_players[0]->m_map = m_currentMap;
	m_players[0]->m_currentlyPossesedActor = playerActor->GetActorUID();


}

Player* Game::GetKeyboardPlayer()
{
	for (size_t i = 0; i < m_players.size(); i++)
	{
		if (m_players[i]->m_controllerIndex == -1)
		{
			return m_players[i];
		}
	}
	return nullptr;
}

void Game::Update(float deltaSeconds)
{
	switch (m_currentGameMode)
	{
	case GameMode::ATTRACT:
		UpdateAttract(deltaSeconds);
		break;
	case GameMode::LOBBY:
		UpdateLobby(deltaSeconds);
		break;
	case GameMode::PLAYING:
		UpdateGame(deltaSeconds);
		break;
	case GameMode::FOOM:
		UpdateFoom(deltaSeconds);
		break;
	default:
		break;
	}
}

void Game::UpdateAttract(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (IsPlayerJoining())
	{
		g_theAudio->StartSound(SOUND_ID_BUTTON_CLICK);
		m_currentGameMode = GameMode::LOBBY;
	}
	//check for start game

}

Task* Game::GetTaskByName(std::string name)
{
	for (size_t i = 0; i < m_tasks.size(); i++)
	{
		if (m_tasks[i]->m_name == name)
		{
			return m_tasks[i];
		}
	}
	return nullptr;
}

bool Game::IsPlayerLeaving()
{
	bool playerLeft = false;
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) && GetKeyboardPlayer() != nullptr)
	{
		for (size_t i = 0; i < m_players.size(); i++)
		{
			if (m_players[i]->m_controllerIndex == -1)
			{
				m_players.erase(m_players.begin() + i);
				playerLeft = true;
				break;
			}
		}
	}

	for (int controllerIdx = 0; controllerIdx < 4; controllerIdx++)
	{
		const XboxController controller = g_theInput->GetController(controllerIdx);
		if (controller.IsConnected() && controller.WasButtonJustPressed(XboxController::BACK_BUTTON))
		{
			Player* controllerPlayer = GetControllerPlayer(controllerIdx);
			if (controllerPlayer != nullptr)
			{
				for (size_t playerIdx = 0; playerIdx < m_players.size(); playerIdx++)
				{
					if (m_players[playerIdx]->m_controllerIndex == controllerIdx)
					{
						m_players.erase(m_players.begin() + playerIdx);
						playerLeft = true;
						break;
					}
				}
			}
		}
	}

	return playerLeft;
}

bool Game::IsPlayerJoining()
{
	if ((int)m_players.size() == MAX_PLAYER_COUNT)
	{
		return false;
	}

	if (g_theInput->WasKeyJustPressed(' '))
	{
		if (GetKeyboardPlayer() == nullptr)
		{
			m_players.push_back(new Player(this, (int)m_players.size(), -1));
			return true;
		}
	}
	for (int i = 0; i < 4; i++)
	{
		const XboxController controller = g_theInput->GetController(i);
		if (controller.IsConnected() && controller.WasButtonJustPressed(XboxController::START_BUTTON))
		{
			if (GetControllerPlayer(i) == nullptr)
			{
				m_players.push_back(new Player(this, (int)m_players.size(), i));
				return true;
			}
		}
	}
	return false;
}

//#ToDo let players join and leave in lobby
void Game::UpdateLobby(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (IsPlayerJoining() || IsPlayerLeaving())
	{
		g_theAudio->StartSound(SOUND_ID_BUTTON_CLICK);
		HandleScreenSplitting();
	}
	else if (ReadyToStartGame())
	{
		m_currentGameMode = GameMode::PLAYING;
		StartGame();
	}
	else if (m_players.size() == 0)
	{
		m_currentGameMode = GameMode::ATTRACT;
	}
}

void Game::HandleScreenSplitting()
{
	if (m_players.size() == 1)
	{
		g_gameConfigBlackboard.SetValue("screenWidth", "1600");
		g_gameConfigBlackboard.SetValue("screenHeight", "800");
		m_players[0]->SetNormalizedViewport(AABB2::ZERO_TO_ONE);
		Vec2 screenDimensions = GetScreenDimensions();
		float newAspect = 2.f;
		m_players[0]->m_worldCamera.SetAspectRatio(newAspect);
	}
	if (m_players.size() == 2)
	{
		Vec2 screenDimensions = GetScreenDimensions();
		float newAspect = (screenDimensions.x / screenDimensions.y) * 2.f;

		m_players[0]->SetNormalizedViewport(AABB2(Vec2(0.f, .5f), Vec2::ONE));
		m_players[0]->m_worldCamera.SetAspectRatio(newAspect);

		m_players[1]->SetNormalizedViewport(AABB2(Vec2::ZERO, Vec2(1.f, .5f)));
		m_players[1]->m_worldCamera.SetAspectRatio(newAspect);
	}
}

void Game::LoadAssets()
{
	TileDefinition::InitializeDefinitions("Data/Definitions/TileDefinitions.xml");
	ActorDefinition::InitializeDefinitions("Data/Definitions/ProjectileActorDefinitions.xml");
	WeaponDefinition::InitializeDefinitions("Data/Definitions/WeaponDefinitions.xml");
	ActorDefinition::InitializeDefinitions("Data/Definitions/ActorDefinitions.xml");
	MapDefinition::InitializeDefinitions("Data/Definitions/MapDefinitions.xml");

	SOUND_ID_BUTTON_CLICK = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("buttonClickSound", "not found"));
	SOUND_ID_MENU_MUSIC = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("mainMenuMusic", "not found"));
	SOUND_ID_GAME_MUSIC = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("gameMusic", "not found"));
	SOUND_ID_TYPEWRITER_1 = g_theAudio->CreateOrGetSound("Data/Audio/TypeWriter1.wav");
	SOUND_ID_WEAPON_PICKUP = g_theAudio->CreateOrGetSound("Data/Audio/PickupWeapon.wav");
	SOUND_ID_ERROR_MESSAGE = g_theAudio->CreateOrGetSound("Data/Audio/ErrorMessageSound.mp3");
}

void Game::InitializeTasks()
{
	m_tasks.push_back(new Task1_Start("Task1_Start"));
	m_tasks.push_back(new Task2_SpawnDemons("Task2_SpawnDemons"));
	m_tasks.push_back(new Task3_RemovePointLights("Task3_RemovePointLights"));
	m_tasks.push_back(new Task4_OutOfBoundsError("Task4_OutOfBoundsError"));
	m_tasks.push_back(new Task5_CheckForBinkeyAlerted("Task5_CheckForBinkeyAlerted"));
	m_tasks.push_back(new Task6_BinkeyRunIntoWall("Task6_BinkeyRunIntoWall"));
	m_tasks.push_back(new Task7_BinkeyRespawn("Task7_BinkeyRespawn"));
	m_tasks.push_back(new Task8_BinkeyCombat("Task8_BinkeyCombat"));
	m_tasks.push_back(new Task9_BinkeyRap("Task9_BinkeyRap"));
	m_tasks.push_back(new Task10_ShrinkRayIntro("Task10_ShrinkRayIntro"));
	m_tasks.push_back(new Task11_ShrinkRayEnd("Task11_ShrinkRayEnd"));
	m_tasks.push_back(new Task12_FOOM("Task12_FOOM"));
}

void Game::UpdateTasks()
{
	if (m_tasks.size() == 0)
	{
		return;
	}

	if (m_tasks[0]->Tick())
	{
		m_tasks[0]->OnEnd();
		delete m_tasks[0];
		m_tasks[0] = nullptr;
		m_tasks.erase(m_tasks.begin());

		if (m_tasks.size() > 0)
		{
			m_tasks[0]->OnStart();
		}
	}
	
}

void Game::RenderFoom()
{
	g_theRenderer->BeginCamera(m_entireScreenCamera);
	std::vector<Vertex_PCU> foomVerts;
	Vec2 screenDimensions = GetScreenDimensions();
	AABB2 foomBounds(Vec2::ZERO, screenDimensions);
	AddVertsForAABB2D(foomVerts, foomBounds, Rgba8::WHITE);
	g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Foom.png"));
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(foomVerts.size(), foomVerts.data());
	g_theRenderer->EndCamera(m_entireScreenCamera);
}

void Game::CompleteCurrentTask()
{
	if (m_tasks.size() > 0)
	{
		m_tasks[0]->OnEnd();
		m_tasks.erase(m_tasks.begin());
		if (m_tasks.size() > 0)
		{
			m_tasks[0]->OnStart();
		}
	}
}

void Game::UpdateGame(float deltaSeconds)
{
	for (size_t i = 0; i < m_players.size(); i++)
	{
		m_currentlyUpdatingPlayer = m_players[i];
		m_players[i]->Update(deltaSeconds);
	}
	m_currentlyUpdatingPlayer = nullptr;	
	if (g_dialogSystem->IsGameStopped())
	{
		m_currentMap->Update(0.f);
	}
	else
	{
		m_currentMap->Update(deltaSeconds);
	}
	g_dialogSystem->Update();
	UpdateTasks();
}

void Game::UpdateFoom(float deltaSeconds)
{
	m_currentErrorCooldown += deltaSeconds;
	if (m_currentErrorCooldown > m_windowsErrorCooldown)
	{
		g_theAudio->StartSound(SOUND_ID_ERROR_MESSAGE);
		m_currentErrorCooldown = 0.f;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE) || g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->ResetGame();
	}
}

void Game::Render()
{
	switch (m_currentGameMode)
	{
	case GameMode::ATTRACT:
		RenderAttract();
		break;
	case GameMode::LOBBY:
		RenderLobby();
		break;
	case GameMode::PLAYING:
		RenderGame();
		break;
	case GameMode::FOOM:
		RenderFoom();
		break;
	default:
		break;
	}
}

void Game::ShutDown()
{
	DebugRenderClear();
	for (size_t i = 0; i < m_maps.size(); i++)
	{
		delete m_maps[i];
	}
	for (size_t i = 0; i < m_players.size(); i++)
	{
		delete m_players[i];
	}
}

