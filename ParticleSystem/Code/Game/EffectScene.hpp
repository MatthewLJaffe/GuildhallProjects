#pragma once
#include "Game/Scene.hpp"

class EffectScene : public Scene
{
	virtual void Update(float deltaSeconds);
	virtual void StartUp() override;
	virtual void SwitchOn() override;
	virtual void SwitchOff() override;

	Timer m_laserBeamTimer;
	Timer m_earthShatterTimer;
	
};