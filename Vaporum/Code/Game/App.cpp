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
#include "Engine/Network/NetSystem.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Game/UnitDefinition.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"

Game* g_theGame = nullptr;
App* g_theApp = nullptr;
Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
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


void App::StartUp(std::string const& commandLineArgs)
{
	EventArgs defaultGameConfigArgs;
	defaultGameConfigArgs.SetValue("File", "Data/GameConfig.xml");
	LoadGameConfig(defaultGameConfigArgs);

	//Create engine subsystems and game
	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);
	g_theEventSystem->Startup();

	g_theEventSystem->SubscribeEventCallbackFunction("Quit", App::QuitGame);
	g_theEventSystem->SubscribeEventCallbackFunction("LoadGameConfig", App::LoadGameConfig);
	if (commandLineArgs != "")
	{
		g_theEventSystem->Execute(commandLineArgs);
	}

	InputConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_windowTitle = g_gameConfigBlackboard.GetValue("windowTitle", "Vaporum");
	windowConfig.m_clientAspect = g_gameConfigBlackboard.GetValue("windowAspect", 2.f);
	windowConfig.m_isFullscreen = g_gameConfigBlackboard.GetValue("windowFullscreen", false);
	windowConfig.m_size = g_gameConfigBlackboard.GetValue("windowSize", IntVec2(-1, -1));

	windowConfig.m_position = g_gameConfigBlackboard.GetValue("windowPosition", IntVec2(-1, -1));

	g_theWindow = new Window(windowConfig);
	g_theWindow->StartUp();

	g_theInput->m_config.m_window = g_theWindow;

	RenderConfig renderConfig;
	renderConfig.m_hdrEnabled = true;
	renderConfig.m_hdrExposure = 1.f;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer( renderConfig );

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_devConsoleCamera.SetOrthographicView( Vec2::ZERO, Vec2((float)g_theWindow->GetConfig().m_screenWidth, (float)g_theWindow->GetConfig().m_screenHeight) );
	devConsoleConfig.m_devConsoleCamera.m_mode = Camera::eMode_Orthographic;
	devConsoleConfig.m_linesToDisplay = 30.f;
	devConsoleConfig.m_fontFilePath = "Data/Fonts/RobotoMonoSemiBold64";
	devConsoleConfig.m_defaultRenderer = g_theRenderer;
	g_theDevConsole = new DevConsole(devConsoleConfig);

	AudioConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );

	NetSystemConfig netSystemConfig;
	std::string netMode = ToLower(g_gameConfigBlackboard.GetValue("netMode", ""));
	if (netMode == "client")
	{
		netSystemConfig.m_mode = Mode::CLIENT;
	}
	else if (netMode == "server")
	{
		netSystemConfig.m_mode = Mode::SERVER;
	}
	netSystemConfig.m_hostAdressString = g_gameConfigBlackboard.GetValue("netHostAddress", netSystemConfig.m_hostAdressString);
	netSystemConfig.m_recvBufferSize = g_gameConfigBlackboard.GetValue("netRecvBufferSize", netSystemConfig.m_recvBufferSize);
	netSystemConfig.m_sendBufferSize = g_gameConfigBlackboard.GetValue("netSendBufferSize", netSystemConfig.m_sendBufferSize);

	g_theNetSystem = new NetSystem(netSystemConfig);

	ParticleSystemConfig particleConfig;
	particleConfig.m_spriteAtlasResolution = IntVec2(8192, 8192);
	particleConfig.m_useImGUI = false;
	g_theParticleSystem = new ParticleSystem(particleConfig);

	g_theGame = new Game( this );
	m_clock = new Clock();

	//Start up engine subsystems and game
	g_theDevConsole->Startup();
	g_theInput->StartUp();
	g_theRenderer->StartUp();
	g_theAudio->StartUp();
	g_theNetSystem->StartUp();
	g_theParticleSystem->Startup();
	g_theParticleSystem->ToggleParticleEditor(false);

	m_moonMaterial = new Material("Data/Materials/Moon.xml", g_theRenderer);
	UnitDefinition::LoadDefinitions("Data/Definitions/UnitDefinitions.xml");
	LoadTileDefinitions();
	LoadMapDefinitions();

	g_theGame->StartUp();
	/*
	if (netSystemConfig.m_mode == Mode::CLIENT)
	{
		g_theNetSystem->m_sendQueue.push_back("Hello from client!");
	}
	else if (netSystemConfig.m_mode == Mode::SERVER)
	{
		g_theNetSystem->m_sendQueue.push_back("Hello from server!");
	}
	*/
	DebugRenderConfig config;
	config.m_renderer = g_theRenderer;

	DebugRenderSystemStartup(config);	
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
	g_theNetSystem->BeginFrame();
	g_theParticleSystem->BeginFrame();
	Clock::TickSystemClock();
	DebugRenderBeginFrame();
}

void App::Update(float deltaSeconds)
{
	HandleMouseMode();
	g_theParticleSystem->Update(deltaSeconds);
	g_theGame->Update(deltaSeconds);

}


void App::Render() const
{
	g_theRenderer->ClearScreen(Rgba8(155,155,155,255));
	g_theGame->Render();
	
	
	g_theRenderer->BeginCamera(g_theGame->m_player->m_playerCamera);
	g_theRenderer->CompositeHDR();
	g_theRenderer->EndCamera(g_theGame->m_player->m_playerCamera);
	
	//g_theRenderer->RenderEmissive();
	//g_theRenderer->CompositeHDR();
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
	g_theNetSystem->EndFrame();
	g_theParticleSystem->EndFrame();
}

bool App::QuitGame(EventArgs& args)
{
	UNUSED(args);
	g_theNetSystem->m_sendQueue.push_back("PlayerQuit");
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
	g_theInput->SetCursorMode(false, false);
	/*
	if (g_theDevConsole->GetMode() != DevConsoleMode::HIDDEN || !g_theWindow->IsWindowFocus())
	{
		g_theInput->SetCursorMode(false, false);
	}
	else
	{
		g_theInput->SetCursorMode(true, true);
	}
	*/
}

void App::LoadTileDefinitions()
{
	std::string fileName = "Data/Definitions/TileDefinitions.xml";
	XmlDocument tileDefDocument;
	GUARANTEE_OR_DIE(tileDefDocument.LoadFile(fileName.c_str()) == 0, Stringf("Failed to load file %s", fileName.c_str()));
	XmlElement* rootElement = tileDefDocument.FirstChildElement("TileDefinitions");
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root element for %s", fileName.c_str()));

	for (XmlElement const* currTileDef = rootElement->FirstChildElement("TileDefinition"); currTileDef != nullptr; currTileDef = currTileDef->NextSiblingElement())
	{
		TileDefinition tileDef;
		tileDef.m_name = ParseXmlAttribute(*currTileDef, "name", "Missing");
		tileDef.m_symbol = ParseXmlAttribute(*currTileDef, "symbol", '?');
		tileDef.m_isBlocked = ParseXmlAttribute(*currTileDef, "isBlocked", false);
		g_theApp->m_tileDefinitions.push_back(tileDef);
	}
}

void App::LoadMapDefinitions()
{
	std::string fileName = "Data/Definitions/MapDefinitions.xml";
	XmlDocument mapDefDocument;
	GUARANTEE_OR_DIE(mapDefDocument.LoadFile(fileName.c_str()) == 0, Stringf("Failed to load file %s", fileName.c_str()));
	XmlElement* rootElement = mapDefDocument.FirstChildElement("MapDefinitions");
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root element for %s", fileName.c_str()));

	for (XmlElement const* currMapDef = rootElement->FirstChildElement("MapDefinition"); currMapDef != nullptr; currMapDef = currMapDef->NextSiblingElement())
	{
		MapDefinition mapDef;
		mapDef.m_name = ParseXmlAttribute(*currMapDef, "name", "missing");
		mapDef.m_gridSize = ParseXmlAttribute(*currMapDef, "gridSize", IntVec2::ZERO);
		mapDef.m_worldBoundsMin = ParseXmlAttribute(*currMapDef, "worldBoundsMin", Vec3::ZERO);
		mapDef.m_worldBoundsMax = ParseXmlAttribute(*currMapDef, "worldBoundsMax", Vec3::ZERO);
		std::string overlayShaderName = ParseXmlAttribute(*currMapDef, "overlayShader", "");
		if (overlayShaderName != "")
		{
			//mapDef.m_overlayShader = g_theRenderer->CreateOrGetShaderFromFile(overlayShaderName.c_str());
		}
		XmlElement const* tiles = currMapDef->FirstChildElement("Tiles");
		std::string tileData = tiles->GetText();
		TrimString(tileData, ' ');
		TrimString(tileData, '\n');
		for (int i = 0; i < (int)tileData.size(); i++)
		{
			IntVec2 tileCoords(i % mapDef.m_gridSize.x, i / mapDef.m_gridSize.y);
			int actualDataIndex = (mapDef.m_gridSize.y - tileCoords.y - 1) * mapDef.m_gridSize.x + tileCoords.x;
			mapDef.m_tiles.push_back(tileData[actualDataIndex]);
		}

		XmlElement const* firstUnits = currMapDef->FirstChildElement("Units");
		GetUnitData(firstUnits, mapDef);
		XmlElement const* nextUnits = firstUnits->NextSiblingElement("Units");
		GetUnitData(nextUnits, mapDef);
		g_theApp->m_mapDefinitions.push_back(mapDef);
	}
}

void App::GetUnitData(XmlElement const* unitsXML, MapDefinition& mapDef)
{
	int unitsPlayer = ParseXmlAttribute(*unitsXML, "player", 1);
	std::string unitData = unitsXML->GetText();
	TrimString(unitData, ' ');
	TrimString(unitData, '\n');
	for (int i = 0; i < (int)unitData.size(); i++)
	{
		IntVec2 tileCoords(i % mapDef.m_gridSize.x, i / mapDef.m_gridSize.y);
		int actualDataIndex = (mapDef.m_gridSize.y - tileCoords.y - 1) * mapDef.m_gridSize.x + tileCoords.x;
		UnitInMap currUnit;
		currUnit.m_unitType = unitData[actualDataIndex];
		if (currUnit.m_unitType == '.')
		{
			continue;
		}
		currUnit.m_startCoords = tileCoords;
		if (unitsPlayer == 1)
		{
			mapDef.m_player1Units.push_back(currUnit);
		}
		else
		{
			mapDef.m_player2Units.push_back(currUnit);

		}
	}
}

bool App::LoadGameConfig(EventArgs& eventArgs)
{
	std::string gameConfigFilePath = eventArgs.GetValue("File", std::string(""));
	XmlDocument effectConfigDocument;
	GUARANTEE_OR_DIE(effectConfigDocument.LoadFile(gameConfigFilePath.c_str()) == 0, Stringf("Failed to load effect config file %s", gameConfigFilePath.c_str()));
	XmlElement* rootElement = effectConfigDocument.FirstChildElement("GameConfig");
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root element for effect config %s", gameConfigFilePath.c_str()));
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*rootElement);
	return true;
}

void App::HandleSpecialCommands()
{
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
	g_theInput->ShutDown();
	g_theDevConsole->Shutdown();
	g_theEventSystem->Shutdown();
	g_theNetSystem->Shutdown();

	delete m_moonMaterial;
	delete g_randGen;
	delete g_theGame;
	delete g_theParticleSystem;
	delete g_theNetSystem;
	delete g_theAudio;
	delete g_theInput;
	delete g_theRenderer;
	delete g_theDevConsole;
	delete g_theEventSystem;
	delete m_clock;
}