#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"


App* g_theApp = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_theWindow = nullptr;
Game* g_theGame = nullptr;
RandomNumberGenerator* g_randGen = nullptr;

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

bool TestEvent1(EventArgs& eventArgs);
bool TestEvent2(EventArgs& eventArgs);
bool InterruptEvent(EventArgs& eventArgs);
bool LogSomething(EventArgs& eventArgs);


void App::StartUp()
{
	
	LoadGameConfigBlackboard();

	//Create engine subsystems and game
	InputConfig inputConfig;
	g_theInput = new InputSystem( inputConfig );

	WindowConfig windowConfig;
	const float screenWidth = g_gameConfigBlackboard.GetValue("screenWidth", 16.f);
	const float screenHeight = g_gameConfigBlackboard.GetValue("screenHeight", 8.f);
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_windowTitle = "Libra";
	windowConfig.m_clientAspect = 2.f;
	windowConfig.m_screenHeight = screenHeight;
	windowConfig.m_screenWidth= screenWidth;
	g_theWindow = new Window( windowConfig );

	g_theInput->m_config.m_window = g_theWindow;

	RenderConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer( renderConfig );

	AudioConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_linesToDisplay = 12.5f;
	devConsoleConfig.m_fontFilePath = "Data/Fonts/SquirrelFixedFont";
	devConsoleConfig.m_defaultRenderer = g_theRenderer;
	devConsoleConfig.m_devConsoleCamera.m_mode = Camera::eMode_Orthographic;
	devConsoleConfig.m_devConsoleCamera.SetOrthographicView(Vec2::ZERO, Vec2(windowConfig.m_screenWidth, windowConfig.m_screenHeight));
	g_theDevConsole = new DevConsole(devConsoleConfig);

	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	g_theGame = new Game( this );
	m_clock = new Clock();

	//Start up engine subsystems and game
	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theInput->StartUp();
	g_theWindow->StartUp();
	g_theRenderer->StartUp();
	g_theAudio->StartUp();


	g_theGame->StartUp();
	g_theEventSystem->SubscribeEventCallbackFunction("Quit", App::QuitGame);

	//get current time for updating
	m_timeLastFrame = GetCurrentTimeSeconds();

	//Sub events
	g_theEventSystem->SubscribeEventCallbackFunction("Test1", TestEvent1);

	g_theEventSystem->SubscribeEventCallbackFunction("Test2", TestEvent2);
	g_theEventSystem->SubscribeEventCallbackFunction("Test2", InterruptEvent);
	g_theEventSystem->SubscribeEventCallbackFunction("Test2", LogSomething);

}

bool InterruptEvent(EventArgs& eventArgs)
{
	UNUSED(eventArgs);
	return true;
}

bool TestEvent1(EventArgs& eventArgs)
{
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Hello!");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, eventArgs.GetValue("Message1", ""));
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, eventArgs.GetValue("Message2", ""));

	return false;
}

bool TestEvent2(EventArgs& eventArgs)
{
	Vec2 pos = eventArgs.GetValue("Pos", Vec2::ZERO);
	g_theDevConsole->AddLine(DevConsole::INFO_WARNING, "Logging position: " + Stringf("%f, %f", pos.x, pos.y));

	return false;
}

bool LogSomething(EventArgs& eventArgs)
{
	UNUSED(eventArgs);
	g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "SOMETHING!");
	return false;
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
	BeginFrame();
	HandleSpecialCommands();
	Update(m_clock->GetDeltaSeconds());
	Render();
	EndFrame();
}

void App::BeginFrame()
{
	Clock::TickSystemClock();
	g_theEventSystem->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();
}

void App::Update(float deltaSeconds)
{
	g_theGame->Update(deltaSeconds);
}


void App::Render() const
{
	g_theRenderer->ClearScreen(Rgba8(255, 0, 255, 255));
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
	return false;
}

void App::HandleSpecialCommands()
{
	//reset in victory
	if (m_pendingGameReset)
	{
		ResetGame();
		m_pendingGameReset = false;
		return;
	}

	//Quit / pause commands
	XboxController controller = g_theInput->GetController(0);

	if ((controller.IsConnected() && controller.WasButtonJustPressed(XboxController::BACK_BUTTON)) || g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		if (g_theGame->m_gameState == GAME_STATE_ATTRACT)
		{
			m_isQuitting = true;
		}
		else if (g_theGame->m_isPaused)
		{
			ResetGame();
		}
		else
		{
			g_theGame->PauseGame();
		}
	}
	

	//SLOWMO
	if (g_theInput->IsKeyDown('T'))
	{
		m_clock->SetTimeScale(.1f);
		m_isSlowMo = true;
		if (!g_theGame->m_isPaused)
		{
			g_theAudio->SetSoundPlaybackSpeed(g_theGame->m_gamePlayMusicPlayback, .5f);
		}
	}
	else if (g_theInput->WasKeyJustReleased('T'))
	{
		m_clock->SetTimeScale(1.f);
		m_isSlowMo = false;
		if (!g_theGame->m_isPaused)
		{
			g_theAudio->SetSoundPlaybackSpeed(g_theGame->m_gamePlayMusicPlayback, 1.f);
		}
	}

	if (g_theInput->IsKeyDown('Y'))
	{
		m_clock->SetTimeScale(4.f);
		m_isFastSpeed = true;
		if (!g_theGame->m_isPaused)
		{
			g_theAudio->SetSoundPlaybackSpeed(g_theGame->m_gamePlayMusicPlayback, 2.f);
		}
	}
	else if (g_theInput->WasKeyJustReleased('Y'))
	{
		m_clock->SetTimeScale(4.f);

		m_isFastSpeed = false;
		if (!g_theGame->m_isPaused)
		{
			g_theAudio->SetSoundPlaybackSpeed(g_theGame->m_gamePlayMusicPlayback, 1.f);
		}
	}

	//Debug mode
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		bool debugMode = g_gameConfigBlackboard.GetValue("debugMode", false);
		if (debugMode)
		{
			g_gameConfigBlackboard.SetValue("debugMode", "false");
		}
		else
		{
			g_gameConfigBlackboard.SetValue("debugMode", "true");
		}

	}

	//No damage
	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_noDamage = !m_noDamage;
	}

	//no clip
	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_noClip = !m_noClip;
	}

	//debug camera
	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_debugCamera = !m_debugCamera;
	}

	//Hard reset
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		ResetGame();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_theDevConsole->ToggleMode(DevConsoleMode::FULLSCREEN);
	}

	//Test events
	if (g_theInput->WasKeyJustPressed('1'))
	{
		EventArgs args;
		args.SetValue("Message1", "my event system works");
		args.SetValue("Message2", "give me points");
		FireEvent("Test1", args);
	}

	if (g_theInput->WasKeyJustPressed('2'))
	{
		FireEvent("Test2");
	}

	if (g_theInput->WasKeyJustPressed('3'))
	{
		UnsubscribeEventCallbackFunction("Test1", TestEvent1);
	}

	if (g_theInput->WasKeyJustPressed('4'))
	{
		SubscribeEventCallbackFunction("Test1", LogSomething);
	}

	if (g_theInput->WasKeyJustPressed('5'))
	{
		g_theDevConsole->Execute("Test1 Message1=Working Message2=StillWorking\nTest2 Empty= Pos=4.43,6.435\n");
	}
}

void App::ResetGame()
{
	g_theGame->ShutDown();
	delete g_theGame;
	g_theGame = new Game(this);
 	g_theGame->StartUp();
}

void App::Shutdown()
{
	g_theGame->ShutDown();
	g_theEventSystem->Shutdown();
	g_theDevConsole->Shutdown();
	g_theAudio->Shutdown();
	g_theRenderer->ShutDown();
	g_theWindow->Shutdown();
	g_theInput->ShutDown();

	delete g_randGen;
	delete g_theGame;
	delete g_theEventSystem;
	delete g_theDevConsole;
	delete g_theAudio;
	delete g_theInput;
	delete g_theRenderer;
	delete m_clock;
}