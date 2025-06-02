#pragma once
#include "Game/GameCommon.hpp"

class Game;
class App;
class Renderer;

extern App* g_theApp;
extern Renderer* g_theRenderer;

class Entity
{

public:
	Entity(Game* game, const Vec3& startPos);
	virtual ~Entity();
	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;
	Mat44 GetModelMatrix() const;
	Vec3 GetForwardNormal();

public:
	Vec3 m_position;
	Vec3 m_velocity = Vec3(0.f, 0.f, 0.f);
	Vec3 m_scale = Vec3(1.f, 1.f,1.f);
	EulerAngles m_orientationDegrees;
	EulerAngles m_angularVelocity = EulerAngles(45.f, 0.f, 0.f);
	Game* m_game = nullptr;
	Rgba8 m_color = Rgba8::WHITE;
	bool m_renderDebug = false;
protected:
	void CreateDebugTangentAndBasisVectors();
	std::vector<Vertex_PCUTBN> m_vertexes;
	std::vector<unsigned int> m_indexes;
	std::vector<Vertex_PCU> m_debugVertexes;
	VertexBuffer* m_debugVertexBuffer = nullptr;
};
