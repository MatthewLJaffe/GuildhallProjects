#pragma once
#include "Game/Entity.hpp"
#include "Engine/Renderer/Camera.hpp"

class Player : public Entity
{
public:
	Player(Game* game, Vec3 const& startPos);
	void Update(float deltaSeconds) override;
	void Render() const override;
	void HandleControlsKeyboard(float deltaSeconds);
	void HandleControlsController(float deltaSeconds);
	Camera m_playerCamera;
	float m_horizontalMoveSpeed = 10.0f;
	float m_verticalMoveSpeed = 10.0f;
	float m_turnRateDegrees = 90.f;
	float mouseLookSensitivity = 30.f;
	float controllerLookSensitivity = 30.f;
	Vec3 m_minCameraPos = Vec3(-15.f, -15.f, -15.f);
	Vec3 m_maxCameraPos = Vec3(15.f, 15.f, 15.f);
};