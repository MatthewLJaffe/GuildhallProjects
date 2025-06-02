#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"

class Game;
class Clock;

class App
{
public: 
	App();
	~App();

	void Run();
	void StartUp();
	void Shutdown();
	void HandleSpecialCommands();
	void ResetGame();
	bool m_isSlowMo = false;
	bool m_isFastSpeed = false;
	bool m_noClip = false;
	bool m_noDamage = false;
	bool m_debugCamera = false;
	bool m_pendingGameReset = false;

private:
	void LoadGameConfigBlackboard();
	void RunFrame();
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();
	static bool QuitGame(EventArgs& args);
	Clock* m_clock = nullptr;

private:
	bool m_isQuitting = false;
	double m_timeLastFrame;
	float m_deltaTime;
};