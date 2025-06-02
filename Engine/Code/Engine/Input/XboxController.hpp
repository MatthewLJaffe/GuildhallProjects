#pragma once
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Input/AnalogJoystick.hpp"

constexpr int NUM_XBOX_BUTTONS = 14;
class XboxController
{
	friend class InputSystem;
public:
	enum XboxButtonID
	{
		X_BUTTON,
		Y_BUTTON,
		A_BUTTON,
		B_BUTTON,
		L3_BUTTON,
		R3_BUTTON,
		RB_BUTTON,
		LB_BUTTON,
		BACK_BUTTON,
		START_BUTTON,
		DPAD_LEFT_BUTTON,
		DPAD_UP_BUTTON,
		DPAD_RIGHT_BUTTON,
		DPAD_DOWN_BUTTON
	};

	XboxController();
	~XboxController() = default;
	bool IsConnected() const;
	int GetControllerID() const;
	AnalogJoystick const& GetLeftStick() const;
	AnalogJoystick const& GetRightStick() const;
	float GetLeftTrigger() const;
	float GetRightTrigger() const;
	KeyButtonState const& GetButton( XboxButtonID buttonID ) const;
	bool WasButtonJustPressed(XboxButtonID buttonID) const;
	bool WasButtonJustReleased(XboxButtonID buttonID) const;
	bool IsButtonDown(XboxButtonID buttonID) const;

private:
	void Update();
	void Reset();
	void UpdateJoystick( AnalogJoystick& out_joystick, short rawX, short rawY);
	void UpdateTrigger( float& out_triggerValue, unsigned char rawValue );
private:
	void UpdateButtonState(unsigned short wButtons, XboxButtonID buttonID, unsigned short buttonMask);
	int m_id = -1;
	bool m_isConnected = false;
	KeyButtonState m_buttonStates[NUM_XBOX_BUTTONS];
	AnalogJoystick m_analogJoystickLeft;
	AnalogJoystick m_analogJoystickRight;
	float m_leftTriggerNormalized = 0.f;
	float m_rightTriggerNormalized = 0.f;
};