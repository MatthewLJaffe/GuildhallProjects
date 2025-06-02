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
	bool m_debugMode = false;
	Clock* m_clock = nullptr;

private:
	void RunFrame();
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();
	static bool QuitGame(EventArgs& args);
	void PrintControlsToConsole();
	void HandleMouseMode();

private:
	bool m_isQuitting = false;
	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_runOnce = false;
	Game* m_theGame = nullptr;
	double m_timeLastFrame = 0.0;
	float m_deltaTime = 0.0f;
};