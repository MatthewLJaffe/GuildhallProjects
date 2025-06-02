#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameState.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Timer.hpp"


class Entity;
class GameState;
class App;

enum class SoundType
{
	MUSIC,
	SFX,
	COUNT
};

class Game
{
public:
	Game();
	~Game();
	void StartUp();
	void Update(float deltaSeconds);
	void Render() const;
	void ShutDown();
	void SwitchGameState(GameStateType newGameState);
	void PlaySound(SoundID soundID, SoundType soundType, bool isLooped = false, float volume = 1.f, float balance = .5f, float speed = 1.f, float timeOffset = 0.f);
	void UpdateMusicVolume();
	void ResetCurrentState();
	void RestartCurrentState();
	void AdvanceLevel();
	void PauseMusic();
	void ResumeMusic();
	Vec2 GetRandomPosInWorldScreen(Vec2 const& normallizedPadding);
	Vec2 GetRandomPosOffScreen(float distanceOffScreen);
	void SetWorldCameraPos(Vec2 newPos);
	void SetMusicSpeed(float musicSpeed);
	void SetMusicVolume(float volume0To1);
	void ShakeScreen(float time, float intensity);
	void UpdateScreenShake();
	float m_screenShakeIntensity = 0.f;
	Vec2 m_cameraPosBeforeScreenShake;
	std::vector<GameState*> m_gameStates;
	GameState* m_currentGameState = nullptr;
	GameState* m_nextGamestate = nullptr;
	Timer m_screenShakeTimer;
	Camera m_worldCamera;
	Camera m_screenCamera;

private:
	void StartGame();
	SoundPlaybackID m_currentMusic = MISSING_SOUND_ID;
	float m_baseMusicVolume = 1.f;
};

