#pragma once
class Entity;
class VisualTest;

class Texture;
class SpriteSheet;

#include "Game/GameCommon.hpp"
#include "Game/VisualTest.hpp"

class Game
{
public:
	Game(App* app);
	~Game();
	void StartUp();
	void Update(float deltaSeconds);
	void Render() const;
	void ShutDown();
	void LoadAssets();
	VisualTest* GetCurrentVisualTest();
	
	VisualTestType m_visualTestType;
	std::vector<VisualTest*> m_visualTests;
	Camera m_screenCamera;

private:
	void StartGame();
	App* m_theApp;
	
};

