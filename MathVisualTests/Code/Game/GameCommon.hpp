#pragma once
#include "Game/App.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

extern BitmapFont* g_bitmapFont;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern Window* g_theWindow;
extern App* g_theApp;
extern RandomNumberGenerator* g_randGen;

//sounds
extern SoundID SOUND_ID_TEST;

const Rgba8 LIGHT_BLUE = Rgba8(100, 150, 255);
const Rgba8 DARK_BLUE = Rgba8(50, 50, 100);
const Rgba8 DARK_GREY = Rgba8(50, 50, 50);

float GetScreenWidth();
float GetScreenHeight();





