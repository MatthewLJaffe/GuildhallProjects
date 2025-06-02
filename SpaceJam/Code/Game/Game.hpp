#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Timer.hpp"

class ParticleEmitter;
class Entity;
class Player;
class Texture;
class BitmapFont;
class App;
class CPUMesh;
class GPUMesh;
class ParticleEffect;
class IndividualParticleEffect;
class Scene;
class PlayerShip;
class GameScene;

enum class GameState
{
	START_MENU,
	GAME,
	EDITOR,
	END_MENU,
	NUM_GAME_STATES
};

struct GameSound
{
	float m_volume = 1.f;
	SoundID m_soundID = MISSING_SOUND_ID;
	Timer m_cooldownTimer = Timer(.1f);
};

class Game
{
public:
	Game();
	~Game();
	void StartUp();
	void Update(float deltaSeconds);
	void UpdateGameSounds();
	void Render();
	void EndFrame();
	void ShutDown();
	void PlayGameSound(GameSound& gameSound);
	Camera* GetPlayerCamera();
	Player* m_editorPlayer = nullptr;
	PlayerShip* m_playerShip = nullptr;
	bool m_controllingPlayer = true;
	EulerAngles m_sunOrientaiton = EulerAngles(120.f, 45.f, 0.f);
	Camera m_screenCamera;
	GameScene* m_gameScene = nullptr;
	BitmapFont* font;
	GameState GetGameState();
	Scene* GetGameScene(GameState gameState);
	void SetDesiredGameState(GameState desiredGameState);
private:
	void CheckForSwitchGameState();
	void CheckForDebugCommands();
	std::vector<Scene*> m_scenes;
	std::vector<GameSound> m_gameSounds;
	GameState m_currentGameState = GameState::START_MENU;
	GameState m_desiredGameState = GameState::START_MENU;

};

