#pragma once
#include "Game/GameCommon.hpp"

class Entity;

class Scene
{
public:
	Scene();
	virtual ~Scene() = default;
	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;
	virtual void StartUp() = 0;
	virtual void ShutDown() = 0;
	bool m_startedUp = false;
};