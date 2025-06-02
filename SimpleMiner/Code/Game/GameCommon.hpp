#pragma once
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/BlockDefinition.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

class World;

extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_theWindow;
extern App* g_theApp;
extern RandomNumberGenerator* g_randGen;
extern Game* g_theGame;
extern World* g_theWorld;

//sounds
extern SoundID SOUND_ID_TEST;
extern SpriteSheet* SPRITE_SHEET_BASIC_SPRITES;

extern Rgba8 SkyColor;

Vec2 GetScreenDimensions();

enum class Direction
{
	EAST = 0,
	WEST = 1,
	NORTH = 2,
	SOUTH = 3,
	TOP = 4,
	BOTTOM = 5,
	COUNT = 6
};
