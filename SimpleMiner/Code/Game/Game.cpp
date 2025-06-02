#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/World.hpp"

World* g_theWorld = nullptr;

Game::Game(App* app)
	: m_theApp(app)
{}

Game::~Game() {}

void Game::StartUp()
{
	m_player = new Player(this, Vec3::ZERO);
	m_allEntities.push_back(m_player);
	//DebugAddWorldBasis(Mat44(), -1.f, DebugRenderMode::ALWAYS);

	font = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Fonts/SquirrelFixedFont");
	m_screenCamera.SetOrthographicView(Vec2::ZERO, Vec2(1600.f, 800.f));

	m_screenCamera.m_mode = Camera::eMode_Orthographic;
	m_player->m_playerCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.f, .1f, 1000.f);
	m_player->m_playerCamera.m_mode = Camera::eMode_Perspective;
	m_player->m_playerCamera.SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.f, 1.f, 0.f));
	m_player->m_position = Vec3(-2.f, 0.f, 90.f);
	m_player->m_playerCamera.m_position = m_player->m_position;

	LoadAssets();

	StartGame();
}

void Game::StartGame()
{
	g_theWorld = new World();
	g_theWorld->StartUp();
}

void Game::CheckForDebugCommands()
{
	if (g_theInput->WasKeyJustPressed('1'))
	{
		DebugAddWorldLine(m_player->m_position, m_player->m_position + m_player->GetForwardNormal() * 20.f, .05f, 10.f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::X_RAY);
	}
	if (g_theInput->IsKeyDown('2'))
	{
		DebugAddWorldPoint(Vec3(m_player->m_position.x, m_player->m_position.y, 0.f), .25f, 60.f, Rgba8(150, 75, 0), Rgba8(150, 75, 0), DebugRenderMode::USE_DEPTH);
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		DebugAddWorldWireSphere(m_player->m_position + m_player->GetForwardNormal() * 2.f, 1.f, 5.f, Rgba8::GREEN, Rgba8::RED, DebugRenderMode::USE_DEPTH);
	}
	if (g_theInput->WasKeyJustPressed('4'))
	{
		Mat44 playerTrans;
		playerTrans.AppendTranslation3D(m_player->m_position);
		playerTrans.Append(m_player->m_orientationDegrees.GetAsMatrix_IFwd_JLeft_KUp());
		DebugAddWorldBasis(playerTrans, 20.f);
	}
	if (g_theInput->WasKeyJustPressed('5'))
	{
		std::string outputString = Stringf("Position: %.1f, %.1f, %.1f Orientation: %.1f, %.1f, %.1f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z, 
			m_player->m_orientationDegrees.m_yaw, m_player->m_orientationDegrees.m_pitch, m_player->m_orientationDegrees.m_roll);
		DebugAddWorldBillboardText(outputString, m_player->m_position, .1f, Vec2(.5f, .5f), 10.f, Rgba8::WHITE, Rgba8::RED, DebugRenderMode::USE_DEPTH);
	}
	if (g_theInput->WasKeyJustPressed('6'))
	{
		DebugAddWorldWireCylinder(m_player->m_position, m_player->m_position + Vec3(0.f, 0.f, 2.f), .5f, 10.f, Rgba8::WHITE, Rgba8::RED);
	}
	if (g_theInput->WasKeyJustPressed('7'))
	{
		std::string outputString = Stringf("Orientation: %.1f, %.1f, %.1f",
			m_player->m_orientationDegrees.m_yaw, m_player->m_orientationDegrees.m_pitch, m_player->m_orientationDegrees.m_roll);
		DebugAddMessage(outputString, 20.f, 5.f);
	}
}

void Game::LoadAssets()
{
	SPRITE_SHEET_BASIC_SPRITES = new SpriteSheet(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/BasicSprites_64x64.png"), IntVec2(64, 64));

	//BlockDefs							name		visible	solid	opaque	topSprite			sideSprite			bottomSprite			//blockType
	BlockDefinition::CreateNewBlockDef("Air",		false,	false,	false,	IntVec2::ZERO,		IntVec2::ZERO,		IntVec2::ZERO);			//0
	BlockDefinition::CreateNewBlockDef("Stone",		true,	true,	true,	IntVec2(33,32),		IntVec2(33, 32),	IntVec2(33, 32));		//1
	BlockDefinition::CreateNewBlockDef("Dirt",		true,	true,	true,	IntVec2(32, 34),	IntVec2(32, 34),	IntVec2(32, 34));		//2
	BlockDefinition::CreateNewBlockDef("Grass",		true,	true,	true,	IntVec2(32, 33),	IntVec2(33, 33),	IntVec2(32, 34));		//3
	BlockDefinition::CreateNewBlockDef("Coal",		true,	true,	true,	IntVec2(63, 34),	IntVec2(63, 34),	IntVec2(63, 34));		//4
	BlockDefinition::CreateNewBlockDef("Iron",		true,	true,	true,	IntVec2(63, 35),	IntVec2(63, 35),	IntVec2(63, 35));		//5
	BlockDefinition::CreateNewBlockDef("Gold",		true,	true,	true,	IntVec2(63, 36),	IntVec2(63, 36),	IntVec2(63, 36));		//6
	BlockDefinition::CreateNewBlockDef("Diamond",	true,	true,	true,	IntVec2(63, 37),	IntVec2(63, 37),	IntVec2(63, 37));		//7
	BlockDefinition::CreateNewBlockDef("Water",		true,	true,	true,	IntVec2(32, 44),	IntVec2(32, 44),	IntVec2(32, 44));		//8
	BlockDefinition::CreateNewBlockDef("Brick",		true,	true,	true,	IntVec2(42, 40),	IntVec2(42, 40),	IntVec2(42, 40));		//9
	BlockDefinition::CreateNewBlockDef("Sand",		true,	true,	true,	IntVec2(34, 34),	IntVec2(34, 34),	IntVec2(34, 34));		//10
	BlockDefinition::CreateNewBlockDef("Glowstone",	true,	true,	true,	IntVec2(46, 34),	IntVec2(46, 34),	IntVec2(46, 34), 15);	//11
	BlockDefinition::CreateNewBlockDef("Ice",		true,	true,	true,	IntVec2(36, 35),	IntVec2(36, 35),	IntVec2(36, 35));		//12
	BlockDefinition::CreateNewBlockDef("Cactus",	true,	true,	true,	IntVec2(38, 36),	IntVec2(37, 36),	IntVec2(39, 36));		//13
	BlockDefinition::CreateNewBlockDef("OakLog",	true,	true,	true,	IntVec2(38, 33),	IntVec2(36, 33),	IntVec2(38, 33));		//14
	BlockDefinition::CreateNewBlockDef("OakLeaf",	true,	true,	true,	IntVec2(32, 35),	IntVec2(32, 35),	IntVec2(32, 35));		//15
	BlockDefinition::CreateNewBlockDef("SpruceLog",	true,	true,	true,	IntVec2(38, 33),	IntVec2(35, 33),	IntVec2(38, 33));		//16
	BlockDefinition::CreateNewBlockDef("SpruceLeaf",true,	true,	true,	IntVec2(34, 35),	IntVec2(34, 35),	IntVec2(34, 35));		//17
}	

void Game::UnloadAssets()
{
	delete SPRITE_SHEET_BASIC_SPRITES;
	BlockDefinition::DeleteBlockDefinitions();
}

void Game::Update(float deltaSeconds)
{
	g_theWorld->Update(deltaSeconds);
	for (size_t i = 0; i < m_allEntities.size(); i++)
	{
		m_allEntities[i]->Update(deltaSeconds);
	}
	CheckForDebugCommands();
}

void Game::Render() const 
{
	//World Space Rendering
	g_theRenderer->BeginCamera(m_player->m_playerCamera);
	g_theWorld->Render();
	for (size_t i = 0; i < m_allEntities.size(); i++)
	{
		m_allEntities[i]->Render();
	}
	g_theRenderer->EndCamera(m_player->m_playerCamera);
	DebugRenderWorld(m_player->m_playerCamera);

	//ScreenSpace Rendering
	DebugRenderScreen(m_screenCamera);
}

void Game::ShutDown()
{
	UnloadAssets();
	DebugRenderClear();
	g_theWorld->ShutDown();
}


