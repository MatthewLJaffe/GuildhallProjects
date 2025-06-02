#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Game/Game.hpp"


class Player 
{
public:
	Player(Game* game, Vec3 const& startPos);
	void Update(float deltaSeconds);
	void Render() const;
	void HandleControlsKeyboard(float deltaSeconds);
	void HandleControlsController(float deltaSeconds);
	Game* m_game = nullptr;
	Camera m_playerCamera;
	float m_currMoveSpeed = 2.0f;
	EulerAngles m_orientationDegrees;
	Vec3 m_position;
	float m_defaultMoveSpeed = 2.0f;
	float m_sprintMoveSpeed = m_defaultMoveSpeed * 15.f;
	float m_turnRateDegrees = 90.f;
	float mouseLookSensitivity = 0.075f;
	float controllerLookSensitivity = 30.f;
};