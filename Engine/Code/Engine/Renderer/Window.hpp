#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"

class InputSystem;
struct Vec2;
struct IntVec2;

struct WindowConfig
{
	InputSystem* m_inputSystem = nullptr;
	float m_clientAspect = 2.0f;
	std::string m_windowTitle = "Untitled App";
	float m_screenHeight = 8;
	float m_screenWidth = 16;
	IntVec2 m_position = IntVec2(-1, -1);
	IntVec2 m_size = IntVec2(-1, -1);
	bool m_isFullscreen = false;
	bool m_enableIMGUI = false;
};

class Window
{
public:
	Window( WindowConfig const& config );
	~Window();
	
	void StartUp();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	WindowConfig const& GetConfig() const;
	static Window* GetTheWindowInstance();
	void* GetDisplayContext() const;
	Vec2 GetNormalizedCursorPos() const;
	float GetAspectRatio();
	void* GetHwnd() const;
	IntVec2 GetClientDimensions();
	bool IsWindowFocus();
	std::string GetFileFromWindowsExplorer();
	Vec3 GetWorldMousePosition(Camera const& worldCamera);

protected:
	void CreateOSWindow();
	void RunMessagePump();

protected:
	IntVec2 m_clientDimensions;
	void* m_windowHandle = nullptr;
	void* m_displayContext = nullptr;
	WindowConfig m_config;
	static Window* s_theWindow; //#ToDo later on: refactor to support multiple windows
};