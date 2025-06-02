#include "GameStateLevelSelect.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Button.hpp"

GameStateLevelSelect::GameStateLevelSelect(GameStateType gameStateType)
	: GameState(gameStateType)
{
}

GameStateLevelSelect::~GameStateLevelSelect()
{
	GameState::~GameState();
}

void GameStateLevelSelect::StartUp()
{
	ButtonConfig buttonConfig;
	Vec2 screenDimensions = GetScreenDimensions();
	buttonConfig.m_clickDimensions = Vec2(200.f, 100.f);
	buttonConfig.m_eventName = "Back";
	buttonConfig.m_idleSprite = TEXTURE_BACK_BUTTON;
	buttonConfig.m_pressedSprite = TEXTURE_BACK_BUTTON_PRESSED;
	buttonConfig.m_pressedNoise = SOUND_ID_CLICK;
	buttonConfig.m_spriteDimensions = Vec2(200.f, 100.f);
	buttonConfig.m_cellHeight = 25.f;
	Vec2 firstButtonPos = Vec2(0.f, GetScreenDimensions().y) + Vec2(150.f, -100.f);
	AddEntity(new Button(this, EntityType::BUTTON, firstButtonPos, buttonConfig));

	buttonConfig.m_clickDimensions = Vec2(200.f, 200.f);
	buttonConfig.m_eventName = "Level1";
	g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_Level1Pressed);
	buttonConfig.m_idleSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Boss1Button1.png");
	buttonConfig.m_pressedSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Boss1Button2.png");
	buttonConfig.m_pressedNoise = SOUND_ID_CLICK;
	buttonConfig.m_spriteDimensions = Vec2(200.f, 200.f);
	Vec2 boss1ButtonPos = Vec2(screenDimensions.x*.225f, screenDimensions.y*.3f);
	AddEntity(new Button(this, EntityType::BUTTON, boss1ButtonPos, buttonConfig));

	buttonConfig.m_clickDimensions = Vec2(200.f, 200.f);
	buttonConfig.m_eventName = "Level2";
	g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_Level2Pressed);
	buttonConfig.m_idleSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Boss2Button1.png");
	buttonConfig.m_pressedSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Boss2Button2.png");
	buttonConfig.m_pressedNoise = SOUND_ID_CLICK;
	buttonConfig.m_spriteDimensions = Vec2(200.f, 200.f);
	Vec2 boss2ButtonPos = boss1ButtonPos + Vec2::RIGHT*425.f;
	AddEntity(new Button(this, EntityType::BUTTON, boss2ButtonPos, buttonConfig));

	buttonConfig.m_clickDimensions = Vec2(200.f, 200.f);
	buttonConfig.m_eventName = "Level3";
	g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_Level3Pressed);
	buttonConfig.m_idleSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Boss3Button1.png");
	buttonConfig.m_pressedSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Boss3Button2.png");
	buttonConfig.m_pressedNoise = SOUND_ID_CLICK;
	buttonConfig.m_spriteDimensions = Vec2(200.f, 200.f);
	Vec2 boss3ButtonPos = boss1ButtonPos + Vec2::RIGHT * 850.f;
	AddEntity(new Button(this, EntityType::BUTTON, boss3ButtonPos, buttonConfig));
}

void GameStateLevelSelect::Render()
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);

	std::vector<Vertex_PCU> menuBorderVerts;
	g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/LevelSelectBorder.png"));
	AddVertsForAABB2D(menuBorderVerts, AABB2(Vec2::ZERO, GetScreenDimensions()), Rgba8::WHITE);
	g_theRenderer->DrawVertexArray(menuBorderVerts.size(), menuBorderVerts.data());
	GameState::Render();
}

bool Event_Level1Pressed(EventArgs& args)
{
	UNUSED(args);
	g_theGame->SwitchGameState(GameStateType::LEVEL_1);
	return false;
}

bool Event_Level2Pressed(EventArgs& args)
{
	UNUSED(args);
	g_theGame->SwitchGameState(GameStateType::LEVEL_2);
	return false;
}

bool Event_Level3Pressed(EventArgs& args)
{
	UNUSED(args);
	g_theGame->SwitchGameState(GameStateType::LEVEL_3);
	return false;
}
