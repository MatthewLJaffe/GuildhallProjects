#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Game/VisualTest.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"

App* g_theApp = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
Window* g_theWindow = nullptr;
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


void App::StartUp()
{
	//Create engine subsystems and game
	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	InputConfig inputConfig;
	g_theInput = new InputSystem( inputConfig );

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_windowTitle = "MathVisualTest";
	windowConfig.m_clientAspect = 2.f;
	windowConfig.m_screenHeight = 100.f;
	windowConfig.m_screenWidth = 200.f;
	g_theWindow = new Window(windowConfig);

	RenderConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer( renderConfig );

	g_theInput->m_config.m_window = g_theWindow;

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_devConsoleCamera.SetOrthographicView(Vec2::ZERO, Vec2(windowConfig.m_screenWidth, windowConfig.m_screenHeight));
	devConsoleConfig.m_devConsoleCamera.m_mode = Camera::eMode_Orthographic;
	devConsoleConfig.m_linesToDisplay = 38.5f;
	devConsoleConfig.m_fontFilePath = "Data/Fonts/SquirrelFixedFont";
	devConsoleConfig.m_defaultRenderer = g_theRenderer;
	g_theDevConsole = new DevConsole(devConsoleConfig);

	m_theGame = new Game( this );
	m_gameClock = new Clock();

	//Start up engine subsystems and game
	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theInput->StartUp();
	g_theWindow->StartUp();
	g_theRenderer->StartUp();
	DebugRenderConfig config;
	config.m_renderer = g_theRenderer;
	DebugRenderSystemStartup(config);

	m_theGame->StartUp();

	//A3 Testing
	/*
	g_theEventSystem->SubscribeEventCallbackObjectMethod("Quit", *this, &App::HandleQuitRequested);
	g_theEventSystem->SubscribeEventCallbackObjectMethod("Hello", *this, &App::LogHello);

	er1 = new TestEventRecipient(1);
	er2 = new TestEventRecipient(2);

	//HCIS TESTING
	//constructors
	HCIS defaultConstructed;
	HCIS copyFrom = "I am a copy";
	HCIS copyConstructed(copyFrom);
	HCIS cStrConstructed("C String");
	HCIS strConstructed(std::string("std String"));

	//operators
	HCIS lessThan("Aaaa");
	HCIS greaterThan("Bbbbb");
	GUARANTEE_OR_DIE(lessThan < greaterThan, "< test failed");

	HCIS equalTo("Aaaa");
	GUARANTEE_OR_DIE(lessThan == equalTo, "== test failed");
	GUARANTEE_OR_DIE(lessThan == "Aaaa", "== test failed");
	GUARANTEE_OR_DIE(lessThan == std::string("Aaaa"), "== test failed");

	GUARANTEE_OR_DIE(lessThan != greaterThan, "!= test failed");
	GUARANTEE_OR_DIE(lessThan != "sdlfkjkls", "!= test failed");
	GUARANTEE_OR_DIE(lessThan != std::string(""), "!= test failed");

	HCIS thing = "Thing 1";
	thing = "Thing 1";
	thing = HCIS("Thing 2");
	thing = std::string("Thing 3");

	//NAMED PROPERTIES TESTING
	std::string lastName("Eiserloh");
	std::string firstName("Squirrel");

	NamedProperties employmentInfo;
	employmentInfo.SetValue("Something?", true);

	NamedProperties p;
	p.SetValue("Height", 1.93f);
	p.SetValue("Age", std::string("50"));
	p.SetValue("Lives", 3);
	p.SetValue("IsMarried", true);
	p.SetValue("Position", Vec2(3.5f, 6.2f));
	p.SetValue("EyeColor", Rgba8(77, 38, 23));
	p.SetValue("LastName", "lastName");
	p.SetValue("FirstName", firstName);
	p.SetValue("EmployeeInfo", employmentInfo);

	p.SetValue("GPA", 3.14f);
	p.SetValue("YearBorn", "1973");
	p.SetValue("StreetAddress", "2210");

	float height = p.GetValue("height", 1.75f);
	int health = p.GetValue("health", 100);
	std::string missing = "Missing!";
	firstName = p.GetValue("FirstName", "missing");
	int age = p.GetValue("Age", 0);
	int lives = p.GetValue("lives", 6);
	lastName = p.GetValue("LastName", missing);
	//int height2 = p.GetValue("Height", 76); //error if uncommented

	NamedProperties employerInfo = p.GetValue("employeeinfo", NamedProperties());
	bool shouldBeTrue = employerInfo.GetValue("Something?", false);

	float GPA = p.GetValue("GPa", 0.f);
	int yearBorn = p.GetValue("YearBorn", 2000);
	//Vec2* pointer = p.GetValue("StreetAddress", nullptr); //error if uncommented
	
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, Stringf("Height %.2f", p.GetValue("Height", 1.f)));
	*/
}

TestEventRecipient::TestEventRecipient(int id)
	: m_id(id)
{
	g_theEventSystem->SubscribeEventCallbackObjectMethod("Test", *this, &TestEventRecipient::LogEventRecipient);
}

bool TestEventRecipient::LogEventRecipient(EventArgs& args)
{
	UNUSED(args);
	if (args.GetValue("id", 0) == m_id)
	{
		g_theDevConsole->AddLine(Rgba8::GREEN, Stringf("EventRecipient %d", m_id));
	}
	return false;
}


void App::RunFrame()
{
	BeginFrame();
	HandleSpecialCommands();
	Update(m_gameClock->GetDeltaSeconds());
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
	Clock::TickSystemClock();
	DebugRenderBeginFrame();
}

void App::Update(float deltaSeconds)
{
	m_theGame->Update(deltaSeconds);
	if (g_theInput->WasKeyJustPressed('1'))
	{
		
	}
	if (g_theInput->WasKeyJustPressed('2'))
	{
		g_theEventSystem->UnsubscribeEventObjectMethod("Quit", *this, &App::HandleQuitRequested);
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		g_theEventSystem->UnsubscribeAllEventsForObject(*this);
	}
	if (g_theInput->WasKeyJustPressed('4'))
	{
		XmlDocument effectConfigDocument;
		GUARANTEE_OR_DIE(effectConfigDocument.LoadFile("Data/Commands/TestCommandScript.xml") == 0, Stringf("Failed to load file"));
		XmlElement* rootElement = effectConfigDocument.FirstChildElement("TestCommandScript");
		GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root element"));
		g_theDevConsole->ExecuteXmlCommandScriptNode(*rootElement);
	}
	if (g_theInput->WasKeyJustPressed('5'))
	{
		g_theDevConsole->ExecuteXmlCommandScript("Data/Commands/TestCommandScript.xml");
	}
	if (g_theInput->WasKeyJustPressed('6'))
	{
		delete er1;
		er1 = nullptr;

		delete er2;
		er2 = nullptr;
	}
}


void App::Render() const
{
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
	m_theGame->Render();
	DebugRenderScreen(m_theGame->m_screenCamera);
	g_theDevConsole->Render(AABB2(g_theDevConsole->m_config.m_devConsoleCamera.GetOrthoBottomLeft(), g_theDevConsole->m_config.m_devConsoleCamera.GetOrthoTopRight()));
}


void App::EndFrame()
{
	g_theEventSystem->EndFrame();
	g_theDevConsole->EndFrame();
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	DebugRenderEndFrame();
}

bool App::HandleQuitRequested(EventArgs& args)
{
	UNUSED(args);
	m_isQuitting = true;
	return true;
}

bool App::LogHello(EventArgs& args)
{
	UNUSED(args);
	g_theDevConsole->AddLine(Rgba8::CYAN, "Logging Hello!");
	return true;
}

Game* App::GetGame()
{
	return m_theGame;
}

void App::HandleSpecialCommands()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_isQuitting = true;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_debugMode = !m_debugMode;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		DecrementVisualTest();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		AdvanceVisualTest();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		ResetCurrentTest();
	}

	//clock stuff
	if (g_theInput->WasKeyJustPressed('T'))
	{
		m_gameClock->SetTimeScale(.1f);
	}
	if (g_theInput->WasKeyJustReleased('T'))
	{
		m_gameClock->SetTimeScale(1.f);
	}
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_gameClock->SetSingleFrame();
	}
	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_gameClock->TogglePause();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_theDevConsole->ToggleMode(DevConsoleMode::FULLSCREEN);
	}
}

void App::ResetCurrentTest()
{
	m_theGame->m_visualTests[(int)m_theGame->m_visualTestType]->RandomizeTest();
}

void App::AdvanceVisualTest()
{
	int currVisualTestIdx = (int)(m_theGame->m_visualTestType);
	currVisualTestIdx++;
	if (currVisualTestIdx >= VisualTestType::NUM_VISUAL_TESTS)
	{
		currVisualTestIdx = 0;
	}
	m_theGame->m_visualTestType = (VisualTestType)currVisualTestIdx;
	m_theGame->m_visualTests[m_theGame->m_visualTestType]->InitializeTest();
}

void App::DecrementVisualTest()
{
	int currVisualTestIdx = (int)(m_theGame->m_visualTestType);
	currVisualTestIdx--;

	if (currVisualTestIdx < 0)
	{
		currVisualTestIdx = ((int)VisualTestType::NUM_VISUAL_TESTS) - 1;
	}
	m_theGame->m_visualTestType = (VisualTestType)currVisualTestIdx;
	m_theGame->m_visualTests[m_theGame->m_visualTestType]->InitializeTest();
}

Clock* App::GetGameClock()
{
	return m_gameClock;
}

void App::Shutdown()
{
	m_theGame->ShutDown();
	DebugRenderSystemShutdown();
	g_theRenderer->ShutDown();
	g_theWindow->Shutdown();
	g_theInput->ShutDown();
	g_theDevConsole->Shutdown();
	g_theEventSystem->Shutdown();

	delete g_randGen;
	delete m_theGame;
	delete g_theInput;
	delete g_theRenderer;
	delete g_theDevConsole;
	delete g_theEventSystem;
	delete m_gameClock;
}
