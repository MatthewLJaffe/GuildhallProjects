#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"

SoundID SOUND_ID_TEST;
SpriteSheet* SPRITE_SHEET_BASIC_SPRITES = nullptr;
Rgba8 SkyColor = Rgba8(200, 230, 255);

Vec2 GetScreenDimensions()
{
	return g_theGame->m_player->m_playerCamera.GetOrthoDimensions();
}
