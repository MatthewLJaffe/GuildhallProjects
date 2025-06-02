#pragma once
#pragma warning (disable : 26812)
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/DevConsole.hpp"
#include <Vector>

class DevConsole;
class EventSystem;
class Window;
class InputSystem;
class JobSystem;
class Renderer;
class NetSystem;

#define UNUSED(x) (void)(x);
extern NamedStrings g_gameConfigBlackboard;
extern DevConsole* g_theDevConsole;
extern EventSystem* g_theEventSystem;
extern InputSystem* g_theInput;
extern JobSystem* g_theJobSystem;
extern Renderer* g_theRenderer;
extern NetSystem* g_theNetSystem;

std::string GetShaderFilePath(std::string shaderName);
