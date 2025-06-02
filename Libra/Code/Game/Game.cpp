#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Game/Tile.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/Image.hpp"
#include "Game/MapDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

Game::Game(App* app)
	: m_theApp(app)
{
	const float screenWidth = g_gameConfigBlackboard.GetValue("screenWidth", 16.f);
	const float screenHeight = g_gameConfigBlackboard.GetValue("screenHeight", 8.f);
	const float worldToUIScale =  g_gameConfigBlackboard.GetValue("worldToUIScale", 8.f);
	m_worldCamera.m_mode = Camera::eMode_Orthographic;
	m_worldCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(screenWidth, screenHeight));

	m_screenCamera.m_mode = Camera::eMode_Orthographic;
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(screenWidth * worldToUIScale, screenHeight * worldToUIScale));

	m_debugCamera.m_mode = Camera::eMode_Orthographic;
	m_debugCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(screenWidth * 6.f, screenHeight * 6.f));
}

Game::~Game() {}

void Game::StartUp()
{
	m_gameState = GAME_STATE_ATTRACT;
	//Play Button
	m_attractPlayVerts[0] = Vertex_PCU(Vec3(1, 0, 0), Rgba8(61, 186, 86, 255));
	m_attractPlayVerts[1] = Vertex_PCU(Vec3(-1, 1, 0), Rgba8(61, 186, 86, 255));
	m_attractPlayVerts[2] = Vertex_PCU(Vec3(-1, -1, 0), Rgba8(61, 186, 86, 255));

	m_placeholderGameverts[0] = Vertex_PCU(Vec3(-1.f, 1.f, 0.f), Rgba8(0, 0, 255, 0));
	m_placeholderGameverts[1] = Vertex_PCU(Vec3(-1.f, -1.f, 0.f), Rgba8(0, 0, 255, 255));
	m_placeholderGameverts[2] = Vertex_PCU(Vec3(1.f, -1.f, 0.f), Rgba8(0, 0, 255, 255));
	
	m_placeholderGameverts[3] = Vertex_PCU(Vec3(1.f, 1.f, 0.f), Rgba8(0, 0, 255, 255));
	m_placeholderGameverts[4] = Vertex_PCU(Vec3(1.f, -1.f, 0.f), Rgba8(0, 0, 255, 255));
	m_placeholderGameverts[5] = Vertex_PCU(Vec3(-1.f, 1.f, 0.f), Rgba8(0, 0, 255, 0));

	LoadAssets();
	InitializeTileDefinitions();
	m_startMusicPlayback = g_theAudio->StartSound(SOUND_ID_STARTUP_MUSIC, true);
}

void Game::LoadAssets()
{
	//font
	g_bitmapFont = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Fonts/SquirrelFixedFont");

	//sound
	SOUND_ID_STARTUP_MUSIC = g_theAudio->CreateOrGetSound("Data/Audio/AttractMusic.mp3");
	SOUND_ID_GAMEPLAY_MUSIC = g_theAudio->CreateOrGetSound("Data/Audio/GameplayMusic.mp3");
	SOUND_ID_START = g_theAudio->CreateOrGetSound("Data/Audio/Click.mp3");
	SOUND_ID_ENEMY_HIT = g_theAudio->CreateOrGetSound("Data/Audio/EnemyHit.wav");
	SOUND_ID_ENEMY_DIED = g_theAudio->CreateOrGetSound("Data/Audio/EnemyDied.wav");
	SOUND_ID_ENEMY_SHOOT = g_theAudio->CreateOrGetSound("Data/Audio/EnemyShoot.wav");
	SOUND_ID_PAUSE = g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3");
	SOUND_ID_PLAYER_DIED = g_theAudio->CreateOrGetSound("Data/Audio/GameOver.mp3");
	SOUND_ID_PLAYER_HIT = g_theAudio->CreateOrGetSound("Data/Audio/PlayerHit.wav");
	SOUND_ID_PLAYER_SHOOT = g_theAudio->CreateOrGetSound("Data/Audio/PlayerShootNormal.ogg");
	SOUND_ID_QUIT = g_theAudio->CreateOrGetSound("Data/Audio/ExitMap.wav");
	SOUND_ID_UNPAUSE = g_theAudio->CreateOrGetSound("Data/Audio/Unpause.mp3");
	SOUND_ID_VICTORY = g_theAudio->CreateOrGetSound("Data/Audio/Victory.mp3");
	SOUND_ID_WELCOME = g_theAudio->CreateOrGetSound("Data/Audio/Welcome.mp3");
	SOUND_ID_BULLET_RICOCHET1 = g_theAudio->CreateOrGetSound("Data/Audio/BulletRicochet.wav");
	SOUND_ID_BULLET_RICOCHET2 = g_theAudio->CreateOrGetSound("Data/Audio/BulletRicochet2.wav");
	SOUND_ID_DISCOVER = g_theAudio->CreateOrGetSound("Data/Audio/DiscoverSound.wav");

	//texture
	TEXTURE_ARIES = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTank2.png");
	TEXTURE_CAPRICORN = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTank3.png");
	TEXTURE_GOOD_BULLET = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/FriendlyBolt.png");
	TEXTURE_EVIL_BULLET =  g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyBolt.png");
	TEXTURE_WIN_SCREEN = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/AttractScreen.png");
	TEXTURE_MISSILE = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyShell.png");
	TEXTURE_TILE_SHEET = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	TEXTURE_EXPLOSION_SHEET = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Explosion_5x5.png");
	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Test_StbiFlippedAndOpenGL.png");
	m_loseScreenTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/YouDiedScreen.png");
	m_winScreenTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/VictoryScreen.jpg");

	//sprite sheet
	SPRITE_SHEET_TILES = new SpriteSheet(TEXTURE_TILE_SHEET, IntVec2(8, 8));
	SPRITE_SHEET_EXPLOSION = new SpriteSheet(TEXTURE_EXPLOSION_SHEET, IntVec2(5,5));

	//animation
	ANIM_DEFINITION_EXPLOSION = new SpriteAnimDefinition(*SPRITE_SHEET_EXPLOSION, 0, 24, .5f);
}

void Game::PauseGame()
{
	m_isPaused = true;
	g_theAudio->SetSoundPlaybackSpeed(m_gamePlayMusicPlayback, 0.f);
	g_theAudio->StartSound(SOUND_ID_PAUSE);
}

void Game::InitializeMaps()
{

	MapDefinition::InitializeMapDefs();
	m_maps.push_back( new Map( *MapDefinition::GetMapDefFromName("Approach") ) );
	m_maps[0]->StartUp();

	m_maps.push_back( new Map( *MapDefinition::GetMapDefFromName("Courtyard") ) );
	m_maps[1]->StartUp();

	m_maps.push_back( new Map( *MapDefinition::GetMapDefFromName("Tunnel") ) );
	m_maps[2]->StartUp();

	m_maps.push_back( new Map(*MapDefinition::GetMapDefFromName("Fortress") ) );
	m_maps[3]->StartUp();

	m_maps.push_back( new Map(*MapDefinition::GetMapDefFromName("Sanctum") ) );
	m_maps[4]->StartUp();

	m_maps.push_back( new Map(*MapDefinition::GetMapDefFromName("TestZoo1") ) );
	m_maps[5]->StartUp();

	m_currentMap = m_maps[0];
	m_currMapIdx = 0;
	AdjustDebugCameraForCurrentMap();
}

void Game::SwitchMaps()
{
	m_currMapIdx++;
	if (m_currMapIdx >= (int)m_maps.size())
	{
		m_isPaused = true;
		m_gameState = GAME_STATE_WIN;
		g_theAudio->SetSoundPlaybackSpeed(m_gamePlayMusicPlayback, 0.f);
		g_theAudio->StartSound(SOUND_ID_VICTORY);
		return;
	}
	g_theAudio->StartSound(SOUND_ID_QUIT);
	Map* nextMap = m_maps[m_currMapIdx];
	for (size_t i = 0; i < m_currentMap->m_entityListsByType[ENTITY_TYPE_GOOD_PLAYER].size(); i++)
	{
		Entity* player = m_currentMap->m_entityListsByType[ENTITY_TYPE_GOOD_PLAYER][i];
		player->m_position = Vec2(1.5f, 1.5f);
		player->m_orientationDegrees = 45.f;
		m_currentMap->RemoveEntityFromMap(player);
		nextMap->AddEntityToMap(player);
	}
	m_currentMap = nextMap;
	AdjustDebugCameraForCurrentMap();
}

void Game::Update_Attract(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	UpdatePlaceHolderAttractScreen();
	//check for start game
	if (g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('P'))
	{
		StartGame();
	}
	const XboxController controller = g_theInput->GetController(0);
	if (controller.IsConnected() && (controller.WasButtonJustPressed(XboxController::A_BUTTON) || controller.WasButtonJustPressed(XboxController::START_BUTTON)))
	{
		StartGame();
	}
}

void Game::Update_Playing(float deltaSeconds)
{
	HandleSpecialGameplayInputs();
	m_debugTextVerts.clear();
	if (m_isPaused)
	{
		return;
	}
	if (m_losePending)
	{
		m_currLoseTime -= deltaSeconds;
		if (m_currLoseTime <= 0.f)
		{
			m_gameState = GAME_STATE_LOSE;
			m_isPaused = true;
		}
	}
	m_currentMap->Update(deltaSeconds);

	//fps
	//g_bitmapFont->AddVertsForText2D(m_debugTextVerts, m_screenCamera.GetOrthoTopRight() - Vec2(40.f, 10.f), 3.f, Stringf("FPS: %.1f", 1.f/ deltaSeconds));
}

void Game::Render_Attract() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	RenderPlaceHolderAttractScreen();
}

void Game::Render_Playing() const
{
	//World Space Rendering
	if (g_theApp->m_debugCamera)
	{
		g_theRenderer->BeginCamera(m_debugCamera);
	}
	else
	{
		g_theRenderer->BeginCamera(m_worldCamera);
	}
	m_currentMap->RenderMapTiles();
	m_currentMap->DrawCurrentHeatMap();
	m_currentMap->RenderMapEntities();
	m_currentMap->RenderMapHealthBars();
	if (g_gameConfigBlackboard.GetValue("debugMode", false))
	{
		m_currentMap->RenderDebugMapEntities();
	}
	if (g_theApp->m_debugCamera)
	{
		g_theRenderer->EndCamera(m_debugCamera);
	}
	else
	{
		g_theRenderer->EndCamera(m_worldCamera);
	}
	//ScreenSpace Rendering
	g_theRenderer->BeginCamera(m_screenCamera);
	if (m_gameState == GAME_STATE_LOSE)
	{
		DrawLoseScreen();
	}
	else if (m_gameState == GAME_STATE_WIN)
	{
		DrawWinScreen();
	}
	else if (m_isPaused)
	{
		DrawPausedEffect();
	}

	if (m_debugTextVerts.size() > 0)
	{
		g_theRenderer->BindTexture(g_bitmapFont->GetTexture());
		g_theRenderer->DrawVertexArray(m_debugTextVerts.size(), m_debugTextVerts.data());
	}
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::AdjustDebugCameraForCurrentMap()
{
	if (m_currentMap->m_dimensions.x >= 2 * m_currentMap->m_dimensions.y)
	{
		m_debugCamera.SetCameraHeight((float)m_currentMap->m_dimensions.x * .5f);
	}
	else
	{
		m_debugCamera.SetCameraHeight((float)m_currentMap->m_dimensions.y);
	}
}

void Game::InitializeTileDefinitions()
{
	SPRITE_SHEET_TILES = new SpriteSheet(TEXTURE_TILE_SHEET, IntVec2(8, 8));
	TileDefinition::InitializeTileDefs(*SPRITE_SHEET_TILES);
}

void Game::HandleSpecialGameplayInputs()
{
	XboxController controller = g_theInput->GetController(0);
	if (m_gameState == GAME_STATE_LOSE)
	{
		if (g_theInput->WasKeyJustPressed('N') || (controller.IsConnected() && controller.WasButtonJustPressed(XboxController::START_BUTTON)))
		{
			RespawnPlayer();
		}
		return;
	}
	if (g_theInput->WasKeyJustPressed('P') || (controller.IsConnected() && controller.WasButtonJustPressed(XboxController::START_BUTTON)))
	{
		if (m_gameState == GAME_STATE_WIN)
		{
			g_theApp->m_pendingGameReset = true;
		}
		m_isPaused = !m_isPaused;
		if (m_isPaused)
		{
			g_theAudio->StartSound(SOUND_ID_PAUSE);
			g_theAudio->SetSoundPlaybackSpeed(m_gamePlayMusicPlayback, 0.f);
		}
		else
		{
			g_theAudio->StartSound(SOUND_ID_UNPAUSE);
			g_theAudio->SetSoundPlaybackSpeed(m_gamePlayMusicPlayback, 1.f);
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_currentMap->CycleDebugHeatMap();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
	{
		SwitchMaps();
	}
}

void Game::RespawnPlayer()
{
	m_losePending = false;
	m_currLoseTime = m_playOutLoseTime;
	m_currentMap->RespawnPlayer();
	m_isPaused = false;
	m_gameState = GAME_STATE_PLAYING;
}

void Game::StartGame()
{
	m_gameState = GAME_STATE_PLAYING;
	InitializeMaps();
	SetNumTilesInViewVertically(m_numTilesInViewVertically);
	g_theAudio->StopSound(m_startMusicPlayback);
	m_gamePlayMusicPlayback =  g_theAudio->StartSound(SOUND_ID_GAMEPLAY_MUSIC, true);
	g_theAudio->StartSound(SOUND_ID_START);
	m_currentMap->AddEntityToMap(new Player(Vec2(1.5f, 1.5f), 45.f));
	m_currLoseTime = m_playOutLoseTime;
}

void Game::DrawPausedEffect() const
{
	std::vector<Vertex_PCU> pauseQuadVerts;
	AddVertsForAABB2D(pauseQuadVerts, m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight(), Rgba8(0 ,0 ,0, 125));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(pauseQuadVerts.size(), pauseQuadVerts.data());
}

void Game::DrawLoseScreen() const
{
	std::vector<Vertex_PCU> loseScreenVerts;
	AddVertsForAABB2D(loseScreenVerts, m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight(), Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(m_loseScreenTexture);
	g_theRenderer->DrawVertexArray(loseScreenVerts.size(), loseScreenVerts.data());
}

void Game::DrawWinScreen() const
{
	std::vector<Vertex_PCU> winScreenVerts;
	AddVertsForAABB2D(winScreenVerts, m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight(), Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(m_winScreenTexture);
	g_theRenderer->DrawVertexArray(winScreenVerts.size(), winScreenVerts.data());
}

void Game::RenderPlaceHolderAttractScreen() const
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);

	std::vector<Vertex_PCU> startScreenVerts;
	AddVertsForAABB2D(startScreenVerts, m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight(), Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(TEXTURE_WIN_SCREEN);
	g_theRenderer->DrawVertexArray(startScreenVerts.size(), startScreenVerts.data());
}

void Game::UpdatePlaceHolderAttractScreen()
{
	float currT = static_cast<float>(GetCurrentTimeSeconds());
	m_attractAlpha = RangeMap(sinf(currT * 2.f), -1.f, 1.f, .5f, 1.f);
}

void Game::Update(float deltaSeconds)
{
	if (m_gameState == GAME_STATE_ATTRACT)
	{
		Update_Attract(deltaSeconds);
	}
	else
	{
		Update_Playing(deltaSeconds);
	}
}

void Game::Render() const 
{
	if (m_gameState == GAME_STATE_ATTRACT)
	{
		Render_Attract();
	}
	else
	{
		Render_Playing();
	}
}

void Game::ShutDown()
{
	delete m_tilesSpriteSheet;
	g_theAudio->StopSound(m_gamePlayMusicPlayback);
	g_theAudio->StopSound(m_startMusicPlayback);
	if (m_currentMap)
	{
		m_currentMap->ShutDown();
	}
}


void Game::SetNumTilesInViewVertically(float newNumTilesInViewVertically)
{
	m_numTilesInViewVertically = newNumTilesInViewVertically;
	m_worldCamera.SetCameraHeight(m_numTilesInViewVertically);
}


