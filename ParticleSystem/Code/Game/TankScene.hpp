#pragma once
#include "Game/Scene.hpp"

class TankScene : public Scene
{
	virtual void StartUp() override;
	virtual void SwitchOn() override;
	virtual void SwitchOff() override;
	virtual void Update(float deltaSeconds) override;

	std::vector<Entity*> m_blueTanks;
	std::vector<Entity*> m_redTanks;
	std::vector<Entity*> m_projectiles;


	Timer m_blueFireTimer;
	Timer m_redFireTimer;
	Timer m_blueRespawnTimer;
	Timer m_redRespawnTimer;
};