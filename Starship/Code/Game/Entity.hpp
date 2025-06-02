#pragma once
#include "Game/GameCommon.hpp"

class Game;
class App;
class Renderer;
class Missile;

extern App* g_theApp;
extern Renderer* g_theRenderer;

class Entity
{

public:
	Entity(Game* game, const Vec2& startPos);
	virtual ~Entity();
	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const;
	virtual void Die();
	bool IsOffScreen() const;
	Vec2 GetForwardNormal() const;
	bool IsAlive() const;
	virtual void RenderDebug() const;

public:
	Vec2 m_position;
	Vec2 m_velocity = Vec2(0, 0);
	float m_orientationDegrees = 0.f;
	float m_angularVelocity = 0.f;
	float m_physicsRadius = 0.f;
	float m_cosmeticRadius = 0.f;
	float m_health = 1.f;
	bool m_isDead = false;
	bool m_isGarbage = false;
	Game* m_game = nullptr;
	int m_typeID = -1;
	bool m_missileLockedOn = false;
	Missile* m_targetingMissile;
protected:
	int m_numVerts = 0;
	Vertex_PCU* m_localVerts = nullptr;
	Vertex_PCU* m_tempWorldVerts = nullptr;
	virtual void InitializeLocalVerts() = 0;
	void WrapAround();
};
