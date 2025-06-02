#pragma once
#include "Game/App.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/ActorUID.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/DialogSystem.hpp"


class Game;

extern Game* g_theGame;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_theWindow;
extern App* g_theApp;
extern RandomNumberGenerator* g_randGen;
extern BitmapFont* g_bitMapFont;
extern DialogSystem* g_dialogSystem;


//sounds
extern SoundID SOUND_ID_TEST;
extern SoundID SOUND_ID_BUTTON_CLICK;
extern SoundID SOUND_ID_MENU_MUSIC;
extern SoundID SOUND_ID_GAME_MUSIC;
extern SoundID SOUND_ID_TYPEWRITER_1;
extern SoundID SOUND_ID_WEAPON_PICKUP;
extern SoundID SOUND_ID_ERROR_MESSAGE;

enum class Faction
{
	MARINE,
	DEMON,
	NEUTRAL,
	COUNT
};

Vec2 GetScreenDimensions();

