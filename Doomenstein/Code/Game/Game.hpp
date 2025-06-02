#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Tasks.hpp"

class Entity;
class Player;
class Texture;
class BitmapFont;
class Map;

enum class GameMode
{
	ATTRACT,
	LOBBY,
	PLAYING,
	FOOM,
};

class Game
{
public:
	Game(App* app);
	~Game();
	void StartUp();
	void Update(float deltaSeconds);
	void Render();
	void ShutDown();
	GameMode m_currentGameMode = GameMode::ATTRACT;
	std::vector<Map*> m_maps;
	std::vector<Player*> m_players;
	Map* m_currentMap = nullptr;
	int m_currentMapIdx = 0;
	bool m_freeFlyMode = false;
	Camera m_entireScreenCamera;
	Player* GetCurrentlyRenderingPlayer();
	Player* GetCurrentlyUpdatingPlayer();
	Player* GetKeyboardPlayer();
	Player* GetControllerPlayer(int controllerIdx);
	Actor* GetFirsPlayerActor();
	void CompleteCurrentTask();
	void AdvanceMap();
	Task* GetTaskByName(std::string name);

	SoundPlaybackID m_menuMusicPlayback;
	SoundPlaybackID m_gameMusicPlayback;

private:
	bool ReadyToStartGame();
	bool IsPlayerJoining();
	bool IsPlayerLeaving();
	void StartGame();
	void HandleScreenSplitting();

	void UpdateAttract(float deltaSeconds);
	void UpdateLobby(float deltaSeconds);
	void UpdateGame(float deltaSeconds);
	void UpdateFoom(float deltaSeconds);

	void RenderAttract();
	void RenderLobby();
	void RenderGame();
	void RenderFoom();
	void LoadAssets();

	void CheckForDebugCommands();
	void InitializeTasks();
	void UpdateTasks();


	App* m_theApp;
	float m_attractAlpha = 0.f;
	float m_playRadius = 0.f;
	float m_currT = 0.0f;
	Texture* m_testTexture = nullptr;
	Player* m_currentlyRenderingPlayer = nullptr;
	Player* m_currentlyUpdatingPlayer = nullptr;
	std::vector<Task*> m_tasks;
	float m_windowsErrorCooldown = 1.5f;
	float m_currentErrorCooldown = .0f;
};

constexpr int MAX_PLAYER_COUNT = 2;
