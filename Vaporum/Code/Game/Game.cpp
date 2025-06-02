#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Entity.hpp"
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
#include "Game/Prop.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Math/Plane3.hpp"
#include "Game/UnitDefinition.hpp"
#include "Game/Unit.hpp"
#include "Game/Widget.hpp"
#include "Game/DamageText.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"

Game::Game(App* app)
	: m_theApp(app)
{}

Game::~Game() {}

void Game::StartUp()
{
	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");

	m_attractScreen = true;
	m_player = new Player(this, Vec3::ZERO);
	m_allEntities.push_back(m_player);
	m_bitmapFont = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Fonts/RobotoMonoSemiBold128");
	m_screenCamera.SetOrthographicView(Vec2::ZERO, g_theWindow->GetClientDimensions().GetVec2());

	m_screenCamera.m_mode = Camera::eMode_Orthographic;
	m_player->m_playerCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.f, .01f, 100.f);
	m_player->m_playerCamera.m_mode = Camera::eMode_Perspective;
	m_player->m_playerCamera.SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.f, 1.f, 0.f));
	m_player->m_position = Vec3(5.f, 3.f, 7.f);
	m_player->m_orientationDegrees = EulerAngles(90.f, 60.f, 0.f);
	m_player->m_playerCamera.m_position = m_player->m_position;
	m_player->m_playerCamera.m_orientation = m_player->m_orientationDegrees;

	g_theParticleSystem->m_config.m_playerCamera = &m_player->m_playerCamera;

	Prop* plane = new Prop(this, Vec3::ZERO);
	plane->m_material = g_theApp->m_moonMaterial;
	plane->CreateQuad(Vec2(120.f, 60.f));
	plane->m_position = Vec3(0.f, 10.f, 0.f);
	m_allEntities.push_back(plane);

	g_theEventSystem->SubscribeEventCallbackFunction("LoadMap", Game::Event_LoadMap);
	g_theEventSystem->SubscribeEventCallbackFunction("PlayerReady", Game::Event_PlayerReady);
	g_theEventSystem->SubscribeEventCallbackFunction("SetFocusedHex", Game::Event_SetFocusedHex);
	g_theEventSystem->SubscribeEventCallbackFunction("StartTurn", Game::Event_StartTurn);
	g_theEventSystem->SubscribeEventCallbackFunction("SelectFocusedUnit", Game::Event_SelectFocusedUnit);
	g_theEventSystem->SubscribeEventCallbackFunction("SelectPreviousUnit", Game::Event_SelectPreviousUnit);
	g_theEventSystem->SubscribeEventCallbackFunction("SelectNextUnit", Game::Event_SelectNextUnit);
	g_theEventSystem->SubscribeEventCallbackFunction("Move", Game::Event_Move);
	g_theEventSystem->SubscribeEventCallbackFunction("Cancel", Game::Event_Cancel);
	g_theEventSystem->SubscribeEventCallbackFunction("Stay", Game::Event_Stay);
	g_theEventSystem->SubscribeEventCallbackFunction("Attack", Game::Event_Attack);
	g_theEventSystem->SubscribeEventCallbackFunction("HoldFire", Game::Event_HoldFire);
	g_theEventSystem->SubscribeEventCallbackFunction("PlayerQuit", Game::Event_PlayerQuit);
	g_theEventSystem->SubscribeEventCallbackFunction("EndTurn", Game::Event_EndTurn);

	WidgetConfig startMenuWidgetConfig;
	startMenuWidgetConfig.m_borderSize = Vec2::ZERO;
	m_startScreenWidget = new Widget(nullptr, startMenuWidgetConfig);
	
	WidgetConfig startImageConfig;
	startImageConfig.m_borderSize = Vec2::ZERO;
	startImageConfig.m_panelColor = Rgba8::WHITE;
	startImageConfig.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Logo.png");
	startImageConfig.m_panelSize = Vec2(.6f / g_theWindow->GetAspectRatio(), .6f);
	m_startScreenWidget->AddChild(startImageConfig);

	
	WidgetConfig titleheaderConfig;
	titleheaderConfig.m_borderSize = Vec2::ZERO;
	titleheaderConfig.m_panelSize = Vec2(.6f / g_theWindow->GetAspectRatio(), .2f);
	titleheaderConfig.m_text = "Vaporum";
	titleheaderConfig.m_textBounds = AABB2::ZERO_TO_ONE;
	titleheaderConfig.m_textHeight = 120.f;
	titleheaderConfig.m_allignment = Vec2(.5f, 1.f);
	m_startScreenWidget->AddChild(titleheaderConfig);
	
	WidgetConfig bottomTextConfig;
	bottomTextConfig.m_borderSize = Vec2::ZERO;
	bottomTextConfig.m_panelSize = Vec2(1.f, .2f);
	bottomTextConfig.m_text = "Press ENTER or click to start";
	bottomTextConfig.m_textBounds = AABB2::ZERO_TO_ONE;
	bottomTextConfig.m_textHeight = 60.f;
	bottomTextConfig.m_allignment = Vec2(.5f, 0.f);
	m_startScreenWidget->AddChild(bottomTextConfig);

	WidgetConfig menuBaseConfig;
	menuBaseConfig.m_borderSize = Vec2::ZERO;
	m_mainMenuWidget = new Widget(nullptr, menuBaseConfig);

	m_mainMenuWidget->AddChild(startImageConfig);


	WidgetConfig menuTextConfig;
	menuTextConfig.m_text = "Main Menu";
	menuTextConfig.m_panelSize = Vec2(.5f, .2f);
	menuTextConfig.m_panelColor = Rgba8(255,255,255,0);
	menuTextConfig.m_borderSize = Vec2::ZERO;
	menuTextConfig.m_textHeight = 120.f;
	menuTextConfig.m_orientation = 90.f;
	menuTextConfig.m_allignment = Vec2(-.1f, .5f);
	m_mainMenuWidget->AddChild(menuTextConfig);

	
	WidgetConfig menuSideBarConfig;
	menuSideBarConfig.m_panelSize = Vec2(.01f, 1.f);
	menuSideBarConfig.m_borderSize = Vec2::ZERO;
	menuSideBarConfig.m_panelColor = Rgba8(255, 255, 255, 255);
	menuSideBarConfig.m_allignment = Vec2(.225f, .5f);
	m_mainMenuWidget->AddChild(menuSideBarConfig);

	WidgetConfig buttonConfig;
	buttonConfig.m_text = "Local Game";
	buttonConfig.m_borderSize = Vec2(.01f, .01f);
	buttonConfig.m_borderColor = Rgba8::BLACK;
	buttonConfig.m_panelSize = Vec2(.3f, .05f);
	buttonConfig.m_allignment = Vec2(.332f, .5f);
	buttonConfig.m_textAlignment = Vec2(.05f, .5f);
	buttonConfig.m_textBounds = AABB2::ZERO_TO_ONE;
	buttonConfig.m_panelColor = Rgba8::WHITE;
	buttonConfig.m_textColor = Rgba8::BLACK;
	m_mainMenuButtons.push_back(m_mainMenuWidget->AddChild(buttonConfig));

	buttonConfig.m_text = "Network Game";
	buttonConfig.m_textColor = Rgba8::WHITE;
	buttonConfig.m_borderColor = Rgba8(0, 0, 0, 0);
	buttonConfig.m_panelColor = Rgba8(0, 0, 0, 0);
	buttonConfig.m_allignment = Vec2(.332f, .45f);
	m_mainMenuButtons.push_back(m_mainMenuWidget->AddChild(buttonConfig));

	buttonConfig.m_text = "Quit";
	buttonConfig.m_allignment = Vec2(.332f, .4f);
	m_mainMenuButtons.push_back(m_mainMenuWidget->AddChild(buttonConfig));

	m_pauseMenuWidget = new Widget(nullptr, menuBaseConfig);
	menuTextConfig.m_text = "Pause Menu";
	m_pauseMenuWidget->AddChild(startImageConfig);
	m_pauseMenuWidget->AddChild(menuTextConfig);
	m_pauseMenuWidget->AddChild(menuSideBarConfig);

	buttonConfig.m_text = "Resume Game";
	buttonConfig.m_allignment = Vec2(.332f, .5f);
	buttonConfig.m_borderColor = Rgba8::BLACK;
	buttonConfig.m_panelColor = Rgba8::WHITE;
	buttonConfig.m_textColor = Rgba8::BLACK;
	m_pauseMenuButtons.push_back(m_pauseMenuWidget->AddChild(buttonConfig));

	buttonConfig.m_text = "Main Menu";
	buttonConfig.m_textColor = Rgba8::WHITE;
	buttonConfig.m_borderColor = Rgba8(0, 0, 0, 0);
	buttonConfig.m_panelColor = Rgba8(0, 0, 0, 0);
	buttonConfig.m_allignment = Vec2(.332f, .45f);
	m_pauseMenuButtons.push_back(m_pauseMenuWidget->AddChild(buttonConfig));

	WidgetConfig blueTeamInfoConfig;
	blueTeamInfoConfig.m_allignment = Vec2(.04f, .12f);
	blueTeamInfoConfig.m_panelSize = Vec2(.12f, .6f);
	m_blueTeamUnitInfo = new Widget(nullptr, blueTeamInfoConfig);

	WidgetConfig blueTeamUnitNameConfig;
	blueTeamUnitNameConfig.m_panelSize = Vec2::ONE;
	blueTeamUnitNameConfig.m_allignment = Vec2(.5f, .5f);
	blueTeamUnitNameConfig.m_borderSize = Vec2::ZERO;
	blueTeamUnitNameConfig.m_panelColor = Rgba8(0, 0, 0, 0);
	blueTeamUnitNameConfig.m_text = "Polar";
	blueTeamUnitNameConfig.m_textColor = Rgba8::WHITE;
	blueTeamUnitNameConfig.m_textAlignment = Vec2(.5f, .95f);
	blueTeamUnitNameConfig.m_textHeight = 28.f;
	m_blueTeamUnitInfo->AddChild(blueTeamUnitNameConfig);

	WidgetConfig blueTeamUnitImageConfig;
	blueTeamUnitImageConfig.m_panelSize = Vec2(.55f, .55f);
	blueTeamUnitImageConfig.m_allignment = Vec2(.5f, .7f);
	blueTeamUnitImageConfig.m_borderSize = Vec2::ZERO;
	blueTeamUnitImageConfig.m_forceSquareAspect = true;
	blueTeamUnitImageConfig.m_panelColor = Rgba8::WHITE;
	blueTeamUnitImageConfig.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Tanks/Bison.png");
	m_blueTeamUnitInfo->AddChild(blueTeamUnitImageConfig);

	WidgetConfig blueTeamUnitStatConfig;
	blueTeamUnitStatConfig.m_allignment = Vec2(.5f, .35f);
	blueTeamUnitStatConfig.m_panelSize = Vec2(.85f, .1f);
	blueTeamUnitStatConfig.m_panelColor = Rgba8(0,0,0,0);
	blueTeamUnitStatConfig.m_borderSize = Vec2::ZERO;
	blueTeamUnitStatConfig.m_textHeight = 28.f;
	blueTeamUnitStatConfig.m_text = "Attack       60";
	blueTeamUnitStatConfig.m_textColor = Rgba8::WHITE;
	m_blueTeamUnitInfo->AddChild(blueTeamUnitStatConfig);

	blueTeamUnitStatConfig.m_allignment = Vec2(.5f, 0.275f);
	blueTeamUnitStatConfig.m_text = "Defense      60";
	m_blueTeamUnitInfo->AddChild(blueTeamUnitStatConfig);

	blueTeamUnitStatConfig.m_allignment = Vec2(.5f, 0.2f);
	blueTeamUnitStatConfig.m_text = "Range     1 - 1";
	m_blueTeamUnitInfo->AddChild(blueTeamUnitStatConfig);

	blueTeamUnitStatConfig.m_allignment = Vec2(.5f, 0.125f);
	blueTeamUnitStatConfig.m_text = "Move          4";
	m_blueTeamUnitInfo->AddChild(blueTeamUnitStatConfig);

	blueTeamUnitStatConfig.m_allignment = Vec2(.5f, 0.05f);
	blueTeamUnitStatConfig.m_text = "Health        8";
	m_blueTeamUnitInfo->AddChild(blueTeamUnitStatConfig);

	m_blueTeamUnitInfo->m_enabled = false;

	blueTeamInfoConfig.m_allignment.x = .96f;
	m_redTeamUnitInfo = new Widget(nullptr, blueTeamInfoConfig);
	m_redTeamUnitInfo->AddChild(blueTeamUnitNameConfig);
	m_redTeamUnitInfo->AddChild(blueTeamUnitImageConfig);
	blueTeamUnitStatConfig.m_allignment = Vec2(.5f, .35f);
	m_redTeamUnitInfo->AddChild(blueTeamUnitStatConfig);
	blueTeamUnitStatConfig.m_allignment = Vec2(.5f, 0.275f);
	m_redTeamUnitInfo->AddChild(blueTeamUnitStatConfig);
	blueTeamUnitStatConfig.m_allignment = Vec2(.5f, 0.2f);
	m_redTeamUnitInfo->AddChild(blueTeamUnitStatConfig);
	blueTeamUnitStatConfig.m_allignment = Vec2(.5f, 0.125f);
	m_redTeamUnitInfo->AddChild(blueTeamUnitStatConfig);
	blueTeamUnitStatConfig.m_allignment = Vec2(.5f, 0.05f);
	m_redTeamUnitInfo->AddChild(blueTeamUnitStatConfig);

	m_redTeamUnitInfo->m_enabled = false;

	WidgetConfig waitingForPlayersFill;
	waitingForPlayersFill.m_borderSize = Vec2::ZERO;
	m_waitingForPlayersWidget = new Widget(nullptr, waitingForPlayersFill);

	WidgetConfig waitingForPlayers;
	waitingForPlayers.m_panelSize = Vec2(.25f, .35f);
	waitingForPlayers.m_text = "Waiting For Players...";
	waitingForPlayers.m_textHeight = 40.f;
	m_waitingForPlayersWidget->AddChild(waitingForPlayers);

	waitingForPlayers.m_text = "Player 1's Turn";
	waitingForPlayers.m_textHeight = 60.f;
	waitingForPlayers.m_textBounds = AABB2(Vec2(.1f, 0.f), Vec2(.9f, 1.f));
	waitingForPlayers.m_textAlignment = Vec2(.5f, .95f);
	m_turnBeginWidget = new Widget(nullptr, waitingForPlayers);

	WidgetConfig nextTurnBottomTextConfig;
	nextTurnBottomTextConfig.m_text = "Press ENTER or click to continue";
	nextTurnBottomTextConfig.m_borderSize = Vec2::ZERO;
	nextTurnBottomTextConfig.m_panelColor = Rgba8(0, 0, 0, 0);
	nextTurnBottomTextConfig.m_textHeight = 40.f;
	nextTurnBottomTextConfig.m_panelSize = Vec2(.9f, 1.f);
	nextTurnBottomTextConfig.m_textAlignment = Vec2(.5f, .1f);
	m_turnBeginWidget->AddChild(nextTurnBottomTextConfig);

	WidgetConfig playerTurnConfig;
	playerTurnConfig.m_panelSize = Vec2(.2f, .1f);
	playerTurnConfig.m_textColor = Rgba8::GREEN;
	playerTurnConfig.m_text = "Player 1's Turn";
	playerTurnConfig.m_textBounds = AABB2(Vec2(.1f, 0.f), Vec2(.9f, 1.f));
	playerTurnConfig.m_allignment = Vec2(.025f, .95f);
	m_playerTurnIndicator = new Widget(nullptr, playerTurnConfig);

	WidgetConfig playerControlsWidget;
	playerControlsWidget.m_panelSize = Vec2(.65f, .15f);
	playerControlsWidget.m_allignment = Vec2(.5f, .05f);
	playerControlsWidget.m_borderSize = Vec2(.005f, .005f);
	m_controlsWidget = new Widget(nullptr, playerControlsWidget);

	WidgetConfig controlBox;
	controlBox.m_allignment = Vec2(.02f, .8f);
	controlBox.m_panelColor = Rgba8(0,0,0,0);
	controlBox.m_borderSize = Vec2::ZERO;
	controlBox.m_panelSize = Vec2(.2f, .2f);
	Widget* control1 = m_controlsWidget->AddChild(controlBox);

	WidgetConfig iconConfig;
	iconConfig.m_forceSquareAspect = true;
	iconConfig.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Icons/LMB.png");
	iconConfig.m_borderSize = Vec2::ZERO;
	iconConfig.m_panelColor = Rgba8::WHITE;
	iconConfig.m_panelSize = Vec2(.1f, 1.f);
	iconConfig.m_allignment = Vec2(.0f, .5f);
	control1->AddChild(iconConfig);

	WidgetConfig textConfig;
	textConfig.m_panelColor = Rgba8(0, 0, 0, 0);
	textConfig.m_borderSize = Vec2::ZERO;
	textConfig.m_panelSize = Vec2(.85f, 1.f);
	textConfig.m_text = "Move";
	textConfig.m_allignment = Vec2(1.f, .5f);
	textConfig.m_textAlignment = Vec2(0.f, .5f);
	textConfig.m_textHeight = 35.f;
	control1->AddChild(textConfig);
	m_controlWidgets.push_back(control1);

	controlBox.m_allignment = Vec2(.4f, .8f);
	Widget* control2 = m_controlsWidget->AddChild(controlBox);
	iconConfig.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Icons/RMB.png");
	control2->AddChild(iconConfig);
	textConfig.m_text = "Deselect";
	control2->AddChild(textConfig);
	m_controlWidgets.push_back(control2);

	controlBox.m_allignment = Vec2(.02f, .2f);
	Widget* control3 = m_controlsWidget->AddChild(controlBox);
	iconConfig.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Icons/Left.png");
	control3->AddChild(iconConfig);
	textConfig.m_text = "Previous";
	control3->AddChild(textConfig);
	m_controlWidgets.push_back(control3);

	controlBox.m_allignment = Vec2(.4f, .2f);
	Widget* control4 = m_controlsWidget->AddChild(controlBox);
	iconConfig.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Icons/Right.png");
	control4->AddChild(iconConfig);
	textConfig.m_text = "Next";
	control4->AddChild(textConfig);
	m_controlWidgets.push_back(control4);

	controlBox.m_allignment = Vec2(.78f, .2f);
	Widget* control5 = m_controlsWidget->AddChild(controlBox);
	iconConfig.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Icons/Y.png");
	control5->AddChild(iconConfig);
	textConfig.m_text = "End Turn";
	control5->AddChild(textConfig);
	m_controlWidgets.push_back(control5);

	waitingForPlayers.m_text = "Player 1 Wins!";
	m_gameCompleteWidget = new Widget(nullptr, waitingForPlayers);
	m_gameCompleteWidget->AddChild(nextTurnBottomTextConfig);

	m_controlWidgets[0]->m_enabled = false;
	m_controlWidgets[1]->m_enabled = false;

	std::vector<std::string> effectDefsToLoad;
	effectDefsToLoad.push_back("Data/Saves/ParticleEffects/ExplosionVaporum.xml");
	for (int i = 0; i < (int)effectDefsToLoad.size(); i++)
	{
		g_theParticleSystem->LoadEffectByFileName(effectDefsToLoad[i]);
	}
}

void Game::StartGame()
{
	m_attractScreen = false;
	Mat44 xAxisTextTransform;
	xAxisTextTransform.AppendTranslation3D(Vec3(2.f, 0.f, .25f));
	DebugAddWorldText("X axis", xAxisTextTransform, .25f, Vec2(.5f, .5f), -1.f, Rgba8::RED, Rgba8::RED);

	Mat44 yAxisTextTransform;
	yAxisTextTransform.AppendTranslation3D(Vec3(0.f, 2.f, .25f));
	yAxisTextTransform.AppendZRotation(270.f);
	DebugAddWorldText("Y axis", yAxisTextTransform, .25f, Vec2(.5f, .5f), -1.f, Rgba8::GREEN, Rgba8::GREEN);

	Mat44 zAxisTextTransform;
	zAxisTextTransform.AppendTranslation3D(Vec3(0.f, 0.f, 2.f));
	zAxisTextTransform.AppendZRotation(-90.f);
	zAxisTextTransform.AppendYRotation(90.f);
	DebugAddWorldText("Z axis", zAxisTextTransform, .25f, Vec2(.5f, .5f), -1.f, Rgba8::BLUE, Rgba8::BLUE);

	Mat44 basisTransform;
	basisTransform.AppendTranslation3D(Vec3(0.f, 0.f, .25f));
	DebugAddWorldBasis(basisTransform, -1.f);
}

void Game::HoldFire()
{
	m_currentTurnState = TurnState::SELECTING;
	m_selectedUnit->m_actionsFinished = true;
	m_selectedUnit = nullptr;
}

void Game::Update(float deltaSeconds)
{
	for (size_t i = 0; i < m_allEntities.size(); i++)
	{
		m_allEntities[i]->Update(deltaSeconds);
	}
	if (m_currentGameState == GameState::START_SCREEN)
	{
		Update_StartScreen(deltaSeconds);
	}
	else if (m_currentGameState == GameState::MAIN_MENU)
	{
		Update_MainMenu(deltaSeconds);
	}
	else if (m_currentGameState == GameState::WAITING_FOR_PLAYER)
	{
		Update_WaitingForPlayer(deltaSeconds);
	}
	else if (m_currentGameState == GameState::PLAYING)
	{
		Update_Playing(deltaSeconds);
	}
	else if (m_currentGameState == GameState::PAUSE_MENU)
	{
		Update_PauseMenu(deltaSeconds);
	}
	else if (m_currentGameState == GameState::GAME_COMPLETE)
	{
		Update_GameComplete(deltaSeconds);
	}

	CheckForDebugCommands(deltaSeconds);
}

void Game::Render() const 
{
	//World Space Rendering
	g_theRenderer->BeginCamera(m_player->m_playerCamera);
	LightConstants lightConstants;
	lightConstants.AmbientIntensity = 1.f - m_sunIntensity;
	lightConstants.renderAmbientDebugFlag = (int)m_useAmbient;
	lightConstants.renderDiffuseDebugFlag = (int)m_useDiffuse;
	lightConstants.renderEmissiveDebugFlag = (int)m_useEmissive;
	lightConstants.renderSpecularDebugFlag = (int)m_useSpecular;
	lightConstants.SunDirection = m_sunOrientaiton.GetIFwd();
	lightConstants.SunIntensity = m_sunIntensity;
	lightConstants.useDiffuseMapDebugFlag = (int)m_diffuseMap;
	lightConstants.useEmissiveMapDebugFlag = (int)m_emissiveMap;
	lightConstants.useGlossinessMapDebugFlag = (int)m_glossinessMap;
	lightConstants.useNormalMapDebugFlag = (int)m_normalMap;
	lightConstants.useSpecularMapDebugFlag = (int)m_specularMap;
	lightConstants.worldEyePosition = m_player->m_position;

	g_theRenderer->SetLightingConstants(lightConstants);

	for (size_t i = 0; i < m_allEntities.size(); i++)
	{
		m_allEntities[i]->Render();
	}
	DrawHilightedTile();
	g_theParticleSystem->Render();
	g_theRenderer->RenderEmissive();
	g_theRenderer->EndCamera(m_player->m_playerCamera);
	DebugRenderWorld(m_player->m_playerCamera);
	g_theRenderer->BeginCamera(m_screenCamera);
	if (m_currentGameState == GameState::START_SCREEN)
	{
		m_startScreenWidget->Render();
	}
	else if (m_currentGameState == GameState::MAIN_MENU)
	{
		m_mainMenuWidget->Render();
	}
	else if (m_currentGameState == GameState::PAUSE_MENU)
	{
		m_pauseMenuWidget->Render();
	}
	else if (m_currentGameState == GameState::WAITING_FOR_PLAYER)
	{
		m_waitingForPlayersWidget->Render();
	}
	else if (m_currentGameState == GameState::PLAYING)
	{
		if (m_currentTurnState == TurnState::TURN_BEGIN)
		{
			m_turnBeginWidget->Render();
		}
		m_blueTeamUnitInfo->Render();
		m_redTeamUnitInfo->Render();
		m_playerTurnIndicator->Render();
		if (IsControllingCurrentTurn())
		{
			m_controlsWidget->Render();
		}
	}
	else if (m_currentGameState == GameState::GAME_COMPLETE)
	{
		m_gameCompleteWidget->Render();
	}
	//ScreenSpace Rendering
	DebugRenderScreen(m_screenCamera);
}

void Game::UpdateBlueUnitWidget()
{
	Unit* statsUnit = nullptr;
	if (m_selectedUnit != nullptr && m_selectedUnit->m_team == Team::BLUE)
	{
		statsUnit = m_selectedUnit;
	
	}
	else
	{
		for (int i = 0; i < (int)m_allUnits.size(); i++)
		{
			if (m_allUnits[i]->m_coords == m_focusedHexCoords && m_allUnits[i]->m_team == Team::BLUE)
			{
				statsUnit = m_allUnits[i];
				break;
			}
		}
	}
	if (statsUnit != nullptr)
	{
		m_blueTeamUnitInfo->m_enabled = true;
		m_blueTeamUnitInfo->m_children[0]->m_config.m_text = statsUnit->m_def.m_name;
		m_blueTeamUnitInfo->m_children[1]->m_config.m_texture = statsUnit->m_def.m_image;
		m_blueTeamUnitInfo->m_children[2]->m_config.m_text = Stringf("Attack        %.0f", statsUnit->m_def.m_attackDamage);
		m_blueTeamUnitInfo->m_children[3]->m_config.m_text = Stringf("Defense       %.0f", statsUnit->m_def.m_defense);
		m_blueTeamUnitInfo->m_children[4]->m_config.m_text = Stringf("Range        %d-%d", statsUnit->m_def.m_attackRangeMin, statsUnit->m_def.m_attackRangeMax);
		m_blueTeamUnitInfo->m_children[5]->m_config.m_text = Stringf("Move           %d", statsUnit->m_def.m_movementRange);
		m_blueTeamUnitInfo->m_children[6]->m_config.m_text = Stringf("Health         %d", statsUnit->m_currentHealth);
		m_blueTeamUnitInfo->Build();
	}
	else
	{
		m_blueTeamUnitInfo->m_enabled = false;
	}
}

void Game::UpdateRedUnitWidget()
{
	Unit* statsUnit = nullptr;
	if (m_selectedUnit != nullptr && m_selectedUnit->m_team == Team::RED)
	{
		statsUnit = m_selectedUnit;

	}
	else
	{
		for (int i = 0; i < (int)m_allUnits.size(); i++)
		{
			if (m_allUnits[i]->m_coords == m_focusedHexCoords && m_allUnits[i]->m_team == Team::RED)
			{
				statsUnit = m_allUnits[i];
				break;
			}
		}
	}
	if (statsUnit != nullptr)
	{
		m_redTeamUnitInfo->m_enabled = true;
		m_redTeamUnitInfo->m_children[0]->m_config.m_text = statsUnit->m_def.m_name;
		m_redTeamUnitInfo->m_children[1]->m_config.m_texture = statsUnit->m_def.m_image;
		m_redTeamUnitInfo->m_children[2]->m_config.m_text = Stringf("Attack        %.0f", statsUnit->m_def.m_attackDamage);
		m_redTeamUnitInfo->m_children[3]->m_config.m_text = Stringf("Defense       %.0f", statsUnit->m_def.m_defense);
		m_redTeamUnitInfo->m_children[4]->m_config.m_text = Stringf("Range        %d-%d", statsUnit->m_def.m_attackRangeMin, statsUnit->m_def.m_attackRangeMax);
		m_redTeamUnitInfo->m_children[5]->m_config.m_text = Stringf("Move           %d", statsUnit->m_def.m_movementRange);
		m_redTeamUnitInfo->m_children[6]->m_config.m_text = Stringf("Health         %d", statsUnit->m_currentHealth);
		m_redTeamUnitInfo->Build();
	}
	else
	{
		m_redTeamUnitInfo->m_enabled = false;
	}
}

void Game::ShutDown()
{
	for (int i = 0; i < (int)m_allEntities.size(); i++)
	{
		delete m_allEntities[i];
	}
	m_allEntities.clear();
}



bool Game::IsControllingCurrentTurn() const
{
	if (m_selectedUnit != nullptr && (m_selectedUnit->m_isMoving || m_selectedUnit->m_isAttacking))
	{
		return false;
	}
	if (m_multiplayerTeam == Team::NONE)
	{
		return true;
	}
	return m_currentPlayerTurn == m_multiplayerTeam;
}

bool Game::IsMultiplayerTeamsTurn()
{
	if (m_multiplayerTeam == Team::NONE)
	{
		return false;
	}
	return m_currentPlayerTurn == m_multiplayerTeam;
}

bool Game::Event_LoadMap(EventArgs& args)
{
	std::string mapToLoad = args.GetValue("name", "");
	g_theGame->LoadMap(mapToLoad);
	return true;
}

void Game::LoadMap(std::string mapToLoad)
{
	TrimString(mapToLoad, '\0');
	if (m_multiplayerTeam == Team::BLUE)
	{
		g_theNetSystem->m_sendQueue.push_back(Stringf("LoadMap name=%s", mapToLoad.c_str()));
	}
	else if (m_currentGameState == GameState::MAIN_MENU || m_currentGameState == GameState::START_SCREEN)
	{
		m_multiplayerMapToLoad = mapToLoad;
		return;
	}
	for (int i = 0; i < (int)g_theApp->m_mapDefinitions.size(); i++)
	{
		if (g_theApp->m_mapDefinitions[i].m_name == mapToLoad)
		{
			m_currentMapDef = g_theApp->m_mapDefinitions[i];

			m_hexGrid = new Prop(this, Vec3::ZERO);
			m_hexGrid->CreateHexGrid(m_currentMapDef);
			m_allEntities.push_back(m_hexGrid);
			break;
		}
	}
	for (int i = 0; i < (int)m_currentMapDef.m_player1Units.size(); i++)
	{
		UnitInMap& unitInMap = m_currentMapDef.m_player1Units[i];
		Unit* unit = new Unit(this, unitInMap.m_startCoords, UnitDefinition::GetDefinition(unitInMap.m_unitType), true);
		m_allEntities.push_back(unit);
		m_allUnits.push_back(unit);
		m_player1Units.push_back(unit);
	}

	for (int i = 0; i < (int)m_currentMapDef.m_player2Units.size(); i++)
	{
		UnitInMap& unitInMap = m_currentMapDef.m_player2Units[i];
		Unit* unit = new Unit(this, unitInMap.m_startCoords, UnitDefinition::GetDefinition(unitInMap.m_unitType), false);
		m_allEntities.push_back(unit);
		m_allUnits.push_back(unit);
		m_player2Units.push_back(unit);
	}
	if (m_multiplayerTeam == Team::NONE)
	{
		m_currentGameState = GameState::PLAYING;
	}
	else
	{
		g_theNetSystem->m_sendQueue.push_back("PlayerReady");
		m_currentGameState = GameState::WAITING_FOR_PLAYER;
	}
	m_selfInGame = true;
}

TileDefinition Game::GetTileDefinitionBySymbol(char symbol) const
{
	for (int i = 0; i < (int)g_theApp->m_tileDefinitions.size(); i++)
	{
		if (g_theApp->m_tileDefinitions[i].m_symbol == symbol)
		{
			return g_theApp->m_tileDefinitions[i];
		}
	}
	return g_theApp->m_tileDefinitions[0];
}

void Game::AddVertsForHexCell(std::vector<Vertex_PCU>& verts, IntVec2 const& cellCoords, Rgba8 const& color, bool outline, float size) const
{
	Vec2 cellCenterPos = GetHexPosFromGridCoord(cellCoords);
	if (outline)
	{
		AddVertsForRing2D(verts, cellCenterPos, (1.f / sqrtf(3.f)) * size, .05f, color, 6, .025f);
	}
	else
	{
		AddVertsForDisc2D(verts, cellCenterPos, (1.f / sqrtf(3.f)) * size, color, 6, .025f);
	}
}


void Game::CheckForDebugCommands(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_ARROW_BRACKET))
	{
		m_sunIntensity -= .1f;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_ARROW_BRACKET))
	{
		m_sunIntensity += .1f;
	}
	m_sunIntensity = Clamp(m_sunIntensity, 0.f, 1.f);
	if (g_theInput->WasKeyJustPressed('1'))
	{
		m_useAmbient = !m_useAmbient;
	}
	if (g_theInput->WasKeyJustPressed('2'))
	{
		m_useDiffuse = !m_useDiffuse;
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		m_useSpecular = !m_useSpecular;
	}
	if (g_theInput->WasKeyJustPressed('4'))
	{
		m_useEmissive = !m_useEmissive;
	}
	if (g_theInput->WasKeyJustPressed('5'))
	{
		m_diffuseMap = !m_diffuseMap;
	}
	if (g_theInput->WasKeyJustPressed('6'))
	{
		m_normalMap = !m_normalMap;
	}
	if (g_theInput->WasKeyJustPressed('7'))
	{
		m_specularMap = !m_specularMap;
	}
	if (g_theInput->WasKeyJustPressed('8'))
	{
		m_glossinessMap = !m_glossinessMap;
	}
	if (g_theInput->WasKeyJustPressed('9'))
	{
		m_emissiveMap = !m_emissiveMap;
	}
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_ARROW))
	{
		m_sunOrientaiton.m_yaw -= 90.f * deltaSeconds;
	}
	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_ARROW))
	{
		m_sunOrientaiton.m_yaw += 90.f * deltaSeconds;
	}
	if (g_theInput->IsKeyDown(KEYCODE_DOWN_ARROW))
	{
		m_sunOrientaiton.m_pitch += 45.f * deltaSeconds;
	}
	if (g_theInput->IsKeyDown(KEYCODE_UP_ARROW))
	{
		m_sunOrientaiton.m_pitch -= 45.f * deltaSeconds;
	}
	if (g_theInput->WasKeyJustPressed('N'))
	{
		m_renderDebug = !m_renderDebug;
	}
}

RaycastResult3D Game::RaycastVsHexGrid()
{
	Vec3 rayStart = m_player->m_position;
	Vec3 worldMouseDir = (g_theWindow->GetWorldMousePosition(m_player->m_playerCamera) - m_player->m_position).GetNormalized();
	Vec3 rayDir = worldMouseDir;
	Plane3 groundPlane;
	groundPlane.m_normal = Vec3(0.f, 0.f, 1.f);
	groundPlane.m_distFromOriginAlongNormal = 0.f;
	return RaycastVsPlanes3D(rayStart, rayDir, 100.f, groundPlane);
}

void Game::DrawHilightedTile() const
{
	if (m_focusedHexCoords == IntVec2(-1, -1))
	{
		return;
	}
	std::vector<Vertex_PCU> hexCellverts;
	int closestTileIndex = m_focusedHexCoords.x + m_currentMapDef.m_gridSize.x * m_focusedHexCoords.y;
	TileDefinition currTileDef = GetTileDefinitionBySymbol(m_currentMapDef.m_tiles[closestTileIndex]);
	if (currTileDef.m_isBlocked)
	{
		AddVertsForHexCell(hexCellverts, m_focusedHexCoords, Rgba8::RED, true, .8f);
	}
	else
	{
		AddVertsForHexCell(hexCellverts, m_focusedHexCoords, Rgba8::GREEN, true, .8f);
	}
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->DrawVertexArray(hexCellverts.size(), hexCellverts.data());
	
}

void Game::LoadUnits()
{
}


Vec2 GetHexPosFromGridCoord(IntVec2 const& gridGoord)
{
	Vec2 iBasis(.866f, .5f);
	Vec2 jBasis(0.f, 1.f);

	Vec2 hexPos = iBasis * (float)gridGoord.x + jBasis * (float)gridGoord.y;
	return hexPos;
}

IntVec2 GetGridCoordFromIndex(int index)
{
	IntVec2 dimensions = g_theGame->m_currentMapDef.m_gridSize;
	return IntVec2(index % dimensions.x, index / dimensions.x);
}

int GetIndexFromGridCoord(IntVec2 const& gridCoord)
{
	IntVec2 dimensions = g_theGame->m_currentMapDef.m_gridSize;
	return gridCoord.x + gridCoord.y * dimensions.x;
}

bool IsInBounds(IntVec2 const& gridCoord)
{
	AABB3 worldBounds = AABB3(g_theGame->m_currentMapDef.m_worldBoundsMin, g_theGame->m_currentMapDef.m_worldBoundsMax);
	Vec2 hexPos = GetHexPosFromGridCoord(gridCoord);
	IntVec2 dimensions = g_theGame->m_currentMapDef.m_gridSize;


	return gridCoord.x >= 0 && gridCoord.x < dimensions.x && gridCoord.y >= 0 && gridCoord.y < dimensions.y && worldBounds.IsPointInside(hexPos.GetXYZ());
}

bool IsBlocked(IntVec2 const& gridCoord)
{
	char tile = g_theGame->m_currentMapDef.m_tiles[GetIndexFromGridCoord(gridCoord)];
	TileDefinition const& tileDef = g_theGame->GetTileDefinitionBySymbol(tile);
	return tileDef.m_isBlocked;
}

IntVec2 GetHexCoordFromWorldPos(Vec3 const& worldPosition)
{
	AABB3 mapBounds(g_theGame->m_currentMapDef.m_worldBoundsMin, g_theGame->m_currentMapDef.m_worldBoundsMax);
	if (!mapBounds.IsPointInside(worldPosition))
	{
		return IntVec2(-1, -1);
	}
	float closestDistSquared = 9999999999.f;
	IntVec2 closestTileCoords = IntVec2(-1, -1);
	for (int y = 0; y < g_theGame->m_currentMapDef.m_gridSize.y; y++)
	{
		for (int x = 0; x < g_theGame->m_currentMapDef.m_gridSize.x; x++)
		{
			Vec3 currentTilePos = GetHexPosFromGridCoord(IntVec2(x, y)).GetXYZ();
			if (mapBounds.IsPointInside(currentTilePos))
			{
				float currentDistanceSquared = GetDistanceSquared3D(currentTilePos, worldPosition);
				if (currentDistanceSquared < closestDistSquared)
				{
					closestDistSquared = currentDistanceSquared;
					closestTileCoords = IntVec2(x, y);
				}
			}
		}
	}

	if (closestTileCoords != IntVec2(-1, -1) && closestDistSquared < .25f)
	{
		return closestTileCoords;
	}
	return IntVec2(-1, -1);
}

void Game::SetFocusedHex(IntVec2 const& coords)
{
	if (m_focusedHexCoords == coords || coords == IntVec2(-1, -1))
	{
		return;
	}
	if (m_currentPlayerTurn == m_multiplayerTeam)
	{
		g_theNetSystem->m_sendQueue.push_back(Stringf("SetFocusedHex Coords=%d,%d", coords.x, coords.y));
	}
	m_focusedHexCoords = coords;
}

void Game::SelectFocusedUnit()
{
	if (IsMultiplayerTeamsTurn())
	{
		g_theNetSystem->m_sendQueue.push_back("SelectFocusedUnit");
	}

	bool selected = false;
	for (int i = 0; i < (int)m_allUnits.size(); i++)
	{
		if (m_allUnits[i]->m_coords == m_focusedHexCoords)
		{
			if (!m_allUnits[i]->m_actionsFinished && m_allUnits[i]->m_team == m_currentPlayerTurn)
			{
				selected = true;
				m_selectedUnit = m_allUnits[i];
				m_currentTurnState = TurnState::MOVING;
			}
		}
	}
}

void Game::SelectPreviousUnit()
{
	if (IsMultiplayerTeamsTurn())
	{
		g_theNetSystem->m_sendQueue.push_back("SelectPreviousUnit");
	}

	std::vector<Unit*>& unitsToSelectFrom = m_player1Units;
	if (m_currentPlayerTurn == Team::RED)
	{
		unitsToSelectFrom = m_player2Units;
	}

	if (m_selectedUnit == nullptr)
	{
		for (int i = 0; i < (int)unitsToSelectFrom.size(); i++)
		{
			if (!unitsToSelectFrom[i]->m_actionsFinished)
			{
				m_selectedUnit = unitsToSelectFrom[i];
				m_currentTurnState = TurnState::MOVING;
				return;
			}
		}
	}

	int currentlySelectedIndex = 1;
	int indexToSelect = currentlySelectedIndex - 1;

	for (int i = 0; i < (int)unitsToSelectFrom.size(); i++)
	{
		if (unitsToSelectFrom[i] == m_selectedUnit)
		{
			currentlySelectedIndex = i;
		}
		indexToSelect = currentlySelectedIndex - 1;
		if (indexToSelect < 0)
		{
			indexToSelect = (int)unitsToSelectFrom.size() - 1;
		}
	}
	while (indexToSelect != currentlySelectedIndex)
	{
		if (!unitsToSelectFrom[indexToSelect]->m_actionsFinished)
		{
			if (m_selectedUnit != nullptr)
			{
				m_selectedUnit->m_orientationDegrees.m_yaw = m_selectedUnit->m_previousTileRotation;
			}
			m_selectedUnit = unitsToSelectFrom[indexToSelect];
			m_currentTurnState = TurnState::MOVING;
			break;
		}
		indexToSelect--;
		if (indexToSelect < 0)
		{
			indexToSelect = (int)unitsToSelectFrom.size() - 1;
		}
	}
}

void Game::AttackComplete()
{
	g_theAudio->StartSound(g_theAudio->CreateOrGetSound("Data/Audio/TankShot.wav"));
	ParticleEffect* shootEffect =
		g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/VaporumShoot.xml", m_selectedUnit->m_position + Vec3(.0f, 0.f, .25f) + m_selectedUnit->GetForwardNormal() * .4f, m_selectedUnit->m_orientationDegrees, true, 1.f);
	shootEffect->SetScale(.05f);
	for (int i = 0; i < m_allUnits.size(); i++)
	{
		Unit* currentUnit = m_allUnits[i];
		if (currentUnit->m_coords == m_focusedHexCoords)
		{
			int damage = (int)((2 * m_selectedUnit->m_def.m_attackDamage) / currentUnit->m_def.m_defense);
			currentUnit->m_currentHealth -= damage;
			m_allEntities.push_back(new DamageText(this, currentUnit->m_position + Vec3(0.f, 0.f, .5f), damage));

			//handle retaliation
			if (currentUnit->CanAttackTile(m_selectedUnit->m_coords))
			{
				int retaliationDamage = (int)((2 * currentUnit->m_def.m_attackDamage) / m_selectedUnit->m_def.m_defense);
				m_selectedUnit->m_currentHealth -= retaliationDamage;
				m_allEntities.push_back(new DamageText(this, m_selectedUnit->m_position + Vec3(0.f, 0.f, .5f), retaliationDamage));
			}

			bool playDeathEffects = false;
			//handle death
			if (currentUnit->m_currentHealth <= 0)
			{
				playDeathEffects = true;
				currentUnit->Die();
				delete currentUnit;
				currentUnit = nullptr;
			}
			if (m_selectedUnit->m_currentHealth <= 0)
			{
				playDeathEffects = true;
				m_selectedUnit->Die();
				delete m_selectedUnit;
				m_selectedUnit = nullptr;
			}
			if (playDeathEffects)
			{
				g_theAudio->StartSound(g_theAudio->CreateOrGetSound("Data/Audio/Explosion.wav"));
			}
			if (currentUnit != nullptr)
			{
				g_theAudio->StartSound(g_theAudio->CreateOrGetSound("Data/Audio/Hit.wav"));
				g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/VaporumHitEffect.xml", currentUnit->m_position + Vec3(.0f, 0.f, .25f), EulerAngles(), true, 1.f);
			}
			break;
		}
	}

	m_currentTurnState = TurnState::SELECTING;
	if (m_selectedUnit != nullptr)
	{
		m_selectedUnit->m_actionsFinished = true;
	}
	m_selectedUnit = nullptr;
}

void Game::SelectNextUnit()
{
	if (IsMultiplayerTeamsTurn())
	{
		g_theNetSystem->m_sendQueue.push_back("SelectNextUnit");
	}
	std::vector<Unit*>& unitsToSelectFrom = m_player1Units;
	if (m_currentPlayerTurn == Team::RED)
	{
		unitsToSelectFrom = m_player2Units;
	}

	if (m_selectedUnit == nullptr)
	{
		for (int i = 0; i < (int)unitsToSelectFrom.size(); i++)
		{
			if (!unitsToSelectFrom[i]->m_actionsFinished)
			{
				m_selectedUnit = unitsToSelectFrom[i];
				m_currentTurnState = TurnState::MOVING;
				return;
			}
		}
	}

	int currentlySelectedIndex = 0;
	int indexToSelect = currentlySelectedIndex + 1;

	for (int i = 0; i < (int)unitsToSelectFrom.size(); i++)
	{
		if (unitsToSelectFrom[i] == m_selectedUnit)
		{
			currentlySelectedIndex = i;
		}
		indexToSelect = currentlySelectedIndex + 1;
		if (indexToSelect == unitsToSelectFrom.size())
		{
			indexToSelect = 0;
		}
	}
	while (indexToSelect != currentlySelectedIndex)
	{
		if (!unitsToSelectFrom[indexToSelect]->m_actionsFinished)
		{
			if (m_selectedUnit != nullptr)
			{
				m_selectedUnit->m_orientationDegrees.m_yaw = m_selectedUnit->m_previousTileRotation;
			}
			m_selectedUnit = unitsToSelectFrom[indexToSelect];
			m_currentTurnState = TurnState::MOVING;
			break;
		}
		indexToSelect++;
		if (indexToSelect == unitsToSelectFrom.size())
		{
			indexToSelect = 0;
		}
	}
}

void Game::Move()
{
	if (IsMultiplayerTeamsTurn())
	{
		g_theNetSystem->m_sendQueue.push_back("Move");
	}
	m_selectedUnit->Move();
	if (m_selectedUnit->m_def.m_unitType == UnitType::TANK)
	{
		m_currentTurnState = TurnState::ATTACKING;
	}
	else
	{
		m_currentTurnState = TurnState::SELECTING;
	}
}

void Game::Cancel()
{
	if (IsMultiplayerTeamsTurn())
	{
		g_theNetSystem->m_sendQueue.push_back("Cancel");
	}
	if (m_selectedUnit->m_isStaying)
	{
		m_selectedUnit->m_isStaying = false;
	}
	else
	{
		m_selectedUnit->m_coords = m_selectedUnit->m_previousCoords;
		m_selectedUnit->m_position = GetHexPosFromGridCoord(m_selectedUnit->m_coords).GetXYZ();
		m_selectedUnit->UpdateHeatMap();
	}
	m_currentTurnState = TurnState::MOVING;
}

void Game::Attack()
{
	if (IsMultiplayerTeamsTurn())
	{
		g_theNetSystem->m_sendQueue.push_back("Attack");
	}
	m_selectedUnit->Attack();
}


void Game::Update_WaitingForPlayer(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (m_selfInGame && m_remotePlayerInGame)
	{
		m_currentGameState = GameState::PLAYING;
	}
}

void Game::Update_StartScreen(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE) || g_theInput->WasKeyJustPressed(KEYCODE_ENTER))
	{
		m_currentGameState = GameState::MAIN_MENU;
	}
}


void Game::Update_MainMenu(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	//handle selected button keyboard
	int previouslySelectedButton = m_selectedMainMenuButton;
	if (g_theInput->WasKeyJustPressed(KEYCODE_DOWN_ARROW))
	{
		m_selectedMainMenuButton++;
		if (m_selectedMainMenuButton >= (int)m_mainMenuButtons.size())
		{
			m_selectedMainMenuButton = 0;
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_UP_ARROW))
	{
		m_selectedMainMenuButton--;
		if (m_selectedMainMenuButton < 0)
		{
			m_selectedMainMenuButton = (int)m_mainMenuButtons.size() - 1;
		}
	}
	//handle selected button mouse
	for (int i = 0; i < (int)m_mainMenuButtons.size(); i++)
	{
		if (m_mainMenuButtons[i]->GetMouseHoveredWidgetInHeirarchy() != nullptr)
		{
			m_selectedMainMenuButton = i;
		}
	}

	if (previouslySelectedButton != m_selectedMainMenuButton)
	{
		m_mainMenuButtons[previouslySelectedButton]->m_config.m_panelColor = Rgba8(0, 0, 0, 0);
		m_mainMenuButtons[previouslySelectedButton]->m_config.m_textColor = Rgba8::WHITE;
		m_mainMenuButtons[previouslySelectedButton]->m_config.m_borderColor = Rgba8(0, 0, 0, 0);
		m_mainMenuButtons[previouslySelectedButton]->Build();

		m_mainMenuButtons[m_selectedMainMenuButton]->m_config.m_panelColor = Rgba8::WHITE;
		m_mainMenuButtons[m_selectedMainMenuButton]->m_config.m_textColor = Rgba8::BLACK;
		m_mainMenuButtons[m_selectedMainMenuButton]->m_config.m_borderColor = Rgba8::BLACK;
		m_mainMenuButtons[m_selectedMainMenuButton]->Build();

	}



	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE) || g_theInput->WasKeyJustPressed(KEYCODE_ENTER))
	{
		if (m_selectedMainMenuButton == 0)
		{
			m_multiplayerTeam = Team::NONE;
			m_currentGameState = GameState::PLAYING;
			LoadMap(g_gameConfigBlackboard.GetValue("defaultMap", "Grid12x12"));
		}
		else if (m_selectedMainMenuButton == 1)
		{
			if (g_theNetSystem->m_config.m_mode == Mode::SERVER)
			{
				m_multiplayerTeam = Team::BLUE;
			}
			else if (g_theNetSystem->m_config.m_mode == Mode::CLIENT)
			{
				m_multiplayerTeam = Team::RED;
				m_currentGameState = GameState::WAITING_FOR_PLAYER;
				if (m_multiplayerMapToLoad != "")
				{
					LoadMap(m_multiplayerMapToLoad);
				}
			}
			if (m_multiplayerTeam == Team::NONE || m_multiplayerTeam == Team::BLUE)
			{
				m_currentGameState = GameState::WAITING_FOR_PLAYER;
				LoadMap(g_gameConfigBlackboard.GetValue("defaultMap", "Grid12x12"));
			}
		}
		else if (m_selectedMainMenuButton == 2)
		{
			g_theNetSystem->m_sendQueue.push_back("PlayerQuit");
			g_theApp->m_isQuitting = true;
		}
	}
	else if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theNetSystem->m_sendQueue.push_back("PlayerQuit");
		g_theApp->m_isQuitting = true;
	}
}


void Game::EndTurn()
{
	if (IsMultiplayerTeamsTurn())
	{
		g_theNetSystem->m_sendQueue.push_back("EndTurn");
	}

	for (int i = 0; i < m_allUnits.size(); i++)
	{
		m_allUnits[i]->m_actionsFinished = false;
		m_allUnits[i]->m_isStaying = false;
	}
	if (m_currentPlayerTurn == Team::BLUE)
	{
		m_currentPlayerTurn = Team::RED;
		m_turnBeginWidget->m_config.m_text = "Player 2's Turn";
		m_playerTurnIndicator->m_config.m_text = "Player 2's Turn";
		m_turnBeginWidget->Build();
		m_playerTurnIndicator->Build();
	}
	else
	{
		m_currentPlayerTurn = Team::BLUE;
		m_turnBeginWidget->m_config.m_text = "Player 1's Turn";
		m_playerTurnIndicator->m_config.m_text = "Player 1's Turn";
		m_turnBeginWidget->Build();
		m_playerTurnIndicator->Build();
	}
	m_currentTurnState = TurnState::TURN_BEGIN;
}

void Game::PlayerQuit()
{
	if (m_isQuitting)
	{
		g_theNetSystem->m_sendQueue.push_back("PlayerQuit");
	}
	else
	{
		m_remotePlayerInGame = false;
	}
}


void Game::Update_Playing(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		Vec3 explosionPos = Vec3(GetHexPosFromGridCoord(m_focusedHexCoords).x, GetHexPosFromGridCoord(m_focusedHexCoords).y, 1.f);
		g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/ExplosionVaporum.xml", explosionPos, EulerAngles(), true, 2.f);
	}
	UNUSED(deltaSeconds);
	UpdateBlueUnitWidget();
	UpdateRedUnitWidget();
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_currentGameState = GameState::PAUSE_MENU;
	}

	RaycastResult3D hexCastResult = RaycastVsHexGrid();
	if (IsControllingCurrentTurn())
	{
		SetFocusedHex(GetHexCoordFromWorldPos(hexCastResult.m_impactPos));
	}
	if (m_currentTurnState == TurnState::TURN_BEGIN)
	{
		m_controlsWidget->m_enabled = false;
		if ((g_theInput->WasKeyJustPressed(KEYCODE_ENTER) || g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE)) && IsControllingCurrentTurn())
		{
			StartTurn();
		}
	}
	else if (m_currentTurnState == TurnState::SELECTING)
	{
		m_controlsWidget->m_enabled = IsControllingCurrentTurn();
		m_controlWidgets[0]->m_enabled = false;
		m_controlWidgets[1]->m_enabled = false;
		m_controlWidgets[2]->m_enabled = true;
		m_controlWidgets[3]->m_enabled = true;
		m_controlWidgets[4]->m_enabled = true;

		for (int i = 0; i < (int)m_allUnits.size(); i++)
		{
			if (m_allUnits[i]->m_coords == m_focusedHexCoords && m_allUnits[i]->m_team == m_currentPlayerTurn)
			{
				m_controlWidgets[0]->m_enabled = true;
				m_controlWidgets[0]->m_children[1]->m_config.m_text = "Select";
				m_controlWidgets[0]->Build();
			}
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
		{
			if (IsControllingCurrentTurn())
			{
				SelectFocusedUnit();
			}
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_ARROW) && IsControllingCurrentTurn())
		{
			SelectPreviousUnit();
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_ARROW) && IsControllingCurrentTurn())
		{
			SelectNextUnit();
		}
		if (g_theInput->WasKeyJustPressed('Y') && IsControllingCurrentTurn())
		{
			EndTurn();
		}
	}
	else if (m_currentTurnState == TurnState::MOVING)
	{
		m_controlsWidget->m_enabled = IsControllingCurrentTurn();
		m_controlWidgets[0]->m_enabled = false;
		m_controlWidgets[1]->m_enabled = true;
		m_controlWidgets[2]->m_enabled = true;
		m_controlWidgets[3]->m_enabled = true;
		m_controlWidgets[4]->m_enabled = false;

		if (m_focusedHexCoords == m_selectedUnit->m_coords)
		{
			m_controlWidgets[0]->m_enabled = true;
			m_controlWidgets[0]->m_children[1]->m_config.m_text = "Stay";
			m_controlWidgets[0]->Build();
		}
		else if (m_selectedUnit->CanMoveToTile(m_focusedHexCoords))
		{
			m_controlWidgets[0]->m_enabled = true;
			m_controlWidgets[0]->m_children[1]->m_config.m_text = "Move";
			m_controlWidgets[0]->Build();
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_ARROW) && IsControllingCurrentTurn())
		{
			SelectPreviousUnit();
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_ARROW) && IsControllingCurrentTurn())
		{
			SelectNextUnit();	
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE) && IsControllingCurrentTurn())
		{
			if (m_selectedUnit->CanMoveToTile(m_focusedHexCoords))
			{
				Move();
			}
			else if (m_focusedHexCoords == m_selectedUnit->m_coords)
			{
				Stay();
			}
			else
			{
				SelectFocusedUnit();
			}
		}
	}
	else if (m_currentTurnState == TurnState::ATTACKING)
	{
		m_controlsWidget->m_enabled = IsControllingCurrentTurn();
		m_controlWidgets[0]->m_enabled = false;
		m_controlWidgets[1]->m_enabled = true;
		m_controlWidgets[1]->m_children[1]->m_config.m_text = "Cancel";
		m_controlWidgets[1]->Build();

		m_controlWidgets[2]->m_enabled = false;
		m_controlWidgets[3]->m_enabled = false;
		m_controlWidgets[4]->m_enabled = false;

		if (m_focusedHexCoords == m_selectedUnit->m_coords)
		{
			m_controlWidgets[0]->m_enabled = true;
			m_controlWidgets[0]->m_children[1]->m_config.m_text = "Hold Fire";
			m_controlWidgets[0]->Build();
		}
		else if (m_selectedUnit->CanAttackTile(m_focusedHexCoords))
		{
			m_controlWidgets[0]->m_enabled = true;
			m_controlWidgets[0]->m_children[1]->m_config.m_text = "Attack";
			m_controlWidgets[0]->Build();
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE) && IsControllingCurrentTurn())
		{
			Cancel();
		}
		else if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE) && IsControllingCurrentTurn())
		{
			if (m_selectedUnit->CanAttackTile(m_focusedHexCoords))
			{
				Attack();
			}
			else if (m_focusedHexCoords == m_selectedUnit->m_coords)
			{
				HoldFire();
			}
		}
	}
	CheckForGameComplete();
}

void Game::Update_PauseMenu(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	int previouslySelectedButton = m_selectedPauseMenuButton;
	if (g_theInput->WasKeyJustPressed(KEYCODE_DOWN_ARROW))
	{
		m_selectedPauseMenuButton++;
		if (m_selectedPauseMenuButton >= (int)m_pauseMenuButtons.size())
		{
			m_selectedPauseMenuButton = 0;
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_UP_ARROW))
	{
		m_selectedPauseMenuButton--;
		if (m_selectedPauseMenuButton < 0)
		{
			m_selectedPauseMenuButton = (int)m_pauseMenuButtons.size() - 1;
		}
	}

	//handle selected button mouse
	for (int i = 0; i < (int)m_pauseMenuButtons.size(); i++)
	{
		if (m_pauseMenuButtons[i]->GetMouseHoveredWidgetInHeirarchy() != nullptr)
		{
			m_selectedPauseMenuButton = i;
		}
	}
	if (previouslySelectedButton != m_selectedPauseMenuButton)
	{
		m_pauseMenuButtons[previouslySelectedButton]->m_config.m_panelColor = Rgba8(0, 0, 0, 0);
		m_pauseMenuButtons[previouslySelectedButton]->m_config.m_textColor = Rgba8::WHITE;
		m_pauseMenuButtons[previouslySelectedButton]->m_config.m_borderColor = Rgba8(0, 0, 0, 0);
		m_pauseMenuButtons[previouslySelectedButton]->Build();

		m_pauseMenuButtons[m_selectedPauseMenuButton]->m_config.m_panelColor = Rgba8::WHITE;
		m_pauseMenuButtons[m_selectedPauseMenuButton]->m_config.m_textColor = Rgba8::BLACK;
		m_pauseMenuButtons[m_selectedPauseMenuButton]->m_config.m_borderColor = Rgba8::BLACK;
		m_pauseMenuButtons[m_selectedPauseMenuButton]->Build();

	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE) || g_theInput->WasKeyJustPressed(KEYCODE_ENTER))
	{
		if (m_selectedPauseMenuButton == 0)
		{
			m_currentGameState = GameState::PLAYING;
		}
		else if (m_selectedPauseMenuButton == 1)
		{
			m_isQuitting = true;
			PlayerQuit();
			g_theApp->ResetGame();
			g_theGame->m_currentGameState = GameState::MAIN_MENU;
		}
	}
	else if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_isQuitting = true;
		PlayerQuit();
		g_theApp->ResetGame();
		g_theGame->m_currentGameState = GameState::MAIN_MENU;
	}
}


void Game::Update_GameComplete(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (m_redTeamWon)
	{
		m_gameCompleteWidget->m_config.m_text = "Player 2 Wins!";
	}
	else if (m_blueTeamWon)
	{
		m_gameCompleteWidget->m_config.m_text = "Player 1 Wins!";
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		g_theApp->ResetGame();
		g_theGame->m_currentGameState = GameState::MAIN_MENU;
	}
}

bool Game::Event_PlayerReady(EventArgs& args)
{
	UNUSED(args);
	g_theGame->PlayerReady();

	return true;
}

bool Game::Event_SetFocusedHex(EventArgs& args)
{
	IntVec2 coords = args.GetValue("Coords", IntVec2(-1, -1));
	g_theGame->SetFocusedHex(coords);
	return false;
}

bool Game::Event_StartTurn(EventArgs& args)
{
	UNUSED(args);
	g_theGame->StartTurn();
	return true;
}

bool Game::Event_EndTurn(EventArgs& args)
{
	UNUSED(args);
	g_theGame->EndTurn();
	return true;
}



bool Game::Event_SelectFocusedUnit(EventArgs& args)
{
	UNUSED(args);
	g_theGame->SelectFocusedUnit();
	return true;
}

bool Game::Event_SelectNextUnit(EventArgs& args)
{
	UNUSED(args);
	g_theGame->SelectNextUnit();
	return true;
}

bool Game::Event_SelectPreviousUnit(EventArgs& args)
{
	UNUSED(args);
	g_theGame->SelectPreviousUnit();
	return true;
}

bool Game::Event_Move(EventArgs& args)
{
	UNUSED(args);
	g_theGame->Move();
	return true;
}

bool Game::Event_Cancel(EventArgs& args)
{
	UNUSED(args);
	g_theGame->Cancel();
	return true;
}

bool Game::Event_Stay(EventArgs& args)
{
	UNUSED(args);
	g_theGame->Stay();
	return true;
}

bool Game::Event_Attack(EventArgs& args)
{
	UNUSED(args);
	g_theGame->Attack();
	return true;
}

void Game::CheckForGameComplete()
{
	if (m_player2Units.size() == 0 || (!m_remotePlayerInGame && m_multiplayerTeam == Team::BLUE))
	{
		m_blueTeamWon = true;
		m_currentGameState = GameState::GAME_COMPLETE;
	}
	if (m_player1Units.size() == 0 || (!m_remotePlayerInGame && m_multiplayerTeam == Team::RED))
	{
		m_redTeamWon = true;
		m_currentGameState = GameState::GAME_COMPLETE;
	}
}

bool Game::Event_HoldFire(EventArgs& args)
{
	UNUSED(args);
	g_theGame->HoldFire();
	return true;
}

bool Game::Event_PlayerQuit(EventArgs& args)
{
	UNUSED(args);
	g_theGame->PlayerQuit();
	return true;
}


void Game::PlayerReady()
{
	m_remotePlayerInGame = true;
}

void Game::StartTurn()
{
	if (IsMultiplayerTeamsTurn())
	{
		g_theNetSystem->m_sendQueue.push_back("StartTurn");
	}
	m_currentTurnState = TurnState::SELECTING;
}

void Game::Stay()
{
	if (IsMultiplayerTeamsTurn())
	{
		g_theNetSystem->m_sendQueue.push_back("Stay");
	}
	m_selectedUnit->m_isStaying = true;
	m_currentTurnState = TurnState::ATTACKING;
}
