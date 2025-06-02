#include "Engine/Input/XboxController.hpp"
#include <Windows.h>
#include <Xinput.h>
#include <stdio.h>
#include "Engine/Math/MathUtils.hpp"
#pragma comment( lib, "xinput9_1_0" ) // Xinput 1_4 doesn't work in older Windows versions; use XInput 9_1_0 explicitly for best compatibility

void XboxController::Update()
{
	XINPUT_STATE xboxControllerState = {};
	DWORD errorStatus = XInputGetState(m_id, &xboxControllerState);
	if (errorStatus == ERROR_SUCCESS)
	{
		//buttons
		m_isConnected = true;
		unsigned short wButtons = xboxControllerState.Gamepad.wButtons;
		UpdateButtonState(wButtons, A_BUTTON, XINPUT_GAMEPAD_A);
		UpdateButtonState(wButtons, B_BUTTON, XINPUT_GAMEPAD_B);
		UpdateButtonState(wButtons, X_BUTTON, XINPUT_GAMEPAD_X);
		UpdateButtonState(wButtons, Y_BUTTON, XINPUT_GAMEPAD_Y);
		UpdateButtonState(wButtons, DPAD_DOWN_BUTTON, XINPUT_GAMEPAD_DPAD_DOWN);
		UpdateButtonState(wButtons, DPAD_LEFT_BUTTON, XINPUT_GAMEPAD_DPAD_LEFT);
		UpdateButtonState(wButtons, DPAD_RIGHT_BUTTON, XINPUT_GAMEPAD_DPAD_RIGHT);
		UpdateButtonState(wButtons, DPAD_UP_BUTTON, XINPUT_GAMEPAD_DPAD_UP);
		UpdateButtonState(wButtons, RB_BUTTON, XINPUT_GAMEPAD_RIGHT_SHOULDER);
		UpdateButtonState(wButtons, LB_BUTTON, XINPUT_GAMEPAD_LEFT_SHOULDER);
		UpdateButtonState(wButtons, L3_BUTTON, XINPUT_GAMEPAD_LEFT_THUMB);
		UpdateButtonState(wButtons, R3_BUTTON, XINPUT_GAMEPAD_RIGHT_THUMB);
		UpdateButtonState(wButtons, BACK_BUTTON, XINPUT_GAMEPAD_BACK);
		UpdateButtonState(wButtons, START_BUTTON, XINPUT_GAMEPAD_START);

		//triggers
		UpdateTrigger(m_leftTriggerNormalized, xboxControllerState.Gamepad.bLeftTrigger);
		UpdateTrigger(m_rightTriggerNormalized, xboxControllerState.Gamepad.bRightTrigger);

		//sticks
		UpdateJoystick(m_analogJoystickLeft, xboxControllerState.Gamepad.sThumbLX, xboxControllerState.Gamepad.sThumbLY);
		UpdateJoystick(m_analogJoystickRight, xboxControllerState.Gamepad.sThumbRX, xboxControllerState.Gamepad.sThumbRY);
	}
	else if (errorStatus == ERROR_DEVICE_NOT_CONNECTED)
	{
		m_isConnected = false;
		Reset();
	}
	else
	{
		printf("Xbox controller #%i reports error code %u (0x%08x).                  \n", m_id, errorStatus, errorStatus);
	}
}

void XboxController::Reset()
{
    for (int i = 0; i < NUM_XBOX_BUTTONS; i++)
    {
        m_buttonStates[i].m_isPressed = false;
		m_buttonStates[i].m_wasPressedLastFrame = false;
    }
    m_leftTriggerNormalized = 0.f;
    m_rightTriggerNormalized = 0.f;
    m_analogJoystickLeft.Reset();
	m_analogJoystickRight.Reset();
}

void XboxController::UpdateJoystick(AnalogJoystick& out_joystick, short rawX, short rawY)
{
	float rawNormilizedStickX = RangeMap(static_cast<float>(rawX), -32768.f, 32767.f, -1.f, 1.f);
	float rawNormilizedStickY = RangeMap(static_cast<float>(rawY), -32768.f, 32767.f, -1.f, 1.f);
	out_joystick.UpdatePosition(rawNormilizedStickX, rawNormilizedStickY);
}

void XboxController::UpdateTrigger(float& out_triggerValue, unsigned char rawValue)
{
	float triggerRaw = static_cast<float>(rawValue);
	out_triggerValue = RangeMapClamped(triggerRaw, 0.f, 255.f, 0.f, 1.f);
}

void XboxController::UpdateButtonState(unsigned short wButtons, XboxButtonID buttonID, unsigned short buttonMask)
{
	KeyButtonState& button = m_buttonStates[buttonID];
	button.m_wasPressedLastFrame = button.m_isPressed;
	bool buttonPressed = (wButtons & buttonMask) == buttonMask;
	button.m_isPressed = buttonPressed;
}

XboxController::XboxController()
{
	m_analogJoystickLeft.SetDeadZoneThresholds(.3f, .95f);
	m_analogJoystickRight.SetDeadZoneThresholds(.3f, .95f);
}

bool XboxController::IsConnected() const
{
    return m_isConnected;
}

int XboxController::GetControllerID() const
{
    return m_id;
}

AnalogJoystick const& XboxController::GetLeftStick() const
{
    return m_analogJoystickLeft;
}

AnalogJoystick const& XboxController::GetRightStick() const
{
    return m_analogJoystickRight;
}

float XboxController::GetLeftTrigger() const
{
    return m_leftTriggerNormalized;
}

float XboxController::GetRightTrigger() const
{
    return m_rightTriggerNormalized;
}

KeyButtonState const& XboxController::GetButton(XboxButtonID buttonID) const
{
    return m_buttonStates[buttonID];
}

bool XboxController::WasButtonJustPressed(XboxButtonID buttonID) const
{
	return m_buttonStates[buttonID].m_isPressed && !m_buttonStates[buttonID].m_wasPressedLastFrame;
}

bool XboxController::WasButtonJustReleased(XboxButtonID buttonID) const
{
	return !m_buttonStates[buttonID].m_isPressed && m_buttonStates[buttonID].m_wasPressedLastFrame;
}

bool XboxController::IsButtonDown(XboxButtonID buttonID) const
{
	return m_buttonStates[buttonID].m_isPressed;
}

