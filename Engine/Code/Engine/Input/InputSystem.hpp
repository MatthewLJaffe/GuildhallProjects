#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/IntVec2.hpp"

extern const unsigned char KEYCODE_F1;
extern const unsigned char KEYCODE_F2;
extern const unsigned char KEYCODE_F3;
extern const unsigned char KEYCODE_F4;
extern const unsigned char KEYCODE_F5;
extern const unsigned char KEYCODE_F6;
extern const unsigned char KEYCODE_F7;
extern const unsigned char KEYCODE_F8;
extern const unsigned char KEYCODE_F9;
extern const unsigned char KEYCODE_F10;
extern const unsigned char KEYCODE_F11;

extern const unsigned char KEYCODE_ESC;
extern const unsigned char KEYCODE_SPACE;
extern const unsigned char KEYCODE_TAB;


extern const unsigned char KEYCODE_LEFT_MOUSE;
extern const unsigned char KEYCODE_RIGHT_MOUSE;

constexpr unsigned char KEYCODE_L_SHIFT = 16;
constexpr unsigned char KEYCODE_LEFT_ARROW = 37;
constexpr unsigned char KEYCODE_UP_ARROW = 38;
constexpr unsigned char KEYCODE_RIGHT_ARROW = 39;
constexpr unsigned char KEYCODE_DOWN_ARROW = 40;
constexpr unsigned char KEYCODE_HOME = 33;
constexpr unsigned char KEYCODE_END = 35;
constexpr unsigned char KEYCODE_DELETE = 46;
constexpr unsigned char KEYCODE_BACKSPACE = 8;

constexpr unsigned char KEYCODE_TILDE = 0xC0;
constexpr unsigned char KEYCODE_ENTER = 13;
const unsigned char KEYCODE_LEFT_ARROW_BRACKET = 188;
const unsigned char KEYCODE_RIGHT_ARROW_BRACKET = 190;
const unsigned char KEYCODE_RIGHT_SQUARE_BRACKET = 221;
const unsigned char KEYCODE_LEFT_SQUARE_BRACKET = 219;

constexpr int NUM_KEYCODES = 256;
constexpr int NUM_XBOX_CONTROLLERS = 4;

class Window;

struct InputConfig
{
	Window* m_window;
};

struct CursorState
{
	IntVec2 m_cursorClientDelta;
	IntVec2 m_cursorClientPosition;

	bool m_desiredHiddenMode = false;
	bool m_currentHiddenMode = false;
	bool m_relativeMode = false;
};

class InputSystem
{
public:
	InputSystem(InputConfig& inputConfig);
	~InputSystem() = default;
	void StartUp();
	void ShutDown();
	void BeginFrame();
	void EndFrame();
	bool WasKeyJustPressed(unsigned char keyCode);
	bool WasKeyJustReleased(unsigned char keyCode);
	bool IsKeyDown(unsigned char keyCode);
	bool HandleKeyPressed(unsigned char keyCode);
	bool HandleKeyReleased(unsigned char keyCode);
	XboxController const& GetController(int controllerID);
	void UpdateCursorState();

	//Hidden mode controls if the cursor is visible or not
	//Relative mode will calculate a cursor client delta and then reset the cursor
	//and then reset the cursor to the client region center each frame
	//use these together to implement fps style mouse controls
	void SetCursorMode(bool hiddenMode, bool relativeMode);
	CursorState GetCursorMode();
	//returns the current frame cursor delta in pixels, relative to the client region
	//only valid in relative mode it will be zero otherwise
	IntVec2 GetCursorClientDelta() const;

	//Returns the cursor position, normalized to the range [0, 1], relative
	//to the client region, with the y axis inverted to map from windows 
	//conventions to game screen conventions
	Vec2 GetCursorNormalizedPosition() const;

	void ResetCursorClientDelta();

	IntVec2 GetCursorPosition() const;

	InputConfig m_config;

private:
	CursorState m_cursorState;
	XboxController m_xboxControllers[NUM_XBOX_CONTROLLERS] = {};
	KeyButtonState m_keyStates[NUM_KEYCODES] = {};
	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_KeyReleased(EventArgs& args);
};