#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"
#include "Game/Scene.hpp"

struct FlyingFireEntry
{
	EulerAngles m_orientaiton;
	float m_speed = 10.f;
	ParticleEffect* m_effectInstance = nullptr;
};

class PerformanceScene : public Scene
{
public:
	virtual void Update(float deltaSeconds) override;
	virtual void StartUp() override;
	virtual void SwitchOn() override;
	virtual void SwitchOff() override;

	float m_firesThisFrame = 0.f;
	std::vector<FlyingFireEntry> m_flyingFires;
};


