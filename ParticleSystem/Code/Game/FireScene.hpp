#pragma once
#include "Game/Scene.hpp"

class FireScene : public Scene
{
	virtual void Update(float deltaSeconds) override;
	virtual void StartUp() override;
	virtual void SwitchOn() override;

	ParticleEffect* m_wildFire = nullptr;
	ParticlePhysicsAABB3* m_testWallCollider = nullptr;
	FloatRange m_yFireRange = FloatRange(-40.f, -20.f);
	float m_currentSceneTime = 0.f;
	float m_fireMoveSpeed = 45.f;
};