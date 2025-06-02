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
#include "Game/GameCommon.hpp"
#include "Game/AnimationDefinition.hpp"
#include "Engine/Math/FloatCurve.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"

App* g_theApp = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_theWindow = nullptr;
RandomNumberGenerator* g_randGen = nullptr;
BitmapFont* g_bitMapFont = nullptr;

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
	LoadGameConfigBlackboard();
	//Create engine subsystems and game
	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	InputConfig inputConfig;
	g_theInput = new InputSystem( inputConfig );

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_windowTitle = "JustShapes";
	windowConfig.m_clientAspect = 2.f;
	windowConfig.m_screenHeight = 8.f;
	windowConfig.m_screenWidth = 16.f;
	//windowConfig.m_isFullscreen = true;
	g_theWindow = new Window(windowConfig);
	g_theInput->m_config.m_window = g_theWindow;

	RenderConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer( renderConfig );

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_devConsoleCamera.m_mode = Camera::eMode_Orthographic;
	devConsoleConfig.m_devConsoleCamera.SetOrthographicView(Vec2::ZERO, Vec2(windowConfig.m_screenWidth, windowConfig.m_screenHeight));
	devConsoleConfig.m_linesToDisplay = 38.5f;
	devConsoleConfig.m_fontFilePath = "Data/Fonts/SquirrelFixedFont";
	devConsoleConfig.m_defaultRenderer = g_theRenderer;
	g_theDevConsole = new DevConsole(devConsoleConfig);

	AudioConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );

	g_theGame = new Game();
	m_clock = new Clock();

	//Start up engine subsystems and game
	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theInput->StartUp();
	g_theWindow->StartUp();
	g_theRenderer->StartUp();
	g_theAudio->StartUp();

	LoadAssets();
	g_theGame->StartUp();

	DebugRenderConfig config;
	config.m_renderer = g_theRenderer;

	DebugRenderSystemStartup(config);

	//get current time for updating
	//m_timeLastFrame = GetCurrentTimeSeconds();
	g_theEventSystem->SubscribeEventCallbackFunction("Quit", App::QuitGame);
}

void App::LoadAssets()
{
	FloatCurve::LoadCurvesFromXML("Data/Definitions/FloatCurves.xml");
	AnimationDefinition::InitializeDefinitions("Data/Definitions/AnimDefinitions.xml");
	EntityConfig::InitializeEntityConfigs();

	g_bitMapFont = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Fonts/SquirrelFixedFont");

	//SOUNDS
	SOUND_ID_TEST = g_theAudio->CreateOrGetSound("Data/Audio/TestSound.mp3");
	SOUND_ID_MENU_MUSIC = g_theAudio->CreateOrGetSound("Data/Audio/Menu.mp3");
	SOUND_ID_CLICK = g_theAudio->CreateOrGetSound("Data/Audio/Click.mp3");
	SOUND_ID_LEVEL_1_MUSIC = g_theAudio->CreateOrGetSound("Data/Audio/LongLiveTheNewFreshShort.wav");

	//TEXTURES
	TEXTURE_PLAY_BUTTON = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayButton1.png");
	TEXTURE_PLAY_BUTTON_PRESSED = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayButton2.png");
	TEXTURE_MAIN_MENU_BORDER = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MainMenuBorder.png");
	TEXTURE_HOW_TO_PLAY_BUTTON = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/HowToPlayButton1.png");
	TEXTURE_HOW_TO_PLAY_BUTTON_PRESSED = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/HowToPlayButton2.png");
	TEXTURE_SETTINGS_BUTTON = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/SettingsButton1.png");
	TEXTURE_SETTINGS_BUTTON_PRESSED = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/SettingsButton2.png");
	TEXTURE_QUIT_BUTTON = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/QuitButton1.png");
	TEXTURE_QUIT_BUTTON_PRESSED = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/QuitButton2.png");
	TEXTURE_BACK_BUTTON = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/BackButton1.png");
	TEXTURE_BACK_BUTTON_PRESSED = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/BackButton2.png");
	TEXTURE_PLUS_BUTTON = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlusButton1.png");
	TEXTURE_PLUS_BUTTON_PRESSED = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlusButton2.png");
	TEXTURE_MINUS_BUTTON = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MinusButton1.png");
	TEXTURE_MINUS_BUTTON_PRESSED = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MinusButton2.png");
	TEXTURE_HOW_TO_PLAY_BORDER = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/HowToPlayBorder.png");
	TEXTURE_SETTINGS_BORDER = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/SettingsBorder.png");

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
	g_theGame->Update(deltaSeconds);
}


void App::Render() const
{
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
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

void App::HandleSpecialCommands()
{
	if (g_theInput->WasKeyJustPressed('T'))
	{
		m_clock->SetTimeScale(.25f);
		g_theGame->SetMusicSpeed(.25f);
	}
	if (g_theInput->WasKeyJustReleased('T'))
	{
		m_clock->SetTimeScale(1.f);
		g_theGame->SetMusicSpeed(1.f);
	}
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_clock->SetSingleFrame();
	}
	if (g_theInput->WasKeyJustPressed('P'))
	{
		Pause();
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
	g_theGame = new Game();
 	g_theGame->StartUp();
}

Clock* App::GetGameClock()
{
	return m_clock;
}

void App::Pause()
{
	m_isPaused = !m_isPaused;
	m_clock->TogglePause();
	g_theGame->m_currentGameState->SetPause(m_isPaused);
}

void App::SetPause(bool pause)
{
	if (pause != m_isPaused)
	{
		Pause();
	}
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
	
	delete g_randGen;
	delete g_theGame;
	delete g_theAudio;
	delete g_theInput;
	delete g_theRenderer;
	delete g_theDevConsole;
	delete g_theEventSystem;
	delete m_clock;
}

void App::LoadGameConfigBlackboard()
{
	XmlDocument gameConfigDocument;
	GUARANTEE_OR_DIE(gameConfigDocument.LoadFile("Data/GameConfig.xml") == 0, "Failed to load Data/GameConfig.xml");
	XmlElement* gameConfigElement = gameConfigDocument.FirstChildElement("GameConfig");
	GUARANTEE_OR_DIE(gameConfigElement != nullptr, "Could not get the root GameConfig element from Data/GameConfig.xml");
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*gameConfigElement);	
}
