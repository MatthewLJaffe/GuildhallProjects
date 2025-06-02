#pragma once
#include "Game/GameCommon.hpp"

class Scene
{
public:
	virtual void StartUp();
	virtual void SwitchOn();
	virtual void SwitchOff();
	virtual void Update(float deltaSeconds);
	virtual void Render() const;
	virtual ~Scene();
	std::vector<Entity*> m_sceneEntities;
	std::vector<ParticleEffect*> m_sceneParticleEffects;
};