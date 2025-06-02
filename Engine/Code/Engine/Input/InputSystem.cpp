#include "Engine/Input/InputSystem.hpp"
#include <Windows.h>
#include <Xinput.h>
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Window.hpp"

const unsigned char KEYCODE_F1 = VK_F1;
const unsigned char KEYCODE_F2 = VK_F2;
const unsigned char KEYCODE_F3 = VK_F3;
const unsigned char KEYCODE_F4 = VK_F4;
const unsigned char KEYCODE_F5 = VK_F5;
const unsigned char KEYCODE_F6 = VK_F6;
const unsigned char KEYCODE_F7 = VK_F7;
const unsigned char KEYCODE_F8 = VK_F8;
const unsigned char KEYCODE_F9 = VK_F9;
const unsigned char KEYCODE_F10 = VK_F10;
const unsigned char KEYCODE_F11 = VK_F11;
const unsigned char KEYCODE_ESC = VK_ESCAPE;
const unsigned char KEYCODE_SPACE = VK_SPACE; 
const unsigned char KEYCODE_TAB = VK_TAB;
const unsigned char KEYCODE_LEFT_MOUSE = VK_LBUTTON;
const unsigned char KEYCODE_RIGHT_MOUSE = VK_RBUTTON;

InputSystem::InputSystem(InputConfig& inputConfig)
	: m_config(inputConfig)
{
	for (int i = 0; i < NUM_XBOX_CONTROLLERS; i++)
	{
		m_xboxControllers[i].m_id = i;
	}
}

void InputSystem::BeginFrame()
{
	for (int i = 0; i < NUM_XBOX_CONTROLLERS; i++)
	{
		m_xboxControllers[i].Update();
	}

	UpdateCursorState();
}

void InputSystem::EndFrame()
{
	for (int i = 0; i < 256; i++)
	{
		m_keyStates[i].m_wasPressedLastFrame = m_keyStates[i].m_isPressed;
	}
}

bool InputSystem::HandleKeyPressed(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isPressed = true;
	return true;
}

bool InputSystem::HandleKeyReleased(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isPressed = false;
	return true;
}

void InputSystem::StartUp()
{
	g_theEventSystem->SubscribeEventCallbackFunction("KeyPressed", InputSystem::Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFunction("KeyReleased", InputSystem::Event_KeyReleased);
}

bool InputSystem::IsKeyDown(unsigned char keyCode)
{
	return m_keyStates[keyCode].m_isPressed;
}

void InputSystem::ShutDown()
{

}

bool InputSystem::WasKeyJustPressed(unsigned char keyCode)
{
	return m_keyStates[keyCode].m_isPressed && !m_keyStates[keyCode].m_wasPressedLastFrame;
}

bool InputSystem::WasKeyJustReleased(unsigned char keyCode)
{
	return !m_keyStates[keyCode].m_isPressed && m_keyStates[keyCode].m_wasPressedLastFrame;
}

XboxController const& InputSystem::GetController(int controllerID)
{
	if (controllerID < 0 || controllerID > NUM_XBOX_CONTROLLERS)
	{
		ERROR_AND_DIE("Invalid controllerId")
	}
	return m_xboxControllers[controllerID];
}

void InputSystem::UpdateCursorState()
{
	//Check if our current hidden mode does not match the state that windows currently has the cursor in and if so, tell Windows to show or hide the cursor accordingly
	if (m_cursorState.m_currentHiddenMode != m_cursorState.m_desiredHiddenMode)
	{
		if (m_cursorState.m_desiredHiddenMode)
		{
			//hide cursor
			while (::ShowCursor(false) >= 0) {}
			m_cursorState.m_currentHiddenMode = true;
		}
		else
		{
			//show cursor
			while (::ShowCursor(true) < 0) {}
			m_cursorState.m_currentHiddenMode = false;
		}
	}
	IntVec2 prevCursorPos = m_cursorState.m_cursorClientPosition;
	m_cursorState.m_cursorClientPosition = GetCursorPosition();
	if (m_cursorState.m_relativeMode)
	{
		m_cursorState.m_cursorClientDelta = m_cursorState.m_cursorClientPosition - prevCursorPos;
		IntVec2 clientDimensions = m_config.m_window->GetClientDimensions();
		IntVec2 cursorCenterPos = IntVec2((int)(clientDimensions.x * .5f), (int)(clientDimensions.y * .5f));

		POINT centerPos;
		centerPos.x = cursorCenterPos.x;
		centerPos.y = cursorCenterPos.y;
		::ClientToScreen(HWND(m_config.m_window->GetHwnd()), &centerPos);
		::SetCursorPos(centerPos.x, centerPos.y);
		m_cursorState.m_cursorClientPosition = GetCursorPosition();
	}
	else
	{
		m_cursorState.m_cursorClientDelta = IntVec2(0, 0);
	}
}

void InputSystem::SetCursorMode(bool hiddenMode, bool relativeMode)
{
	m_cursorState.m_desiredHiddenMode = hiddenMode;
	m_cursorState.m_relativeMode = relativeMode;
}

CursorState InputSystem::GetCursorMode()
{
	return m_cursorState;
}

IntVec2 InputSystem::GetCursorClientDelta() const
{
	return m_cursorState.m_cursorClientDelta;
}

Vec2 InputSystem::GetCursorNormalizedPosition() const
{
	HWND windowHandle = HWND(m_config.m_window->GetHwnd());
	POINT cursorCoords;
	RECT clientRect;
	::GetCursorPos(&cursorCoords); // in screen coordinates (0, 0) top-left
	::ScreenToClient(windowHandle, &cursorCoords);
	::GetClientRect(windowHandle, &clientRect);
	float cursorX = float(cursorCoords.x) / float(clientRect.right);  // normalized x pos
	float cursorY = float(cursorCoords.y) / float(clientRect.bottom); // normalized y pos
	return Vec2(cursorX, 1.f - cursorY); //0, 0 in bottom left
}

void InputSystem::ResetCursorClientDelta()
{
	m_cursorState.m_cursorClientDelta = IntVec2(0, 0);
}

IntVec2 InputSystem::GetCursorPosition() const
{
	HWND windowHandle = HWND(m_config.m_window->GetHwnd());
	POINT cursorCoords;
	RECT clientRect;
	::GetCursorPos(&cursorCoords); // in screen coordinates (0, 0) top-left
	::ScreenToClient(windowHandle, &cursorCoords);
	::GetClientRect(windowHandle, &clientRect);
	IntVec2 clientDimensions = m_config.m_window->GetClientDimensions();
	int cursorX =  cursorCoords.x;
	int cursorY =  clientRect.bottom - cursorCoords.y;
	return IntVec2(cursorX, cursorY); //0, 0 in bottom left
}

bool InputSystem::Event_KeyPressed(EventArgs& args)
{
	if (!g_theInput)
	{
		return false;
	}
	unsigned char keyCode = (unsigned char) args.GetValue("KeyCode", -1);
	g_theInput->HandleKeyPressed(keyCode);
	return true;
}

bool InputSystem::Event_KeyReleased(EventArgs& args)
{
	if (!g_theInput)
	{
		return false;
	}
	unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);
	g_theInput->HandleKeyReleased(keyCode);
	return true;
}
