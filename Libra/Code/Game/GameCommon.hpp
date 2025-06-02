#pragma once
#include "Game/App.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

class Game;
class SpriteAnimDefinition;
class SpriteSheet;
class BitmapFont;

extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_theWindow;
extern DevConsole* g_theDevConsole;
extern App* g_theApp;
extern Game* g_theGame;
extern RandomNumberGenerator* g_randGen;
extern BitmapFont* g_bitmapFont;

//sounds
extern SoundID SOUND_ID_TEST;
extern SoundID SOUND_ID_STARTUP_MUSIC;
extern SoundID SOUND_ID_GAMEPLAY_MUSIC;
extern SoundID SOUND_ID_START;
extern SoundID SOUND_ID_QUIT;
extern SoundID SOUND_ID_PAUSE;
extern SoundID SOUND_ID_UNPAUSE;
extern SoundID SOUND_ID_PLAYER_SHOOT;
extern SoundID SOUND_ID_ENEMY_SHOOT;
extern SoundID SOUND_ID_PLAYER_HIT;
extern SoundID SOUND_ID_ENEMY_HIT;
extern SoundID SOUND_ID_ENEMY_DIED;
extern SoundID SOUND_ID_PLAYER_DIED;
extern SoundID SOUND_ID_VICTORY;
extern SoundID SOUND_ID_WELCOME;
extern SoundID SOUND_ID_BULLET_RICOCHET1;
extern SoundID SOUND_ID_BULLET_RICOCHET2;
extern SoundID SOUND_ID_DISCOVER;


extern Texture* TEXTURE_TILE_SHEET;
extern Texture* TEXTURE_EXPLOSION_SHEET;
extern Texture* TEXTURE_MISSILE;
extern Texture* TEXTURE_WIN_SCREEN;
extern Texture* TEXTURE_ARIES;
extern Texture* TEXTURE_CAPRICORN;
extern Texture* TEXTURE_GOOD_BULLET;
extern Texture* TEXTURE_EVIL_BULLET;

extern SpriteSheet* SPRITE_SHEET_TILES;
extern SpriteSheet* SPRITE_SHEET_EXPLOSION;

extern SpriteAnimDefinition* ANIM_DEFINITION_EXPLOSION;




