#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"
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
#include "Engine/Core/JobSystem.hpp"

App* g_theApp = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_theWindow = nullptr;
RandomNumberGenerator* g_randGen = nullptr;
Game* g_theGame = nullptr;

Rgba8 skyColor;

App::App()
{ }

App::~App()
{ }

void App::Run()
{
	while (!m_isQuitting)
	{
		double beforeFrameTime = GetCurrentTimeSeconds();
		RunFrame();
		double afterFrameTime = GetCurrentTimeSeconds();
		m_timeLastFrameMS = (afterFrameTime - beforeFrameTime) * 1000.0;
	}
}


void App::StartUp()
{
	//Create engine subsystems and game
	LoadGameConfigBlackboard();

	JobSystemConfig jobSystemConfig;
	jobSystemConfig.m_threadsToCreate = (int)std::thread::hardware_concurrency() - 1;
	JobworkerTypeToCreate jobworkerTypeIO;
	jobworkerTypeIO.m_jobWorkersOfType = 1;
	jobworkerTypeIO.m_jobWokerTypeBitmask = JOB_WRITE_CHUNK_TO_DISK | JOB_READ_CHUNK_FROM_DISK;
	jobSystemConfig.m_jobWorkerTypesToCreate.push_back(jobworkerTypeIO);
	JobworkerTypeToCreate jobWorkerTypeNonIO;
	jobWorkerTypeNonIO.m_jobWorkersOfType = jobSystemConfig.m_threadsToCreate - 1;
	jobWorkerTypeNonIO.m_jobWokerTypeBitmask = ~jobworkerTypeIO.m_jobWokerTypeBitmask;
	jobSystemConfig.m_jobWorkerTypesToCreate.push_back(jobWorkerTypeNonIO);

	g_theJobSystem = new JobSystem(jobSystemConfig);

	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	InputConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_windowTitle = "SimpleMiner";
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

	//Start up engine subsystems
	g_theJobSystem->StartUp();
	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theInput->StartUp();
	g_theWindow->StartUp();
	g_theRenderer->StartUp();
	g_theAudio->StartUp();

	DebugRenderConfig config;
	config.m_renderer = g_theRenderer;

	//Start up game
	DebugRenderSystemStartup(config);

	g_theGame->StartUp();

	//get current time for updating
	g_theEventSystem->SubscribeEventCallbackFunction("Quit", App::QuitGame);
	PrintControlsToConsole();
}

void App::RunFrame()
{
	BeginFrame();
	HandleSpecialCommands();
	Update(m_clock->GetDeltaSeconds());
	Render();
	EndFrame();
}

void App::BeginFrame()
{
	g_theJobSystem->BeginFrame();
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


void App::Render() const
{
	g_theRenderer->ClearScreen(SkyColor);
	g_theGame->Render();
	g_theDevConsole->Render(AABB2(g_theDevConsole->m_config.m_devConsoleCamera.GetOrthoBottomLeft(), g_theDevConsole->m_config.m_devConsoleCamera.GetOrthoTopRight()));
}


void App::EndFrame()
{
	g_theJobSystem->EndFrame();
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
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Keys");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "W\t\t\t\t\t\t- Move Forward");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "A\t\t\t\t\t\t- Strafe Left");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "S\t\t\t\t\t\t- Move Back");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "D\t\t\t\t\t\t- Strafe Right");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Q\t\t\t\t\t\t- Move Up");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "E\t\t\t\t\t\t- Move Down");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Space\t\t- Move Fast");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "F1\t\t\t\t\t- Debug Mode");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "F7\t\t\t\t\t- Chunk Step Mode");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "F8\t\t\t\t\t- Rebuild Chunks");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Escape\t- Exit Game");
}

void App::HandleMouseMode()
{
	if (g_theDevConsole->GetMode() != DevConsoleMode::HIDDEN || !g_theWindow->IsWindowFocus())
	{
		g_theInput->SetCursorMode(false, false);
	}
	else
	{
		g_theInput->SetCursorMode(true, true);
	}
	
}

void App::LoadGameConfigBlackboard()
{
	XmlDocument gameConfigDocument;
	GUARANTEE_OR_DIE(gameConfigDocument.LoadFile("Data/GameConfig.xml") == 0, "Failed to load Data/GameConfig.xml");
	XmlElement* gameConfigElement = gameConfigDocument.FirstChildElement("GameConfig");
	GUARANTEE_OR_DIE(gameConfigElement != nullptr, "Could not get the root GameConfig element from Data/GameConfig.xml");
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*gameConfigElement);
}

void App::HandleSpecialCommands()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_isQuitting = true;
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
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_debugMode = !m_debugMode;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		ResetGame();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_chunkStep = !m_chunkStep;
		if (m_chunkStep)
		{
			g_theWorld->RebuildWorld();
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_theDevConsole->ToggleMode(DevConsoleMode::FULLSCREEN);
	}
}

void App::ResetGame()
{
	g_theGame->ShutDown();
	delete g_theGame;
	g_theGame = new Game(this);
 	g_theGame->StartUp();
}

Clock* App::GetGameClock() const
{
	return m_clock;
}

double App::GetTimeLastFrameMS()
{
	return m_timeLastFrameMS;
}

void App::Shutdown()
{
	g_theGame->ShutDown();
	g_theAudio->Shutdown();
	g_theRenderer->ShutDown();
	g_theWindow->Shutdown();
	g_theInput->ShutDown();
	g_theDevConsole->Shutdown();
	g_theEventSystem->Shutdown();
	g_theJobSystem->Shutdown();
	
	delete g_randGen;
	delete g_theGame;
	delete g_theAudio;
	delete g_theInput;
	delete g_theRenderer;
	delete g_theDevConsole;
	delete g_theEventSystem;
	delete m_clock;
}