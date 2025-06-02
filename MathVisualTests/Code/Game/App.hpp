#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"

class Game;
class Clock;

class TestEventRecipient : public EventRecipient
{
public:
	TestEventRecipient(int id);
	int m_id = 0;
	bool LogEventRecipient(EventArgs& args);
};

class App
{
public: 
	App();
	~App();

	void Run();
	void StartUp();
	void Shutdown();

	void HandleSpecialCommands();
	void ResetCurrentTest();
	void AdvanceVisualTest();
	void DecrementVisualTest();
	Clock* GetGameClock();
	Game* GetGame();
	bool HandleQuitRequested(EventArgs& args);
	bool LogHello(EventArgs& args);


private:
	void RunFrame();
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();

public:
	bool m_debugMode = false;
private:
	bool m_isQuitting = false;
	Game* m_theGame = nullptr;
	Clock* m_gameClock = nullptr;

	TestEventRecipient* er1 = nullptr;
	TestEventRecipient* er2 = nullptr;
};