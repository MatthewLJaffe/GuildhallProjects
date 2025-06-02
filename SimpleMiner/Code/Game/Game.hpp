#pragma once
#include "Engine/Renderer/Camera.hpp"

class App;
class Entity;
class Player;
class Texture;
class BitmapFont;


class Game
{
public:
	Game(App* app);
	~Game();
	void StartUp();
	void Update(float deltaSeconds);
	void Render() const;
	void ShutDown();
	Player* m_player = nullptr;
	Camera m_screenCamera;

private:
	void StartGame();
	void CheckForDebugCommands();
	void LoadAssets();
	void UnloadAssets();
	App* m_theApp;
	std::vector<Entity*> m_allEntities;
	BitmapFont* font;
};

