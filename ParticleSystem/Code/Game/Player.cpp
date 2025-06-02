#include "Game/Player.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Window.hpp"

Player::Player(Game* game, Vec3 const& startPos)
	: Entity(game, startPos)
{
}

void Player::Update(float deltaSeconds)
{
	if (!g_theGame->m_controllingPlayer)
	{
		return;
	}
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
	//rotation
	if (g_theInput->IsKeyDown(KEYCODE_UP_ARROW))
	{
		m_orientationDegrees.m_pitch -= systemDelta * m_turnRateDegrees;
	}
	if (g_theInput->IsKeyDown(KEYCODE_DOWN_ARROW))
	{
		m_orientationDegrees.m_pitch += systemDelta * m_turnRateDegrees;
	}
	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_ARROW))
	{
		m_orientationDegrees.m_yaw -= systemDelta * m_turnRateDegrees;
	}
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_ARROW))
	{
		m_orientationDegrees.m_yaw += systemDelta * m_turnRateDegrees;
	}
	else
	{
		IntVec2 cursorDelta = g_theInput->GetCursorClientDelta();
		IntVec2 clientSize = g_theWindow->GetClientDimensions();
		Vec2 normalizedDelta = Vec2((float)cursorDelta.x / (float)clientSize.x, (float)cursorDelta.y / (float)clientSize.y);
		m_orientationDegrees.m_yaw -= mouseLookSensitivity * normalizedDelta.x;
		m_orientationDegrees.m_pitch -= mouseLookSensitivity * normalizedDelta.y;
	}

	//translation
	Vec3 forward;
	Vec3 left;
	Vec3 up;
	m_orientationDegrees.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

	if (g_theInput->IsKeyDown(KEYCODE_L_SHIFT))
	{
		m_currMoveSpeed = m_sprintMoveSpeed;
	}
	else
	{
		m_currMoveSpeed = m_defaultMoveSpeed;
	}
	if (g_theInput->IsKeyDown('W'))
	{
		m_position += forward * systemDelta * m_currMoveSpeed;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		m_position -= forward * systemDelta * m_currMoveSpeed;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		m_position += left * systemDelta * m_currMoveSpeed;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		m_position -= left * systemDelta * m_currMoveSpeed;
	}
	if (g_theInput->IsKeyDown('Q'))
	{
		m_position += Vec3(0.f, 0.f, 1.f) * systemDelta * m_currMoveSpeed;
	}
	if (g_theInput->IsKeyDown('E'))
	{
		m_position -= Vec3(0.f, 0.f, 1.f) * systemDelta * m_currMoveSpeed;
	}

	if (m_position.z < 2.f)
	{
		m_position.z = 2.f;
	}
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

	m_position += rightJoystickPos.y * forward * deltaSeconds * m_currMoveSpeed;
	m_position += rightJoystickPos.x * left * deltaSeconds * m_currMoveSpeed;
	if (controller.IsButtonDown(XboxController::A_BUTTON))
	{
		m_currMoveSpeed = m_sprintMoveSpeed;
	}
	else
	{
		m_currMoveSpeed = m_defaultMoveSpeed;
	}
	if (controller.IsButtonDown(XboxController::LB_BUTTON))
	{
		m_position -= Vec3(0.f, 0.f, 1.f) * deltaSeconds * m_currMoveSpeed;
	}
	if (controller.IsButtonDown(XboxController::RB_BUTTON))
	{
		m_position += Vec3(0.f, 0.f, 1.f) * deltaSeconds * m_currMoveSpeed;
	}
	if (controller.WasButtonJustPressed(XboxController::START_BUTTON))
	{
		m_position = Vec3::ZERO;
		m_orientationDegrees.m_pitch = 0.f;
		m_orientationDegrees.m_roll = 0.f;
		m_orientationDegrees.m_yaw = 0.f;
	}


}
