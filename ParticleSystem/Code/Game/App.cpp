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
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_impl_win32.h"
#include "ThirdParty/imgui/imgui_impl_dx11.h"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/ComputeShader.hpp"
#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Game/Player.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"

App* g_theApp = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_theWindow = nullptr;
RandomNumberGenerator* g_randGen = nullptr;
Game* g_theGame = nullptr;

App::App()
{ }

App::~App()
{ }

void App::Run()
{
	while (!m_isQuitting)
	{
		double beforeFrame = GetCurrentTimeSeconds();
		RunFrame();
		m_lastFrameTime = GetCurrentTimeSeconds() - beforeFrame;

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
	windowConfig.m_windowTitle = "ParticleSystem";
	windowConfig.m_clientAspect = 2.f;
	windowConfig.m_screenHeight = 8.f;
	windowConfig.m_screenWidth = 16.f;
	windowConfig.m_enableIMGUI = true;
	windowConfig.m_isFullscreen = true;
	g_theWindow = new Window(windowConfig);

	g_theInput->m_config.m_window = g_theWindow;

	RenderConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	renderConfig.m_enableImgui = true;
	renderConfig.m_hdrEnabled = true;
	renderConfig.m_hdrExposure = 1.f;

	g_theRenderer = new Renderer( renderConfig );

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_devConsoleCamera.SetOrthographicView( Vec2::ZERO, Vec2(g_theWindow->GetConfig().m_screenWidth, g_theWindow->GetConfig().m_screenHeight) );
	devConsoleConfig.m_devConsoleCamera.m_mode = Camera::eMode_Orthographic;
	devConsoleConfig.m_linesToDisplay = 38.5f;
	devConsoleConfig.m_fontFilePath = "Data/Fonts/SquirrelFixedFont";
	devConsoleConfig.m_defaultRenderer = g_theRenderer;
	g_theDevConsole = new DevConsole(devConsoleConfig);

	AudioConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );

	ParticleSystemConfig particleConfig;
	particleConfig.m_spriteAtlasResolution = IntVec2(8192, 16384);
	g_theParticleSystem = new ParticleSystem(particleConfig);

	g_theGame = new Game( this );
	m_clock = new Clock();

	SetupDearImGuiContext();

	//Start up engine subsystems and game
	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theInput->StartUp();
	g_theWindow->StartUp();
	g_theRenderer->StartUp();
	g_theAudio->StartUp();
	g_theParticleSystem->Startup();
	g_theGame->StartUp();

	DebugRenderConfig config;
	config.m_renderer = g_theRenderer;

	DebugRenderSystemStartup(config);

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
	g_theEventSystem->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();
	g_theParticleSystem->BeginFrame();
	Clock::TickSystemClock();
	DebugRenderBeginFrame();

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void App::Update(float deltaSeconds)
{
	HandleMouseMode();
	g_theGame->Update(deltaSeconds);
}


void App::Render()
{
	ImGui::Render();
	g_theRenderer->ClearScreen(Rgba8(15,15,15,255));
	g_theGame->Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
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
	g_theParticleSystem->EndFrame();
	g_theGame->EndFrame();
	ImGuiEndFrame();
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
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "W\t\t\t\t\t- Move Forward");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "A\t\t\t\t\t- Strafe Left");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "S\t\t\t\t\t- Move Back");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "D\t\t\t\t\t- Strafe Right");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Up\t\t\t\t- Pitch Up");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Down\t\t- Pitch Down");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Left\t\t- Turn Left");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Right\t- Turn Right");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Escape- Exit Game");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Space\t- Start Game");
}

void App::HandleMouseMode()
{
	if (g_theDevConsole->GetMode() != DevConsoleMode::HIDDEN || !g_theWindow->IsWindowFocus() || !g_theGame->m_controllingPlayer)
	{
		g_theInput->SetCursorMode(false, false);
	}
	else
	{
		g_theInput->SetCursorMode(true, true);
	}
	
}

void App::SetupDearImGuiContext()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
}

void App::ImGuiEndFrame()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Update and Render additional Platform Windows
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
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

void App::Shutdown()
{
	g_theGame->ShutDown();
	g_theParticleSystem->Shutdown();
	g_theAudio->Shutdown();
	g_theRenderer->ShutDown();
	g_theWindow->Shutdown();
	ImGui::DestroyContext();
	g_theInput->ShutDown();
	g_theDevConsole->Shutdown();
	g_theEventSystem->Shutdown();
	
	delete g_randGen;
	delete g_theGame;
	delete g_theParticleSystem;
	delete g_theAudio;
	delete g_theInput;
	delete g_theRenderer;
	delete g_theDevConsole;
	delete g_theEventSystem;
	delete m_clock;
}