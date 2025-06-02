#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"

SoundID SOUND_ID_TEST;
BitmapFont* g_bitmapFont;

float GetScreenWidth()
{
    return g_theApp->GetGame()->m_screenCamera.GetOrthoDimensions().x;
}

float GetScreenHeight()
{
	return g_theApp->GetGame()->m_screenCamera.GetOrthoDimensions().y;
}
