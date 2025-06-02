#pragma once
#include "Game/App.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

//globals
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_theWindow;
extern App* g_theApp;
extern RandomNumberGenerator* g_randGen;
extern Game* g_theGame;
extern BitmapFont* g_bitMapFont;

//textures
extern Texture* TEXTURE_PLAY_BUTTON;
extern Texture* TEXTURE_PLAY_BUTTON_PRESSED;
extern Texture* TEXTURE_HOW_TO_PLAY_BUTTON;
extern Texture* TEXTURE_HOW_TO_PLAY_BUTTON_PRESSED;
extern Texture* TEXTURE_SETTINGS_BUTTON;
extern Texture* TEXTURE_SETTINGS_BUTTON_PRESSED;
extern Texture* TEXTURE_QUIT_BUTTON;
extern Texture* TEXTURE_QUIT_BUTTON_PRESSED;
extern Texture* TEXTURE_BACK_BUTTON;
extern Texture* TEXTURE_BACK_BUTTON_PRESSED;
extern Texture* TEXTURE_PLUS_BUTTON;
extern Texture* TEXTURE_PLUS_BUTTON_PRESSED;
extern Texture* TEXTURE_MINUS_BUTTON;
extern Texture* TEXTURE_MINUS_BUTTON_PRESSED;
extern Texture* TEXTURE_MAIN_MENU_BORDER;
extern Texture* TEXTURE_HOW_TO_PLAY_BORDER;
extern Texture* TEXTURE_SETTINGS_BORDER;


//sounds
extern SoundID SOUND_ID_TEST;
extern SoundID SOUND_ID_MENU_MUSIC;
extern SoundID SOUND_ID_LEVEL_1_MUSIC;
extern SoundID SOUND_ID_CLICK;



Vec2 GetScreenDimensions();
Vec2 GetWorldScreenDimensions();
Vec2 WorldCoordinatesToScreenCoordinates(Vec2 worldCoordinates);
