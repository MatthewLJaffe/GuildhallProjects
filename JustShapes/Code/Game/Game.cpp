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
#include "Game/GameStateMainMenu.hpp"
#include "Game/GameStateHowToPlay.hpp"
#include "Game/GameStateSettings.hpp"
#include "Game/GameStateLevelSelect.hpp"
#include "Game/GameStateLevel1.hpp"
#include "Game/GameStateLevel2.hpp"
#include "Game/GameStateLevel3.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Window.hpp"


Game* g_theGame = nullptr;

Game::Game()
{
}

Game::~Game() {}

void Game::StartUp()
{
	IntVec2 screenDimensions = g_theWindow->GetClientDimensions();
	float aspectRatio = (float)screenDimensions.x / (float)screenDimensions.y;
	m_worldCamera.m_mode = Camera::eMode_Orthographic;
	m_worldCamera.SetOrthographicView(Vec2::ZERO, Vec2(240.f * aspectRatio, 240.f));

	m_screenCamera.m_mode = Camera::eMode_Orthographic;
	m_screenCamera.SetOrthographicView(Vec2::ZERO, Vec2((float)screenDimensions.x, (float)screenDimensions.y));

	m_screenShakeTimer = Timer(1.f, g_theApp->GetGameClock());
	StartGame();
}

void Game::StartGame()
{
	m_gameStates.resize((size_t)GameStateType::COUNT);
	PlaySound(SOUND_ID_MENU_MUSIC, SoundType::MUSIC, true, .5f);
	GameStateMainMenu* mainMenu = new GameStateMainMenu(GameStateType::MAIN_MENU);
	mainMenu->StartUp();
	m_gameStates[(int)GameStateType::MAIN_MENU] = mainMenu;

	GameStateHowToPlay* howToPlay = new GameStateHowToPlay(GameStateType::HOW_TO_PLAY);
	howToPlay->StartUp();
	m_gameStates[(int)GameStateType::HOW_TO_PLAY] = howToPlay;

	GameStateSettings* settings = new GameStateSettings(GameStateType::SETTINGS);
	settings->StartUp();
	m_gameStates[(int)GameStateType::SETTINGS] = settings;

	GameStateLevelSelect* levelSelect = new GameStateLevelSelect(GameStateType::LEVEL_SELECT);
	levelSelect->StartUp();
	m_gameStates[(int)GameStateType::LEVEL_SELECT] = levelSelect;

	GameStateLevel1* level1 = new GameStateLevel1(GameStateType::LEVEL_1, SOUND_ID_LEVEL_1_MUSIC);
	level1->StartUp();
	m_gameStates[(int)GameStateType::LEVEL_1] = level1;

	GameStateLevel2* level2 = new GameStateLevel2(GameStateType::LEVEL_2, g_theAudio->CreateOrGetSound("Data/Audio/La Danse Macabre.mp3"));
	level2->StartUp();
	m_gameStates[(int)GameStateType::LEVEL_2] = level2;

	GameStateLevel3* level3 = new GameStateLevel3(GameStateType::LEVEL_3, g_theAudio->CreateOrGetSound("Data/Audio/Close to Me Shorter.wav"));
	level3->StartUp();
	m_gameStates[(int)GameStateType::LEVEL_3] = level3;

	m_currentGameState = m_gameStates[(int)GameStateType::MAIN_MENU];
}

void Game::Update(float deltaSeconds)
{
	if (m_nextGamestate != nullptr)
	{
		m_currentGameState->OnDisable();
		m_currentGameState = m_nextGamestate;
		m_currentGameState->OnEnable();
		m_nextGamestate = nullptr;
	}
	if (m_currentGameState->m_needsReset)
	{
		RestartCurrentState();
	}
	m_currentGameState->Update(deltaSeconds);
	if (!m_screenShakeTimer.IsStopped())
	{
		if (!m_screenShakeTimer.HasPeriodElapsed())
		{
			UpdateScreenShake();
		}
		else
		{
			m_screenShakeTimer.Stop();
			m_worldCamera.SetCameraPos(m_cameraPosBeforeScreenShake);
		}
	}
}

void Game::Render() const 
{
	//World Space Rendering
	m_currentGameState->Render();
	DebugRenderScreen(g_theGame->m_screenCamera);
}

void Game::ShutDown()
{
	DebugRenderClear();
	for (int i = 0; i < (int)m_gameStates.size(); i++)
	{
		delete m_gameStates[i];
		m_gameStates[i] = nullptr;
	}
}

void Game::SwitchGameState(GameStateType newGameState)
{
	for (int i = 0; i < (int)m_gameStates.size(); i++)
	{
		if (m_gameStates[i]->m_gameStateType == newGameState)
		{
			m_nextGamestate = m_gameStates[i];
			break;
		}
	}
}

void Game::PlaySound(SoundID soundID, SoundType soundType, bool isLooped, float volume, float balance, float speed, float timeOffset)
{
	float soundVolume = volume * g_gameConfigBlackboard.GetValue("masterVolume", .5f);
	if (soundType == SoundType::MUSIC)
	{
		if (m_currentMusic != MISSING_SOUND_ID)
		{
			g_theAudio->StopSound(m_currentMusic);
		}
		soundVolume *= g_gameConfigBlackboard.GetValue("musicVolume", .5f);
		m_baseMusicVolume = volume;
		m_currentMusic = g_theAudio->StartSound(soundID, isLooped, soundVolume, balance, speed, false, timeOffset);
	}
	else if (soundType == SoundType::SFX)
	{
		soundVolume *= g_gameConfigBlackboard.GetValue("sfxVolume", .5f);
		g_theAudio->StartSound(soundID, isLooped, soundVolume, balance, speed, false, timeOffset);
	}
}

void Game::UpdateMusicVolume()
{
	if (m_currentMusic != MISSING_SOUND_ID)
	{
		float soundVolume = m_baseMusicVolume * g_gameConfigBlackboard.GetValue("masterVolume", .5f) * g_gameConfigBlackboard.GetValue("musicVolume", .5f);
		g_theAudio->SetSoundPlaybackVolume(m_currentMusic, soundVolume);
	}
}

void Game::ResetCurrentState()
{
	GameStateType currentGameState = m_currentGameState->m_gameStateType;
	delete m_currentGameState;
	if (currentGameState == GameStateType::HOW_TO_PLAY)
	{
		m_gameStates[(int)currentGameState] = new GameStateHowToPlay(currentGameState);
		m_gameStates[(int)currentGameState]->StartUp();
	}
	if (currentGameState == GameStateType::LEVEL_1)
	{
		m_gameStates[(int)currentGameState] = new GameStateLevel1(currentGameState, SOUND_ID_LEVEL_1_MUSIC);
		m_gameStates[(int)currentGameState]->StartUp();
	}
	if (currentGameState == GameStateType::LEVEL_2)
	{
		m_gameStates[(int)currentGameState] = new GameStateLevel2(currentGameState, g_theAudio->CreateOrGetSound("Data/Audio/La Danse Macabre.mp3"));
		m_gameStates[(int)currentGameState]->StartUp();
	}
	if (currentGameState == GameStateType::LEVEL_3)
	{
		m_gameStates[(int)currentGameState] = new GameStateLevel3(currentGameState, g_theAudio->CreateOrGetSound("Data/Audio/Close to Me Shorter.wav"));
		m_gameStates[(int)currentGameState]->StartUp();
	}
	if (currentGameState == GameStateType::LEVEL_SELECT)
	{
		m_gameStates[(int)currentGameState] = new GameStateLevelSelect(currentGameState);
		m_gameStates[(int)currentGameState]->StartUp();
	}
	if (currentGameState == GameStateType::MAIN_MENU)
	{
		m_gameStates[(int)currentGameState] = new GameStateMainMenu(currentGameState);
		m_gameStates[(int)currentGameState]->StartUp();
	}
	if (currentGameState == GameStateType::SETTINGS)
	{
		m_gameStates[(int)currentGameState] = new GameStateSettings(currentGameState);
		m_gameStates[(int)currentGameState]->StartUp();
	}
	m_currentGameState = m_gameStates[(int)currentGameState];
}

void Game::RestartCurrentState()
{
	ResetCurrentState();
	m_currentGameState->OnEnable();
}

void Game::AdvanceLevel()
{
	if (m_currentGameState->m_gameStateType == GameStateType::LEVEL_1)
	{
		SwitchGameState(GameStateType::LEVEL_2);
	}
	if (m_currentGameState->m_gameStateType == GameStateType::LEVEL_2)
	{
		SwitchGameState(GameStateType::LEVEL_3);
	}
	if (m_currentGameState->m_gameStateType == GameStateType::LEVEL_3)
	{
		SwitchGameState(GameStateType::MAIN_MENU);
	}
}

void Game::PauseMusic()
{
	g_theAudio->SetSoundPlaybackSpeed(m_currentMusic, 0.f);
}

void Game::ResumeMusic()
{
	g_theAudio->SetSoundPlaybackSpeed(m_currentMusic, 1.f);
}

Vec2 Game::GetRandomPosInWorldScreen(Vec2 const& normalizedPadding)
{
	AABB2 cameraBounds(g_theGame->m_worldCamera.GetOrthoBottomLeft(), g_theGame->m_worldCamera.GetOrthoTopRight());
	Vec2 paddingDimensions = cameraBounds.GetDimensions();
	paddingDimensions.x *= normalizedPadding.x;
	paddingDimensions.y *= normalizedPadding.y;
	return cameraBounds.GetRandomPointInsideBox(g_randGen, paddingDimensions);
}

Vec2 Game::GetRandomPosOffScreen(float distanceOffScreen)
{
	AABB2 cameraBounds(g_theGame->m_worldCamera.GetOrthoBottomLeft(), g_theGame->m_worldCamera.GetOrthoTopRight());
	return cameraBounds.GetRandomPointOutsideBox(distanceOffScreen, g_randGen);
}

void Game::SetWorldCameraPos(Vec2 newPos)
{
	Vec2 worldCameraDimensions = m_worldCamera.GetOrthoDimensions();
	Vec2 bottomLeft(newPos.x - worldCameraDimensions.x * .5f, newPos.y - worldCameraDimensions.y * .5f);
	Vec2 topRight(newPos.x + worldCameraDimensions.x * .5f, newPos.y + worldCameraDimensions.y * .5f);
	m_worldCamera.SetOrthographicView(bottomLeft, topRight);
}

void Game::SetMusicSpeed(float musicSpeed)
{
	if (!g_theApp->m_isPaused)
	{
		g_theAudio->SetSoundPlaybackSpeed(m_currentMusic, musicSpeed);
	}
}

void Game::SetMusicVolume(float volume0To1)
{
	float normalVolume = m_baseMusicVolume* g_gameConfigBlackboard.GetValue("masterVolume", .5f)* g_gameConfigBlackboard.GetValue("musicVolume", .5f);
	g_theAudio->SetSoundPlaybackVolume(m_currentMusic, normalVolume * volume0To1);
}

void Game::ShakeScreen(float time, float intensity)
{
	m_cameraPosBeforeScreenShake = m_worldCamera.GetCameraPos();
	m_screenShakeTimer.m_period = time;
	m_screenShakeIntensity = intensity;
	m_screenShakeTimer.Start();
}

void Game::UpdateScreenShake()
{
	Vec2 randomDir = g_randGen->RollRandomNormalizedVec2() * g_randGen->RollRandomFloatInRange(0.f, m_screenShakeIntensity);
	m_worldCamera.SetCameraPos(m_cameraPosBeforeScreenShake + randomDir);
}

