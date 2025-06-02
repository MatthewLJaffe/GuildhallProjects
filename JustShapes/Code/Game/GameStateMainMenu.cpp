#include "Game/GameStateMainMenu.hpp"
#include "Game/Button.hpp"

GameStateMainMenu::GameStateMainMenu(GameStateType gameStateType)
	: GameState(gameStateType)
{
}

GameStateMainMenu::~GameStateMainMenu()
{
	GameState::~GameState();
}

void GameStateMainMenu::StartUp()
{
	g_theEventSystem->SubscribeEventCallbackFunction("Back", Event_BackButtonPressed);

	ButtonConfig buttonConfig;
	buttonConfig.m_clickDimensions = Vec2(200.f, 100.f);
	buttonConfig.m_eventName = "PlayPressed";
	g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_PlayButtonPressed);
	buttonConfig.m_idleSprite = TEXTURE_PLAY_BUTTON;
	buttonConfig.m_pressedSprite = TEXTURE_PLAY_BUTTON_PRESSED;
	buttonConfig.m_pressedNoise = SOUND_ID_CLICK;
	buttonConfig.m_spriteDimensions = Vec2(200.f, 100.f);
	buttonConfig.m_cellHeight = 25.f;

	Vec2 firstButtonPos = GetScreenDimensions()*.5f + Vec2::UP * 50.f;
	AddEntity(new Button(this, EntityType::BUTTON, firstButtonPos, buttonConfig));

	buttonConfig.m_idleSprite = TEXTURE_HOW_TO_PLAY_BUTTON;
	buttonConfig.m_pressedSprite = TEXTURE_HOW_TO_PLAY_BUTTON_PRESSED;
	buttonConfig.m_eventName = "HowToPlayPressed";
	g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_HowToPlayButtonPressed);

	AddEntity(new Button(this, EntityType::BUTTON, firstButtonPos + Vec2::DOWN*120, buttonConfig));

	buttonConfig.m_idleSprite = TEXTURE_SETTINGS_BUTTON;
	buttonConfig.m_pressedSprite = TEXTURE_SETTINGS_BUTTON_PRESSED;
	buttonConfig.m_eventName = "SettingsPressed";
	g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_SettingsButtonPressed);
	AddEntity(new Button(this, EntityType::BUTTON, firstButtonPos + Vec2::DOWN * 240, buttonConfig));

	buttonConfig.m_idleSprite = TEXTURE_QUIT_BUTTON;
	buttonConfig.m_pressedSprite = TEXTURE_QUIT_BUTTON_PRESSED;
	buttonConfig.m_eventName = "Quit";
	AddEntity(new Button(this, EntityType::BUTTON, firstButtonPos + Vec2::DOWN*360, buttonConfig));
}

void GameStateMainMenu::Render()
{
	std::vector<Vertex_PCU> menuBorderVerts;
	g_theRenderer->BindTexture(TEXTURE_MAIN_MENU_BORDER);
	AddVertsForAABB2D(menuBorderVerts,AABB2(Vec2::ZERO, GetScreenDimensions()), Rgba8::WHITE);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->DrawVertexArray(menuBorderVerts.size(), menuBorderVerts.data());
	GameState::Render();
}

void GameStateMainMenu::OnEnable()
{
	g_theGame->PlaySound(SOUND_ID_MENU_MUSIC, SoundType::MUSIC, true, .5f);
}

bool Event_PlayButtonPressed(EventArgs& args)
{
	UNUSED(args);
	g_theGame->SwitchGameState(GameStateType::LEVEL_SELECT);
	return false;
}

bool Event_HowToPlayButtonPressed(EventArgs& args)
{
	UNUSED(args);
	g_theGame->SwitchGameState(GameStateType::HOW_TO_PLAY);
	return false;
}

bool Event_SettingsButtonPressed(EventArgs& args)
{
	UNUSED(args);
	g_theGame->SwitchGameState(GameStateType::SETTINGS);
	return false;
}

bool Event_BackButtonPressed(EventArgs& args)
{
	UNUSED(args);
	g_theApp->SetPause(false);
	g_theGame->SwitchGameState(GameStateType::MAIN_MENU);
	return false;
}
