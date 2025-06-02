#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Game/Player.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/DialogSystem.hpp"
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)


Game* g_theGame = nullptr;
App* g_theApp = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_theWindow = nullptr;
RandomNumberGenerator* g_randGen = nullptr;
BitmapFont* g_bitMapFont = nullptr;
DialogSystem* g_dialogSystem = nullptr;

App::App()
{ }

App::~App()
{ }

void App::Run()
{
	while (!m_isQuitting)
	{
		RunFrame();
	}
}


void App::StartUp()
{
	//Create engine subsystems and game
	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	InputConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_windowTitle = "Doomenstein";
	windowConfig.m_clientAspect = 2.f;
	windowConfig.m_screenHeight = 8.f;
	windowConfig.m_screenWidth = 16.f;
	g_theWindow = new Window(windowConfig);

	g_theInput->m_config.m_window = g_theWindow;

	RenderConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer( renderConfig );

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_devConsoleCamera.SetOrthographicView( Vec2::ZERO, Vec2(windowConfig.m_screenWidth, windowConfig.m_screenHeight) );
	devConsoleConfig.m_devConsoleCamera.m_mode = Camera::eMode_Orthographic;
	devConsoleConfig.m_linesToDisplay = 38.5f;
	devConsoleConfig.m_fontFilePath = "Data/Fonts/SquirrelFixedFont";
	devConsoleConfig.m_defaultRenderer = g_theRenderer;
	g_theDevConsole = new DevConsole(devConsoleConfig);

	AudioConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );

	g_theGame = new Game( this );
	m_clock = new Clock();

	LoadGameConfigBlackboard();

	//Start up engine subsystems and game
	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theInput->StartUp();
	g_theWindow->StartUp();
	g_theRenderer->StartUp();
	g_theAudio->StartUp();
	g_theGame->StartUp();

	DebugRenderConfig config;
	config.m_renderer = g_theRenderer;

	DebugRenderSystemStartup(config);

	//get current time for updating
	g_theEventSystem->SubscribeEventCallbackFunction("Quit", App::QuitGame);
	PrintControlsToConsole();
}

void App::LoadGameConfigBlackboard()
{
	XmlDocument gameConfigDocument;
	GUARANTEE_OR_DIE(gameConfigDocument.LoadFile("Data/GameConfig.xml") == 0, "Failed to load Data/GameConfig.xml");
	XmlElement* gameConfigElement = gameConfigDocument.FirstChildElement("GameConfig");
	GUARANTEE_OR_DIE(gameConfigElement != nullptr, "Could not get the root GameConfig element from Data/GameConfig.xml");
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*gameConfigElement);
}

void App::RunFrame()
{
	if (m_artificialLag > 1)
	{
		Sleep(m_artificialLag);
	}
	BeginFrame();
	HandleSpecialCommands();
	Update(m_clock->GetDeltaSeconds());
	Render();
	EndFrame();
}

void App::BeginFrame()
{
	g_theEventSystem->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();
	Clock::TickSystemClock();
	DebugRenderBeginFrame();
}

void App::Update(float deltaSeconds)
{
	HandleMouseMode();
	g_theGame->Update(deltaSeconds);
}


void App::Render()
{
	g_theRenderer->ClearScreen(Rgba8(155,155,155,255));
	g_theGame->Render();
	g_theDevConsole->Render(AABB2(g_theDevConsole->m_config.m_devConsoleCamera.GetOrthoBottomLeft(), g_theDevConsole->m_config.m_devConsoleCamera.GetOrthoTopRight()));
}


void App::EndFrame()
{
	g_theEventSystem->EndFrame();
	g_theDevConsole->EndFrame();
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	g_theAudio->EndFrame();
}

bool App::QuitGame(EventArgs& args)
{
	UNUSED(args);
	g_theApp->m_isQuitting = true;
	return true;
}

void App::PrintControlsToConsole()
{
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Controls");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Mouse		- Aim");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "W / S		- Move");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "A / D		- Strafe");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "SPACE		- Advance Dialog");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Shift		- Sprint");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "1						- Weapon 1");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "2						- Weapon 2");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "P						- Pause");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "O						- Step Frame");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "F						- Toggle Free Camera");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "~						- Open Dev Console");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Escape	- Exit Game");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Space		- Start Game");
}

void App::HandleMouseMode()
{
	if (g_theDevConsole->GetMode() != DevConsoleMode::HIDDEN || !g_theWindow->IsWindowFocus() || g_theGame->m_currentGameMode == GameMode::ATTRACT)
	{
		g_theInput->SetCursorMode(false, false);
	}
	else
	{
		g_theInput->SetCursorMode(true, true);
	}
	
}

void App::HandleSpecialCommands()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		if (g_theGame->m_currentGameMode == GameMode::PLAYING)
		{
			g_theAudio->StopSound(g_theGame->m_gameMusicPlayback);
			ResetGame();
		}
		else if (g_theGame->m_currentGameMode == GameMode::ATTRACT)
		{
			m_isQuitting = true;
		}
	}
	if (g_theInput->WasKeyJustPressed('T'))
	{
		m_clock->SetTimeScale(.1f);
	}
	if (g_theInput->WasKeyJustReleased('T'))
	{
		m_clock->SetTimeScale(1.f);
	}
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_clock->SetSingleFrame();
	}
	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_clock->TogglePause();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_theDevConsole->ToggleMode(DevConsoleMode::FULLSCREEN);
	}
	if (g_theInput->WasKeyJustPressed('F'))
	{
		if (g_theGame->m_players.size() == 1)
		{
			g_theGame->m_players[0]->m_freeFlyCameraMode = !g_theGame->m_players[0]->m_freeFlyCameraMode;
		}
	}
	if (g_theInput->WasKeyJustPressed('N'))
	{
		if (g_theGame->m_currentMap != nullptr && g_theGame->m_currentGameMode == GameMode::PLAYING)
		{
			//g_theGame->m_currentmap->DebugPossessNext();
		}
	}
}

void App::ResetGame()
{
	g_theGame->ShutDown();
	delete g_theGame;
	g_theGame = new Game(this);
 	g_theGame->StartUp();
}

Clock* App::GetGameClock()
{
	return m_clock;
}

void App::SetArtificialLag(unsigned long lag)
{
	m_artificialLag = lag;
}

void App::Shutdown()
{
	TileDefinition::ClearDefinitions();
	MapDefinition::ClearDefinitions();

	g_theGame->ShutDown();
	g_theAudio->Shutdown();
	g_theRenderer->ShutDown();
	g_theWindow->Shutdown();
	g_theInput->ShutDown();
	g_theDevConsole->Shutdown();
	g_theEventSystem->Shutdown();
	
	delete g_randGen;
	delete g_theGame;
	delete g_theAudio;
	delete g_theInput;
	delete g_theRenderer;
	delete g_theDevConsole;
	delete g_theEventSystem;
	delete m_clock;
}