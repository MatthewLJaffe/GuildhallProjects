#include "Game/Player.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Game/Game.hpp"

Player::Player(Game* game, Vec3 const& startPos)
	: Entity(game, startPos)
{
}

void Player::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	float systemDelta = Clock::GetSystemClock().GetDeltaSeconds();
	if (g_theInput->GetController(0).IsConnected())
	{
		HandleControlsController(deltaSeconds);
	}
	else
	{
		HandleControlsKeyboard(systemDelta);
	}
}

void Player::Render() const
{
}

void Player::HandleControlsKeyboard(float systemDelta)
{
	IntVec2 cursorDelta = g_theInput->GetCursorClientDelta();
	IntVec2 clientSize = g_theWindow->GetClientDimensions();
	Vec2 normalizedDelta = Vec2((float)cursorDelta.x / (float)clientSize.x, (float)cursorDelta.y / (float)clientSize.y);
	//m_orientationDegrees.m_yaw -= mouseLookSensitivity * normalizedDelta.x;
	//m_orientationDegrees.m_pitch -= mouseLookSensitivity * normalizedDelta.y;
	
	//translation
	Vec3 forward;
	Vec3 left;
	Vec3 up;
	m_orientationDegrees.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);


	if (g_theInput->IsKeyDown('W'))
	{
		m_position += Vec3(0.f, 1.f, 0.f) * systemDelta * m_horizontalMoveSpeed;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		m_position -= Vec3(0.f, 1.f, 0.f) * systemDelta * m_horizontalMoveSpeed;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		m_position -= Vec3(1.f, 0.f, 0.f) * systemDelta * m_horizontalMoveSpeed;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		m_position += Vec3(1.f, 0.f, 0.f) * systemDelta * m_horizontalMoveSpeed;
	}
	if (g_theInput->IsKeyDown('Q'))
	{
		m_position += Vec3(0.f, 0.f, 1.f) * systemDelta * m_verticalMoveSpeed;
	}
	if (g_theInput->IsKeyDown('E'))
	{
		m_position -= Vec3(0.f, 0.f, 1.f) * systemDelta * m_verticalMoveSpeed;
	}
	m_position.x = Clamp(m_position.x, g_theGame->m_currentMapDef.m_worldBoundsMin.x, g_theGame->m_currentMapDef.m_worldBoundsMax.x);
	m_position.y = Clamp(m_position.y, g_theGame->m_currentMapDef.m_worldBoundsMin.y -10.f, g_theGame->m_currentMapDef.m_worldBoundsMax.y);
	m_position.z = Clamp(m_position.z, m_minCameraPos.z, m_maxCameraPos.z);

	m_playerCamera.m_position = m_position;
	m_playerCamera.m_orientation = m_orientationDegrees;
}

void Player::HandleControlsController(float deltaSeconds)
{
	//rotation
	XboxController const controller =  g_theInput->GetController(0);
	Vec2 leftjoysticPos = controller.GetLeftStick().GetPosition();
	m_orientationDegrees.m_yaw += leftjoysticPos.x * deltaSeconds * controllerLookSensitivity;
	m_orientationDegrees.m_pitch += leftjoysticPos.y * deltaSeconds * controllerLookSensitivity;
	float leftTriggerValue = controller.GetLeftTrigger();
	float rightTriggerValue = controller.GetRightTrigger();
	m_orientationDegrees.m_roll += leftTriggerValue * controllerLookSensitivity * deltaSeconds;
	m_orientationDegrees.m_roll -= rightTriggerValue * controllerLookSensitivity * deltaSeconds;

	//translation
	Vec3 forward;
	Vec3 left;
	Vec3 up;
	m_orientationDegrees.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
	Vec2 rightJoystickPos = controller.GetRightStick().GetPosition();

	m_position += rightJoystickPos.y * forward * deltaSeconds * m_horizontalMoveSpeed;
	m_position += rightJoystickPos.x * left * deltaSeconds * m_horizontalMoveSpeed;
	if (controller.IsButtonDown(XboxController::LB_BUTTON))
	{
		m_position -= Vec3(0.f, 0.f, 1.f) * deltaSeconds * m_verticalMoveSpeed;
	}
	if (controller.IsButtonDown(XboxController::RB_BUTTON))
	{
		m_position += Vec3(0.f, 0.f, 1.f) * deltaSeconds * m_verticalMoveSpeed;
	}
	m_position.x = Clamp(m_position.x, m_minCameraPos.x, m_maxCameraPos.x);
	m_position.y = Clamp(m_position.y, m_minCameraPos.y, m_maxCameraPos.y);
	m_position.z = Clamp(m_position.z, m_minCameraPos.z, m_maxCameraPos.z);

	if (controller.WasButtonJustPressed(XboxController::START_BUTTON))
	{
		m_position = Vec3::ZERO;
		m_orientationDegrees.m_pitch = 0.f;
		m_orientationDegrees.m_roll = 0.f;
		m_orientationDegrees.m_yaw = 0.f;
	}


}
