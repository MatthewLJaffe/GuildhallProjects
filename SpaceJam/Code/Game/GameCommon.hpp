#pragma once
#include "Game/App.hpp"
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
#include "Game/Game.hpp"

extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_theWindow;
extern App* g_theApp;
extern RandomNumberGenerator* g_randGen;
extern Game* g_theGame;
class Actor;

//sounds
extern SoundID SOUND_ID_TEST;

static const float BEAT_SPEED_MULT = 1.f;
static const float BEAT_TIME = (60.f / (117.f * BEAT_SPEED_MULT));



void DebugDrawLine(Vec2 const& pointA, Vec2 const& pointB, float thickness, Rgba8 const& color);
bool IsValidActor(Actor* actorInQuestion);

