#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"

class Entity;
class Map;
class SpriteSheet;

constexpr int NUM_MAPS = 3;

enum GameState
{
	GAME_STATE_ATTRACT,
	GAME_STATE_PLAYING,
	GAME_STATE_LOSE,
	GAME_STATE_WIN
};

class Game
{
public:
	Game(App* app);
	~Game();
	void StartUp();
	void Update(float deltaSeconds);
	void Render() const;
	void ShutDown();

	void SetNumTilesInViewVertically(float newNumTilesInViewVertically);
	void PauseGame();
	void SwitchMaps();
	bool m_losePending = false;
	Camera m_worldCamera;
	Camera m_debugCamera;
	Camera m_screenCamera;
	int m_currMapIdx = 0;
	std::vector<Map*> m_maps;
	//Map* m_maps[NUM_MAPS]{ nullptr };
	Map* m_currentMap = nullptr;
	GameState m_gameState = GAME_STATE_ATTRACT;
	bool m_isPaused = false;

	SoundPlaybackID m_startMusicPlayback;
	SoundPlaybackID m_gamePlayMusicPlayback;
	SpriteSheet* m_tilesSpriteSheet;
	std::vector<Vertex_PCU> m_debugTextVerts;

private:
	void Update_Attract(float deltaSeconds);
	void Update_Playing(float deltaSeconds);
	void Render_Attract() const;
	void Render_Playing() const;
	void AdjustDebugCameraForCurrentMap();

	float m_numTilesInViewVertically = 8.f;
	void LoadAssets();
	void InitializeTileDefinitions();
	void InitializeMaps();
	void HandleSpecialGameplayInputs();
	void RespawnPlayer();
	void StartGame();
	void DrawPausedEffect() const;
	void DrawLoseScreen() const;
	void DrawWinScreen() const;
	void RenderPlaceHolderAttractScreen() const;
	void UpdatePlaceHolderAttractScreen();
	App* m_theApp;
	Vertex_PCU m_attractPlayVerts[3];
	Vertex_PCU m_placeholderGameverts[6];
	float m_attractAlpha;
	float m_playRadius;
	float m_currLoseTime = 3.f;
	float m_playOutLoseTime = 3.f;
	Texture* m_testTexture; 
	Texture* m_loseScreenTexture;
	Texture* m_winScreenTexture;


};

