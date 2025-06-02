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
	Clock* GetGameClock();
	void SetArtificialLag(unsigned long lag);
	bool m_debugMode = false;

private:
	void RunFrame();
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render();
	void EndFrame();
	static bool QuitGame(EventArgs& args);
	void PrintControlsToConsole();
	void HandleMouseMode();
	void LoadGameConfigBlackboard();

private:
	Clock* m_clock = nullptr;
	bool m_isQuitting = false;
	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_runOnce = false;
	double m_timeLastFrame = 0.0;
	float m_deltaTime = 0.0f;
	unsigned long m_artificialLag = 0;
};