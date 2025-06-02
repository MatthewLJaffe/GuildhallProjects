#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"


App* g_theApp = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_theWindow = nullptr;
RandomNumberGenerator* g_randGen = nullptr;

//sounds
SoundID SOUND_ID_BULLET_SHOOT;
SoundID SOUND_ID_EXPLOSION;
SoundID SOUND_ID_HIT;
SoundID SOUND_ID_LOSE;
SoundID SOUND_ID_NEW_WAVE;
SoundID SOUND_ID_RESPAWN;
SoundID SOUND_ID_SPAWN_WAVE;
SoundID SOUND_ID_STARTUP;
SoundID SOUND_ID_WIN;
SoundID SOUND_ID_THRUST;
SoundID SOUND_ID_LASER;


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
	g_theInput = new InputSystem( inputConfig );

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_windowTitle = "SD1-A4: Starship Gold";
	windowConfig.m_clientAspect = 2.f;
	windowConfig.m_screenHeight = 8.f;
	windowConfig.m_screenWidth = 16.f;
	windowConfig.m_isFullscreen = true;
	g_theWindow = new Window(windowConfig);

	g_theInput->m_config.m_window = g_theWindow;

	RenderConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer( renderConfig );

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_devConsoleCamera.m_mode = Camera::eMode_Orthographic;
	devConsoleConfig.m_devConsoleCamera.SetOrthographicView(Vec2::ZERO, Vec2(windowConfig.m_screenWidth, windowConfig.m_screenHeight), 0.f, 1.f);
	devConsoleConfig.m_linesToDisplay = 38.5f;
	devConsoleConfig.m_fontFilePath = "Data/Fonts/SquirrelFixedFont";
	devConsoleConfig.m_defaultRenderer = g_theRenderer;
	g_theDevConsole = new DevConsole(devConsoleConfig);

	AudioConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );

	m_theGame = new Game( this );
	m_clock = new Clock();

	//Start up engine subsystems and game
	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theInput->StartUp();
	g_theWindow->StartUp();
	g_theRenderer->StartUp();
	g_theAudio->StartUp();
	m_theGame->StartUp();

	g_theEventSystem->SubscribeEventCallbackFunction("Quit", App::QuitGame);

	//initialize sound ids
	SOUND_ID_BULLET_SHOOT = g_theAudio->CreateOrGetSound("Data/Audio/Laser_Shoot.wav");
	SOUND_ID_EXPLOSION = g_theAudio->CreateOrGetSound("Data/Audio/Explosion.wav");
	SOUND_ID_HIT = g_theAudio->CreateOrGetSound("Data/Audio/Hit_Hurt.wav");
	SOUND_ID_LOSE = g_theAudio->CreateOrGetSound("Data/Audio/LoseNoise.wav");
	SOUND_ID_NEW_WAVE = g_theAudio->CreateOrGetSound("Data/Audio/NewWave.wav");
	SOUND_ID_RESPAWN = g_theAudio->CreateOrGetSound("Data/Audio/Respawn.wav");
	SOUND_ID_SPAWN_WAVE = g_theAudio->CreateOrGetSound("Data/Audio/SpawnWave.wav");
	SOUND_ID_STARTUP = g_theAudio->CreateOrGetSound("Data/Audio/StartupSound.wav");
	SOUND_ID_WIN = g_theAudio->CreateOrGetSound("Data/Audio/WinNoise.wav");
	SOUND_ID_THRUST = g_theAudio->CreateOrGetSound("Data/Audio/Thrust.wav");
	SOUND_ID_LASER = g_theAudio->CreateOrGetSound("Data/Audio/BigLaser.wav");
	AddControlsToDevConsole();
	g_theEventSystem->SubscribeEventCallbackFunction("SetTimeScale", App::SetClockTimeScale);
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
	m_theGame->Update(deltaSeconds);
}


void App::Render() const
{
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
	m_theGame->Render();
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
	m_theGame->DeleteGarbageEntities();
}

void App::AddControlsToDevConsole()
{
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Controls:");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Controller: Left stick : rotate playership / boost");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "A Button : Fire bullet");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Start Button : Respawn");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Right Trigger : Fire missiles");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Left Trigger : Fire laser");

	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Keyboard");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "S: rotate left");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "F : rotate right");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "E : boost");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "SPACE : fire bullet");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "N : Respawn");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "W : fire laser beam");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "R : fire missiles");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Escape : Quit to menu / quit game");

	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Engine controls :");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "F1: Developer Mode");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "F8 : Hard reset");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "T : 1 / 10th speed");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "O : Step 1 frame");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "P : Pause / unpause");

}

bool App::QuitGame(EventArgs& args)
{
	UNUSED(args);
	g_theApp->m_isQuitting = true;
	return false;
}

bool App::SetClockTimeScale(EventArgs& args)
{
	
	float newTimeScale = args.GetValue("TimeScale", -1.f);
	if (newTimeScale == -1.f)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Invocatoin: SetTimeScale TimeScale=(TimeScaleValue)");
	}
	else
	{
		g_theApp->m_clock->SetTimeScale(newTimeScale);
	}
	return true;
}

void App::HandleQuitRequested()
{
	m_isQuitting = true;
}

void App::HandleSpecialCommands()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_theDevConsole->ToggleMode(DevConsoleMode::FULLSCREEN);
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
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		if (m_theGame->IsAttractScreen())
			m_isQuitting = true;
		else
		{
			ResetGame();
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		//toggle debug mode
		m_debugMode = !m_debugMode;
	}
	//F8
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		//Hard reset
		ResetGame();
	}
}

void App::ResetGame()
{
	m_theGame->ShutDown();
	delete m_theGame;
	m_theGame = new Game(this);
 	m_theGame->StartUp();
}

void App::Shutdown()
{
	m_theGame->ShutDown();
	g_theAudio->Shutdown();
	g_theRenderer->ShutDown();
	g_theWindow->Shutdown();
	g_theInput->ShutDown();
	g_theDevConsole->Shutdown();
	g_theEventSystem->Shutdown();

	delete g_randGen;
	delete m_theGame;
	delete g_theAudio;
	delete g_theInput;
	delete g_theRenderer;
	delete g_theDevConsole;
	delete g_theEventSystem;
	delete m_clock;
}

bool App::HandleKeyPressed(unsigned char keyCode)
{
	return g_theInput->HandleKeyPressed(keyCode);
}

bool App::HandleKeyReleased(unsigned char keyCode)
{
	return g_theInput->HandleKeyReleased(keyCode);
}