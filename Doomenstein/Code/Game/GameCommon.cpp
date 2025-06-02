#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"

SoundID SOUND_ID_TEST;
SoundID SOUND_ID_BUTTON_CLICK;
SoundID SOUND_ID_MENU_MUSIC;
SoundID SOUND_ID_GAME_MUSIC;
SoundID SOUND_ID_TYPEWRITER_1;
SoundID SOUND_ID_WEAPON_PICKUP;
SoundID SOUND_ID_ERROR_MESSAGE;

Vec2 GetScreenDimensions()
{
	float screenWidth = g_gameConfigBlackboard.GetValue("screenWidth", 1600.f);
	float screenHeight = g_gameConfigBlackboard.GetValue("screenHeight", 800.f);
	return Vec2(screenWidth, screenHeight);
}
