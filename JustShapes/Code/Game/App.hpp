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
	void LoadGameConfigBlackboard();

	void HandleSpecialCommands();
	void ResetGame();
	Clock* GetGameClock();
	void Pause();
	void SetPause(bool pause);
	bool m_debugMode = false;
	bool m_isPaused = false;

private:
	void LoadAssets();
	void RunFrame();
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();
	static bool QuitGame(EventArgs& args);

private:
	Clock* m_clock = nullptr;
	bool m_isQuitting = false;
	bool m_isSlowMo = false;
	bool m_runOnce = false;
};