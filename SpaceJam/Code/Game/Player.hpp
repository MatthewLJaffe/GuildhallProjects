#pragma once
#include "Game/Entity.hpp"
#include "Engine/Renderer/Camera.hpp"

class Player : public Entity
{
public:
	Player(Vec3 const& startPos, EulerAngles const& startOrientation = EulerAngles());
	void Update(float deltaSeconds) override;
	void Render() const override;
	virtual void HandleControlsKeyboard(float deltaSeconds);
	Camera m_playerCamera;
	EulerAngles m_orientationDegrees;
	float m_currMoveSpeed = 2.0f;
	float m_defaultMoveSpeed = 2.0f;
	float m_sprintMoveSpeed = m_defaultMoveSpeed * 10.f;
	float m_turnRateDegrees = 90.f;
	float mouseLookSensitivity = 30.f;
	float controllerLookSensitivity = 30.f;
};