#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Game/Model.hpp"

class Entity;
class Player;
class Texture;
class BitmapFont;
class Prop;


class Scene
{
public:
	std::vector<Entity*> m_entities;
	~Scene();
};

class Game
{
public:
	Game(App* app);
	~Game();
	void StartUp();
	void Update(float deltaSeconds);
	void Render() const;
	void ShutDown();
	bool m_debugRotation = false;

private:
	void StartGame();
	void AddDebugText();
	void CheckForDebugCommands(float deltaSeconds);
	App* m_theApp;
	Prop* m_grid;
	float m_attractAlpha = 0.f;
	float m_playRadius = 0.f;
	float m_currT = 0.0f;
	std::vector<Scene*> m_scenes;
	int currentSceneIndex = 0;
	Texture* m_testTexture = nullptr;
	Player* m_player = nullptr;
	Camera m_screenCamera;
	BitmapFont* font;
	EulerAngles m_sunOrientaiton = EulerAngles(120.f, 45.f, 0.f);
	Model* m_model = nullptr;

	Model* m_woofer1 = nullptr;
	Model* m_woofer2 = nullptr;
	Model* m_woofer3 = nullptr;

	bool m_showModel = false;
	float m_sunOrientaitonRate = 45.f;
	float m_sunIntensity = .5f;
	bool m_useAmbient = true;
	bool m_useDiffuse = true;
	bool m_useSpecular = true;
	bool m_useEmissive = true;
	bool m_diffuseMap = true;
	bool m_normalMap = true;
	bool m_specularMap = true;
	bool m_glossinessMap = true;
	bool m_emissiveMap = true;
	bool m_renderDebug = false;
	int m_sceneIdx = 0;
};

