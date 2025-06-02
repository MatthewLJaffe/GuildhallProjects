#pragma once
class Entity;

#include "Game/GameCommon.hpp"
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
	App* m_theApp;
	bool m_attractScreen = true;
	Vertex_PCU m_attractPlayVerts[3];
	Vertex_PCU m_placeholderGameverts[6];
	float m_attractAlpha = 0.f;
	float m_playRadius = 0.f;
	float m_currT = 0.0f;

	Camera m_worldCamera;
	Camera m_screenCamera;
};

