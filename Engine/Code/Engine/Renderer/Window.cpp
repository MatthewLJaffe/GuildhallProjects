#include "Engine/Renderer/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "ThirdParty/imgui/imgui_impl_win32.h"
#include <commdlg.h>
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"

Window* Window::s_theWindow = nullptr;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);




//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called back by Windows whenever we tell it to (by calling DispatchMessage).
//
// 

LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	//get "THE" window (assumes for now we only have one)
	Window* window = Window::GetTheWindowInstance();
	GUARANTEE_OR_DIE(window != nullptr, "Window was null!");

	// Ask the Window for a pointer to the InputSystem it was created with (in its InputSystemConfig)
	InputSystem* input = window->GetConfig().m_inputSystem;
	GUARANTEE_OR_DIE(input != nullptr, "Window's InputSystem pointer was null!");


	if (ImGui_ImplWin32_WndProcHandler(windowHandle, wmMessageCode, wParam, lParam))
	{
		return true;
	}
	
	bool blockMouseInput = false;
	bool blockKeyboardInput = false;
	if (window->GetConfig().m_enableIMGUI)
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		blockKeyboardInput = io.WantCaptureKeyboard;
		blockMouseInput = io.WantCaptureMouse;

		if ((unsigned int)wParam == KEYCODE_TAB)
		{
			blockKeyboardInput = false;
		}
	}
	
	switch (wmMessageCode)
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
		case WM_CLOSE:
		{
			FireEvent("Quit");
			return 0; // "Consumes" this message (tells Windows "okay, we handled it")
		}

		// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
		case WM_KEYDOWN:
		{
			
			if (blockKeyboardInput)
			{
				return 0;
			}
		
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
			FireEvent("KeyPressed", args);
			return 0; // "Consumes" this message (tells Windows "okay, we handled it")
			
			break;
		}
		case WM_CHAR:
		{
			EventArgs args;
			args.SetValue("Char", Stringf("%d", (unsigned char)wParam));
			FireEvent("CharSent", args);

			break;
		}

		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
			FireEvent("KeyReleased", args);
			return 0; // "Consumes" this message (tells Windows "okay, we handled it")
			
			break;
		}

		case WM_LBUTTONDOWN:
		{
			
			if (blockMouseInput)
			{
				return 0;
			}
			
			unsigned char keycode = KEYCODE_LEFT_MOUSE;
			bool wasConsumed = false;
			if (input)
			{
				wasConsumed = input->HandleKeyPressed(keycode);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		case WM_RBUTTONDOWN:
		{
			
			if (blockMouseInput)
			{
				return 0;
			}
			
			unsigned char keycode = KEYCODE_RIGHT_MOUSE;
			bool wasConsumed = false;
			if (input)
			{
				wasConsumed = input->HandleKeyPressed(keycode);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		case WM_LBUTTONUP:
		{
			unsigned char keycode = KEYCODE_LEFT_MOUSE;
			bool wasConsumed = false;
			if (input)
			{
				wasConsumed = input->HandleKeyReleased(keycode);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		case WM_RBUTTONUP:
		{
			unsigned char keycode = KEYCODE_RIGHT_MOUSE;
			bool wasConsumed = false;
			if (input)
			{
				wasConsumed = input->HandleKeyReleased(keycode);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}


WindowConfig const& Window::GetConfig() const
{
	return m_config;
}

Window* Window::GetTheWindowInstance()
{
	return s_theWindow;
}

void* Window::GetDisplayContext() const
{
	return m_displayContext;
}

Vec2 Window::GetNormalizedCursorPos() const
{
	return m_config.m_inputSystem->GetCursorNormalizedPosition();
	/*
	HWND windowHandle = HWND(m_windowHandle);
	POINT cursorCoords;
	RECT clientRect;
	::GetCursorPos(&cursorCoords); // in screen coordinates (0, 0) top-left
	::ScreenToClient(windowHandle, &cursorCoords);
	::GetClientRect(windowHandle, &clientRect);
	float cursorX = float(cursorCoords.x) / float(clientRect.right);  // normalized x pos
	float cursorY = float(cursorCoords.y) / float(clientRect.bottom); // normalized y pos
	return Vec2( cursorX, 1.f - cursorY ); //0, 0 in bottom left
	*/
}

float Window::GetAspectRatio()
{
	return m_config.m_clientAspect;
}

void* Window::GetHwnd() const
{
	return m_windowHandle;
}

IntVec2 Window::GetClientDimensions()
{
	return m_clientDimensions;
}

bool Window::IsWindowFocus()
{
	return m_windowHandle == GetActiveWindow();
}

std::string Window::GetFileFromWindowsExplorer()
{
	char currentDirectory[MAX_PATH];
	currentDirectory[0] = '\0';
	GetCurrentDirectoryA(MAX_PATH, currentDirectory);

	char directory[MAX_PATH];
	directory[0] = '\0';
	char filename[MAX_PATH];
	filename[0] = '\0';

	OPENFILENAMEA data = { };
	data.lStructSize = sizeof(data);
	data.lpstrFile = filename;
	data.nMaxFile = sizeof(filename);
	data.lpstrFilter = "All\0*.*\0";
	data.nFilterIndex = 1;
	data.lpstrInitialDir = directory;
	data.hwndOwner = (HWND)GetHwnd();
	data.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	CursorState currCursorState = g_theInput->GetCursorMode();
	g_theInput->SetCursorMode(false, false);
	g_theInput->UpdateCursorState();
	bool result = GetOpenFileNameA(&data);
	g_theInput->SetCursorMode(currCursorState.m_currentHiddenMode, currCursorState.m_relativeMode);
	SetCurrentDirectoryA(currentDirectory);
	if (result)
	{
		std::string filePath = "";
		int sIdx = 0;
		while (data.lpstrFile[sIdx] != '\0')
		{
			filePath += data.lpstrFile[sIdx];
			sIdx++;
		}
		Strings filePathSplit = SplitStringOnDelimiter(filePath, '\\');
		std::string finalFilePath;
		bool reachedRelative = false;
		for (int i = 0; i < filePathSplit.size(); i++)
		{
			//trim file so that it is relative to the current project
			if (!reachedRelative)
			{
				if (filePathSplit[i] == "Run")
				{
					reachedRelative = true;
				}
				continue;
			}
			finalFilePath += filePathSplit[i];
			if (i != (int)filePathSplit.size() - 1)
			{
				finalFilePath += '/';
			}
		}

		return finalFilePath;
	}
	return "";
}

Vec3 Window::GetWorldMousePosition(Camera const& worldCamera)
{
	Mat44 viewMatrix = worldCamera.m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	//0, 0 bottom left 1,1 top right
	Vec2 normalizedScreenPos = GetNormalizedCursorPos();
	//DebugAddMessage(Stringf("Normalized cursor pos %.2f, %.2f", normalizedScreenPos.x, normalizedScreenPos.y), 20.f, 0.f);
	float iBasisDistance = worldCamera.GetPerspectiveNear();
	float worldNearPlaneHeight = 2.f * iBasisDistance * TanDegrees(worldCamera.GetPerspectiveFOV() * .5f);
	float worldNearPlaneWidth = worldNearPlaneHeight * worldCamera.GetAspectRatio();
	float jBasisDistance = -RangeMapClamped(normalizedScreenPos.x, 0.f, 1.f, -.5f*worldNearPlaneWidth, .5f*worldNearPlaneWidth);
	float kBasisDistance = RangeMapClamped(normalizedScreenPos.y, 0.f, 1.f, -.5f*worldNearPlaneHeight, .5f*worldNearPlaneHeight);
	Vec3 dispFromCameraPos = iBasisDistance * viewMatrix.GetIBasis3D() + jBasisDistance * viewMatrix.GetJBasis3D() + kBasisDistance * viewMatrix.GetKBasis3D();
	return worldCamera.m_position + dispFromCameraPos;
}

void Window::CreateOSWindow()
{
	HMODULE applicationInstanceHandle = GetModuleHandle(NULL);

	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = applicationInstanceHandle;
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	DWORD windowStyleFlags;
	DWORD windowStyleExFlags;

	if (m_config.m_isFullscreen)
	{
		windowStyleFlags = WS_POPUP | WS_VISIBLE;
		windowStyleExFlags = WS_EX_APPWINDOW;
	}
	else
	{
		windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
		windowStyleExFlags = WS_EX_APPWINDOW;
	}


	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	RECT clientRect;

	if (m_config.m_isFullscreen)
	{
		m_config.m_screenWidth = (float)GetSystemMetrics(SM_CXSCREEN);
		m_config.m_screenHeight = (float)GetSystemMetrics(SM_CYSCREEN);
		clientRect.left = 0;
		clientRect.top = 0;
		clientRect.right = (int)m_config.m_screenWidth;
		clientRect.bottom = (int)m_config.m_screenHeight;

		m_config.m_clientAspect = (float)clientRect.right / (float)clientRect.bottom;
	}
	else
	{
		GetClientRect(desktopWindowHandle, &desktopRect);
		float desktopWidth = (float)(desktopRect.right - desktopRect.left);
		float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
		float desktopAspect = desktopWidth / desktopHeight;

		// Calculate maximum client size (as some % of desktop size)
		constexpr float maxClientFractionOfDesktop = 0.90f;
		float clientWidth;
		float clientHeight;

		if (m_config.m_size != IntVec2(-1, -1))
		{
			clientWidth = (float)m_config.m_size.x;
			clientHeight = (float)m_config.m_size.y;
			m_config.m_clientAspect = clientWidth / clientHeight;
		}
		else
		{
			clientWidth = desktopWidth * maxClientFractionOfDesktop;
			clientHeight = desktopHeight * maxClientFractionOfDesktop;
		}

		if (m_config.m_clientAspect > desktopAspect)
		{
			// Client window has a wider aspect than desktop; shrink client height to match its width
			clientHeight = clientWidth / m_config.m_clientAspect;
		}
		else
		{
			// Client window has a taller aspect than desktop; shrink client width to match its height
			clientWidth = clientHeight * m_config.m_clientAspect;
		}

		// Calculate client rect bounds by centering the client area
		float clientMarginX = 0.5f * (desktopWidth - clientWidth);
		float clientMarginY = 0.5f * (desktopHeight - clientHeight);
		if (m_config.m_position != IntVec2(-1, -1))
		{
			clientMarginX = (float)m_config.m_position.x;
			clientMarginY = (float)m_config.m_position.y;
		}

		clientRect.left = (int)clientMarginX;
		clientRect.right = clientRect.left + (int)clientWidth;
		clientRect.top = (int)clientMarginY;
		clientRect.bottom = clientRect.top + (int)clientHeight;
	}

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, m_config.m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	m_windowHandle = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		(HINSTANCE)applicationInstanceHandle,
		NULL);

	ShowWindow((HWND)m_windowHandle, SW_SHOW);
	SetForegroundWindow((HWND)m_windowHandle);
	SetFocus((HWND)m_windowHandle);

	m_displayContext = GetDC((HWND)m_windowHandle);

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);

	GetClientRect((HWND)m_windowHandle, &clientRect);
	m_clientDimensions = IntVec2((int)(clientRect.right - clientRect.left), (int)(clientRect.bottom - clientRect.top));

	// Setup Platform backend for ImGui
	if (m_config.m_enableIMGUI)
	{
		ImGui_ImplWin32_Init(m_windowHandle);
	}
}

void Window::RunMessagePump()
{
	MSG queuedMessage;
	for (;; )
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}

Window::Window(WindowConfig const& config)
	: m_config( config )
{
	s_theWindow = this;
}

Window::~Window()
{
}

void Window::StartUp()
{
	CreateOSWindow();
}

void Window::BeginFrame()
{
	RunMessagePump();
}

void Window::EndFrame()
{
	
}

void Window::Shutdown()
{
	if (m_config.m_enableIMGUI)
	{
		ImGui_ImplWin32_Shutdown();
	}
}
