//mine
#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Controller.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/Camera.hpp"

//------------------------------------------------------------------------------------------------
class Game;
class Actor;
class Texture;

//------------------------------------------------------------------------------------------------
class Player : public Controller
{
public:
	Player(Game* game, int playerIndex, int controllerIndex);
	virtual ~Player();

	virtual void Update(float deltaSeconds);
	void UpdateGradeText();
	void RenderScreen();
	void UpdateCamera();

	void SetNormalizedViewport(const AABB2& viewport);
	AABB2 GetNormalizedViewport() const;
	AABB2 GetViewport() const;
	virtual void OnKilledBy(ActorUID damageDealer) override;
	void ReduceGrade(int pointsOff, std::string message, float lossTime);
	int m_currentGrade = 90;

	Game* m_game;

	Vec3 m_position;
	EulerAngles	m_orientation;

	int m_playerIndex = 0;
	int m_controllerIndex = -1;

	Camera m_worldCamera;
	Camera m_screenCamera;
	AABB2 m_normalizedViewport = AABB2::ZERO_TO_ONE;

	bool m_freeFlyCameraMode = false;

	int m_kills = 0;
	int m_deaths = 0;

	Texture* m_textBoxTexture = nullptr;
	Texture* m_defaultHudTexture = nullptr;

	bool m_showErrorMessage = false;
	Timer m_scoreReduceTimer;
	Timer m_startGradeReductionTimer;
private:
	int m_pointsToReduce = 0;

private:
	void UpdatePossessInput(float deltaSeconds);
	void UpdateFreeFlyInput();
	void HandlePossessController(float deltaSeconds);
	void HandlePossessKeyboard(float deltaSeconds);
	void HandleFreeFlyController();
	void HandleFreeFlyKeyboard();
	float m_defaultMoveSpeed = 1.0f;
	float m_sprintMoveSpeed = m_defaultMoveSpeed * 15.f;
	float m_currentMoveSpeed = m_defaultMoveSpeed;
	float m_turnRateDegrees = 180.f;
	float m_mouseLookSensitivity = 0.075f;
	float m_controllerLookSensitivity = 180.f;
};



