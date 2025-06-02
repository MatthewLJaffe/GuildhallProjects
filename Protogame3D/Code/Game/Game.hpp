#pragma once
#include "Game/GameCommon.hpp"


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
	bool IsAttractScreen();


private:
	void StartGame();
	void RenderPlaceHolderAttractScreen() const;
	void RenderPlaceholderGameScreen() const;
	void UpdatePlaceHolderAttractScreen();
	void UpdatePlaceHolderGameScreen( float deltaSeconds );
	void AddColoredCube(Vec3 pos, float sideLength);
	void AddAABB3D(AABB3 bounds, Rgba8 color);
	void AddSphere(Vec3 pos, float radius, Rgba8 color);
	void AddGrid();
	void AddCylinder(Vec3 const& startPoint, Vec3 const& endPoint, Rgba8 color);
	void CheckForDebugCommands();
	App* m_theApp;
	bool m_attractScreen = true;
	Vertex_PCU m_attractPlayVerts[3];
	Vertex_PCU m_placeholderGameverts[6];
	float m_attractAlpha = 0.f;
	float m_playRadius = 0.f;
	float m_currT = 0.0f;
	std::vector<Entity*> m_allEntities;
	Texture* m_testTexture = nullptr;
	Player* m_player = nullptr;
	Camera m_screenCamera;
	BitmapFont* font;

	//hack test for axis angle rotation
	Vec3 m_arrowPosition = Vec3(10.f, 0.f, 5.f);
	Mat44 m_testMat44;
	Vec3 m_randomRotationAxis;
};

