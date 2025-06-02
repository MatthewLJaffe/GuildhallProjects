#include "GameStateSettings.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Button.hpp"

GameStateSettings::GameStateSettings(GameStateType gameStateType)
	: GameState(gameStateType)
{
}

GameStateSettings::~GameStateSettings()
{
	GameState::~GameState();
}

void GameStateSettings::StartUp()
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

	buttonConfig.m_clickDimensions = Vec2(80.f, 80.f);
	buttonConfig.m_eventName = "IncreaseMaster";
	g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_IncreaseMasterButtonPressed);
	buttonConfig.m_idleSprite = TEXTURE_PLUS_BUTTON;
	buttonConfig.m_pressedSprite = TEXTURE_PLUS_BUTTON_PRESSED;
	buttonConfig.m_spriteDimensions = Vec2(80.f, 80.f);
	Vec2 masterPlusButtonPos = Vec2(GetScreenDimensions().x * .625f + 100.f, GetScreenDimensions().y * .65f + 20.f);
	AddEntity(new Button(this, EntityType::BUTTON, masterPlusButtonPos, buttonConfig));

	buttonConfig.m_clickDimensions = Vec2(80.f, 80.f);
	buttonConfig.m_eventName = "DecreaseMaster";
	g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_DecreaseMasterButtonPressed);
	buttonConfig.m_idleSprite = TEXTURE_MINUS_BUTTON;
	buttonConfig.m_pressedSprite = TEXTURE_MINUS_BUTTON_PRESSED;
	buttonConfig.m_spriteDimensions = Vec2(80.f, 80.f);
	Vec2 masterMinusButtonPos = masterPlusButtonPos + Vec2::LEFT * 100.f;
	AddEntity(new Button(this, EntityType::BUTTON, masterMinusButtonPos, buttonConfig));

	buttonConfig.m_clickDimensions = Vec2(80.f, 80.f);
	buttonConfig.m_eventName = "IncreaseMusic";
	g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_IncreaseMusicButtonPressed);
	buttonConfig.m_idleSprite = TEXTURE_PLUS_BUTTON;
	buttonConfig.m_pressedSprite = TEXTURE_PLUS_BUTTON_PRESSED;
	buttonConfig.m_spriteDimensions = Vec2(80.f, 80.f);
	Vec2 musicPlusButtonPos = masterPlusButtonPos + Vec2::DOWN * 150.f;
	AddEntity(new Button(this, EntityType::BUTTON, musicPlusButtonPos, buttonConfig));

	buttonConfig.m_clickDimensions = Vec2(80.f, 80.f);
	buttonConfig.m_eventName = "DecreaseMusic";
	g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_DecreaseMusicButtonPressed);
	buttonConfig.m_idleSprite = TEXTURE_MINUS_BUTTON;
	buttonConfig.m_pressedSprite = TEXTURE_MINUS_BUTTON_PRESSED;
	buttonConfig.m_spriteDimensions = Vec2(80.f, 80.f);
	Vec2 musicMinusButtonPos = musicPlusButtonPos + Vec2::LEFT * 100.f;
	AddEntity(new Button(this, EntityType::BUTTON, musicMinusButtonPos, buttonConfig));

	buttonConfig.m_clickDimensions = Vec2(80.f, 80.f);
	buttonConfig.m_eventName = "IncreaseSFX";
	g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_IncreaseSFXButtonPressed);
	buttonConfig.m_idleSprite = TEXTURE_PLUS_BUTTON;
	buttonConfig.m_pressedSprite = TEXTURE_PLUS_BUTTON_PRESSED;
	buttonConfig.m_spriteDimensions = Vec2(80.f, 80.f);
	Vec2 sfxPlusButtonPos = masterPlusButtonPos + Vec2::DOWN * 300.f;
	AddEntity(new Button(this, EntityType::BUTTON, sfxPlusButtonPos, buttonConfig));

	buttonConfig.m_clickDimensions = Vec2(80.f, 80.f);
	buttonConfig.m_eventName = "DecreaseSFX";
	g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_DecreaseSFXButtonPressed);
	buttonConfig.m_idleSprite = TEXTURE_MINUS_BUTTON;
	buttonConfig.m_pressedSprite = TEXTURE_MINUS_BUTTON_PRESSED;
	buttonConfig.m_spriteDimensions = Vec2(80.f, 80.f);
	Vec2 sfxMinusButtonPos = sfxPlusButtonPos + Vec2::LEFT * 100.f;
	AddEntity(new Button(this, EntityType::BUTTON, sfxMinusButtonPos, buttonConfig));
}

void GameStateSettings::Render()
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);

	std::vector<Vertex_PCU> menuBorderVerts;
	g_theRenderer->BindTexture(TEXTURE_SETTINGS_BORDER);
	AddVertsForAABB2D(menuBorderVerts, AABB2(Vec2::ZERO, GetScreenDimensions()), Rgba8::WHITE);
	g_theRenderer->DrawVertexArray(menuBorderVerts.size(), menuBorderVerts.data());


	g_theRenderer->BindTexture(g_bitMapFont->GetTexture());
	std::vector<Vertex_PCU> optionsTextVerts;
	Vec2 masterTextMins = Vec2(GetScreenDimensions().x * .2f, GetScreenDimensions().y * .65f);
	g_bitMapFont->AddVertsForText2D(optionsTextVerts, masterTextMins, 30.f, Stringf("Master Volume:  %.1f", g_gameConfigBlackboard.GetValue("masterVolume", .5f)));

	g_bitMapFont->AddVertsForText2D(optionsTextVerts, masterTextMins + Vec2::DOWN*150.f, 30.f, Stringf("Music Volume:   %.1f", g_gameConfigBlackboard.GetValue("musicVolume", .5f)));

	g_bitMapFont->AddVertsForText2D(optionsTextVerts, masterTextMins + Vec2::DOWN * 300.f, 30.f, Stringf("SFX Volume:     %.1f", g_gameConfigBlackboard.GetValue("sfxVolume", .5f)));

	g_theRenderer->DrawVertexArray(optionsTextVerts.size(), optionsTextVerts.data());
	GameState::Render();
}

bool Event_IncreaseMasterButtonPressed(EventArgs& args)
{
	UNUSED(args);
	float newValue = Clamp(g_gameConfigBlackboard.GetValue("masterVolume", .5f) + .1f, 0.f, 1.f);
	g_gameConfigBlackboard.SetValue("masterVolume", Stringf("%f", newValue));
	g_theGame->UpdateMusicVolume();
	return false;
}

bool Event_DecreaseMasterButtonPressed(EventArgs& args)
{
	UNUSED(args);
	float newValue = Clamp(g_gameConfigBlackboard.GetValue("masterVolume", .5f) - .1f, 0.f, 1.f);
	g_gameConfigBlackboard.SetValue("masterVolume", Stringf("%f", newValue));
	g_theGame->UpdateMusicVolume();
	return false;
}

bool Event_IncreaseMusicButtonPressed(EventArgs& args)
{
	UNUSED(args);
	float newValue = Clamp(g_gameConfigBlackboard.GetValue("musicVolume", .5f) + .1f, 0.f, 1.f);
	g_gameConfigBlackboard.SetValue("musicVolume", Stringf("%f", newValue));
	g_theGame->UpdateMusicVolume();
	return false;
}

bool Event_DecreaseMusicButtonPressed(EventArgs& args)
{
	UNUSED(args);
	float newValue = Clamp(g_gameConfigBlackboard.GetValue("musicVolume", .5f) - .1f, 0.f, 1.f);
	g_gameConfigBlackboard.SetValue("musicVolume", Stringf("%f", newValue));
	g_theGame->UpdateMusicVolume();
	return false;
}

bool Event_IncreaseSFXButtonPressed(EventArgs& args)
{
	UNUSED(args);
	float newValue = Clamp(g_gameConfigBlackboard.GetValue("sfxVolume", .5f) + .1f, 0.f, 1.f);
	g_gameConfigBlackboard.SetValue("sfxVolume", Stringf("%f", newValue));
	return false;
}

bool Event_DecreaseSFXButtonPressed(EventArgs& args)
{
	UNUSED(args);
	float newValue = Clamp(g_gameConfigBlackboard.GetValue("sfxVolume", .5f) - .1f, 0.f, 1.f);
	g_gameConfigBlackboard.SetValue("sfxVolume", Stringf("%f", newValue));
	return false;
}
