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

	bool IsQuitting() const { return m_isQuitting; }
	bool HandleKeyPressed( unsigned char keyCode);
	bool HandleKeyReleased(unsigned char keyCode);
	void HandleQuitRequested();
	void HandleSpecialCommands();
	void ResetGame();
	bool m_debugMode = false;

private:
	void RunFrame();
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();
	void AddControlsToDevConsole();
	static bool QuitGame(EventArgs& args);
	static bool SetClockTimeScale(EventArgs& args);


private:
	bool m_isQuitting = false;
	Game* m_theGame;
	Clock* m_clock;
};