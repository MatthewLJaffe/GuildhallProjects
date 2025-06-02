#include "Game/Player.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Window.hpp"

Player::Player(Vec3 const& startPos, EulerAngles const& startOrientaiton)
	: Entity(startPos, startOrientaiton)
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
	HandleControlsKeyboard(systemDelta);
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

	SetLocalOrientation(m_orientationDegrees);
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
		Translate( forward * systemDelta * m_currMoveSpeed);
	}
	if (g_theInput->IsKeyDown('S'))
	{
		Translate(-forward * systemDelta * m_currMoveSpeed);
	}
	if (g_theInput->IsKeyDown('A'))
	{
		Translate(left * systemDelta * m_currMoveSpeed);
	}
	if (g_theInput->IsKeyDown('D'))
	{
		Translate(-left * systemDelta * m_currMoveSpeed);
	}
	if (g_theInput->IsKeyDown('Q'))
	{
		Translate(Vec3(0.f, 0.f, 1.f) * systemDelta * m_currMoveSpeed);
	}
	if (g_theInput->IsKeyDown('E'))
	{
		Translate(-Vec3(0.f, 0.f, 1.f) * systemDelta * m_currMoveSpeed);
	}

	m_playerCamera.SetTransform(GetWorldTransform());
}