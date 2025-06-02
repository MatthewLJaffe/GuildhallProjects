#include "Game/GameStateHowToPlay.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Button.hpp"
#include "Game/GameStateMainMenu.hpp"

GameStateHowToPlay::GameStateHowToPlay(GameStateType gameStateType)
	: GameState(gameStateType)
{
}

GameStateHowToPlay::~GameStateHowToPlay()
{
	GameState::~GameState();
}

void GameStateHowToPlay::StartUp()
{
	ButtonConfig buttonConfig;
	buttonConfig.m_clickDimensions = Vec2(200.f, 100.f);
	buttonConfig.m_eventName = "Back";
	buttonConfig.m_idleSprite = TEXTURE_BACK_BUTTON;
	buttonConfig.m_pressedSprite = TEXTURE_BACK_BUTTON_PRESSED;
	buttonConfig.m_pressedNoise = SOUND_ID_CLICK;
	buttonConfig.m_spriteDimensions = Vec2(200.f, 100.f);
	buttonConfig.m_cellHeight = 25.f;

	Vec2 firstButtonPos = Vec2(0.f, GetScreenDimensions().y) + Vec2(150.f, -100.f);
	AddEntity(new Button(this, EntityType::BUTTON, firstButtonPos, buttonConfig));
}

void GameStateHowToPlay::Render()
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);

	Vec2 screenDimensions = GetScreenDimensions();
	std::vector<Vertex_PCU> menuBorderVerts;
	g_theRenderer->BindTexture(TEXTURE_HOW_TO_PLAY_BORDER);
	AddVertsForAABB2D(menuBorderVerts, AABB2(Vec2::ZERO, screenDimensions), Rgba8::WHITE);
	g_theRenderer->DrawVertexArray(menuBorderVerts.size(), menuBorderVerts.data());
	g_theRenderer->BindTexture(g_bitMapFont->GetTexture());
	std::vector<Vertex_PCU> howToPlayTextVerts;
	AABB2 howToPlayBounds = AABB2(screenDimensions.x * .1f, screenDimensions.y * .2f, screenDimensions.x * .9f, screenDimensions.y * .8f);
	g_bitMapFont->AddVertsForTextInBox2D(howToPlayTextVerts, howToPlayBounds, 40.f, 
		"Objective:\nAvoid red shapes on the screen. Reach the end of the level\nwhile taking minimal damage to increase your score.\n\nControls:\nMove-\tWASD\nDash-\tSpace\nQuit-\tESC", Rgba8::WHITE, 1.f, Vec2(0.f,.5f));
	g_theRenderer->DrawVertexArray(howToPlayTextVerts.size(), howToPlayTextVerts.data());
	GameState::Render();
}
