#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"

SoundID SOUND_ID_TEST = MISSING_SOUND_ID;
SoundID SOUND_ID_MENU_MUSIC = MISSING_SOUND_ID;
SoundID SOUND_ID_CLICK = MISSING_SOUND_ID;
SoundID SOUND_ID_LEVEL_1_MUSIC = MISSING_SOUND_ID;

Texture* TEXTURE_PLAY_BUTTON = nullptr;
Texture* TEXTURE_PLAY_BUTTON_PRESSED = nullptr;
Texture* TEXTURE_HOW_TO_PLAY_BUTTON = nullptr;
Texture* TEXTURE_HOW_TO_PLAY_BUTTON_PRESSED = nullptr;
Texture* TEXTURE_SETTINGS_BUTTON = nullptr;
Texture* TEXTURE_SETTINGS_BUTTON_PRESSED = nullptr;
Texture* TEXTURE_QUIT_BUTTON = nullptr;
Texture* TEXTURE_QUIT_BUTTON_PRESSED = nullptr;
Texture* TEXTURE_BACK_BUTTON = nullptr;
Texture* TEXTURE_BACK_BUTTON_PRESSED;
Texture* TEXTURE_PLUS_BUTTON = nullptr;
Texture* TEXTURE_PLUS_BUTTON_PRESSED = nullptr;
Texture* TEXTURE_MINUS_BUTTON = nullptr;
Texture* TEXTURE_MINUS_BUTTON_PRESSED = nullptr;
Texture* TEXTURE_MAIN_MENU_BORDER = nullptr;
Texture* TEXTURE_HOW_TO_PLAY_BORDER = nullptr;
Texture* TEXTURE_SETTINGS_BORDER = nullptr;

Vec2 GetScreenDimensions()
{
	return g_theGame->m_screenCamera.GetOrthoDimensions();
}

Vec2 GetWorldScreenDimensions()
{
	return g_theGame->m_worldCamera.GetOrthoDimensions();
}

Vec2 WorldCoordinatesToScreenCoordinates(Vec2 worldCoordinates)
{
	Vec2 screenCoordinates = worldCoordinates;
	screenCoordinates.x = RangeMap(screenCoordinates.x, 0.f, g_theGame->m_worldCamera.GetOrthoDimensions().x, 0.f, g_theGame->m_screenCamera.GetOrthoDimensions().x);
	screenCoordinates.y = RangeMap(screenCoordinates.y, 0.f, g_theGame->m_worldCamera.GetOrthoDimensions().y, 0.f, g_theGame->m_screenCamera.GetOrthoDimensions().y);

	return screenCoordinates;
}

