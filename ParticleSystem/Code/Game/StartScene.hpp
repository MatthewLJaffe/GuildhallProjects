#pragma once
#include "Game/Scene.hpp"

class StartScene : public Scene
{
	virtual void Update(float deltaSeconds);
	virtual void StartUp() override;
	virtual void SwitchOn() override;
	virtual void SwitchOff() override;
};