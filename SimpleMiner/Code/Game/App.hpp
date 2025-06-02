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
	Clock* GetGameClock() const;
	double GetTimeLastFrameMS();
	bool m_debugMode = false;
	bool m_chunkStep = false;
	//Clock* m_clock = nullptr;

private:
	void RunFrame();
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();
	static bool QuitGame(EventArgs& args);
	void PrintControlsToConsole();
	void HandleMouseMode();
	void LoadGameConfigBlackboard();

private:
	bool m_isQuitting = false;
	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_runOnce = false;
	double m_timeLastFrameMS = 0.0;
	float m_deltaTime = 0.0f;
	Clock* m_clock = nullptr;

};