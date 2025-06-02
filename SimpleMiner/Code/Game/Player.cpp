#include "Game/Player.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"

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
	Vec3 infrontOfPlayer = m_position + m_orientationDegrees.GetIFwd() * .2f;
	Mat44 worldBasis;
	worldBasis.SetTranslation3D(infrontOfPlayer);
	worldBasis.AppendScaleUniform3D(.01f);
	//DebugAddWorldBasis(worldBasis, 0.f, DebugRenderMode::ALWAYS);
}

void Player::HandleControlsKeyboard(float systemDelta)
{
	IntVec2 cursorDelta = g_theInput->GetCursorClientDelta();
	IntVec2 clientSize = g_theWindow->GetClientDimensions();
	Vec2 normalizedDelta = Vec2((float)cursorDelta.x / (float)clientSize.x, (float)cursorDelta.y / (float)clientSize.y);
	m_orientationDegrees.m_yaw -= mouseLookSensitivity * normalizedDelta.x;
	m_orientationDegrees.m_pitch -= mouseLookSensitivity * normalizedDelta.y;
	m_orientationDegrees.m_pitch = Clamp(m_orientationDegrees.m_pitch, -89.f, 89.f);

	//translation
	Vec3 forward;
	Vec3 left;
	Vec3 up;
	m_orientationDegrees.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
	forward.z = 0.f;
	forward = forward.GetNormalized();
	left.z = 0.f;
	left = left.GetNormalized();

	if (g_theInput->IsKeyDown(KEYCODE_SPACE))
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
