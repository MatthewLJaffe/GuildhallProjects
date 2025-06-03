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
#include "ThirdParty/imgui/imgui.h"
#include "Engine/ParticleSystem/ParticleEmitter.hpp"
#include "Engine/ParticleSystem/Particle.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"
#include "Engine/ParticleSystem/IndividualParticleEffect.hpp"
#include "Game/Model.hpp"
#include "Game/EditorScene.hpp"
#include "Game/GameScene.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/StartScene.hpp"
#include "Game/EndScene.hpp"

Game::Game()
{}

Game::~Game() {}

void Game::StartUp()
{
	font = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Fonts/SquirrelFixedFont");
	m_screenCamera.SetOrthographicView(Vec2::ZERO, g_theWindow->GetClientDimensions().GetVec2());
	m_screenCamera.m_mode = Camera::eMode_Orthographic;

	StartScene* startScene = new StartScene();
	m_scenes.push_back(startScene);

	GameScene* gameScene = new GameScene();
	m_gameScene = gameScene;
	m_scenes.push_back(gameScene);


	EditorScene* editorScene = new EditorScene();
	m_scenes.push_back(editorScene);

	EndScene* endScene = new EndScene();
	m_scenes.push_back(endScene);
	
	if (m_currentGameState == GameState::EDITOR)
	{
		g_theParticleSystem->ToggleParticleEditor(true);
	}
	else
	{
		g_theParticleSystem->ToggleParticleEditor(false);
	}
	m_scenes[(int)m_currentGameState]->StartUp();
}



void Game::CheckForSwitchGameState()
{
	if (m_desiredGameState == m_currentGameState)
	{
		return;
	}
	if (m_currentGameState == GameState::EDITOR || m_currentGameState == GameState::GAME)
	{
		g_theParticleSystem->KillAllEmitters();
	}
	m_currentGameState = m_desiredGameState;
	m_scenes[(int)m_currentGameState]->StartUp();
	Camera* playerCamera = GetPlayerCamera();
	g_theParticleSystem->m_config.m_playerCamera = playerCamera;
}

GameState Game::GetGameState()
{
	return m_currentGameState;
}

Scene* Game::GetGameScene(GameState gameState)
{
	return m_scenes[(int)gameState];
}

void Game::SetDesiredGameState(GameState desiredGameState)
{
	m_desiredGameState = desiredGameState;
}

void Game::CheckForDebugCommands()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_controllingPlayer = !m_controllingPlayer;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_TAB))
	{
		if (m_currentGameState == GameState::EDITOR)
		{
			SetDesiredGameState(GameState::GAME);
		}
		else
		{
			SetDesiredGameState(GameState::EDITOR);
		}
		g_theParticleSystem->ToggleParticleEditor();
	}
}


void Game::Update(float deltaSeconds)
{
	DebugAddMessage("Making a change!", 10.f, 0.f, Rgba8::WHITE);
	CheckForSwitchGameState();
	m_scenes[(int)m_currentGameState]->Update(deltaSeconds);
	if (m_currentGameState == GameState::EDITOR || m_currentGameState == GameState::GAME)
	{
		g_theParticleSystem->Update(deltaSeconds);
	}
	UpdateGameSounds();
	CheckForDebugCommands();
}

void Game::UpdateGameSounds()
{
	for (int i = 0; i < m_gameSounds.size(); i++)
	{
		if (m_gameSounds[i].m_cooldownTimer.HasPeriodElapsed())
		{
			m_gameSounds[i].m_soundID = MISSING_SOUND_ID;
		}
	}
}


void Game::Render() 
{
	m_scenes[(int)m_currentGameState]->Render();
	Camera* playerCam = g_theGame->GetPlayerCamera();
	if (playerCam != nullptr)
	{
		DebugRenderWorld(*playerCam);
	}
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	DebugRenderScreen(g_theGame->m_screenCamera);
}

void Game::EndFrame()
{
}

void Game::ShutDown()
{
	for (int i = 0; i < (int)m_scenes.size(); i++)
	{
		m_scenes[i]->ShutDown();
		delete m_scenes[i];
		m_scenes[i] = nullptr;
	}
}

void Game::PlayGameSound(GameSound& gameSound)
{
	int vacantEntryIdx = -1;
	for (int i = 0; i < (int)m_gameSounds.size(); i++)
	{
		if (m_gameSounds[i].m_soundID == gameSound.m_soundID && !m_gameSounds[i].m_cooldownTimer.HasPeriodElapsed())
		{
			return;
		}
		if (m_gameSounds[i].m_soundID == MISSING_SOUND_ID)
		{
			vacantEntryIdx = i;
		}
	}

	g_theAudio->StartSound(gameSound.m_soundID, false, gameSound.m_volume);
	if (vacantEntryIdx != -1)
	{
		m_gameSounds[vacantEntryIdx] = gameSound;
		m_gameSounds[vacantEntryIdx].m_cooldownTimer.Start();
	}
	else
	{
		m_gameSounds.push_back(gameSound);
		m_gameSounds[m_gameSounds.size() - 1].m_cooldownTimer.Start();
	}
}

Camera* Game::GetPlayerCamera()
{
	if (m_currentGameState == GameState::EDITOR)
	{
		return &m_editorPlayer->m_playerCamera;
	}
	else if (m_currentGameState == GameState::GAME)
	{
		return &m_playerShip->m_camera;
	}
	return nullptr;
}
