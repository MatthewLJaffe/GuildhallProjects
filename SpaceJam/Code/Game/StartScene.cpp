#include "StartScene.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"

void StartScene::StartUp()
{
	if (m_startedUp)
	{
		return;
	}
	WidgetConfig title;
	title.m_name = "title";
	title.m_text = "Space Jam";
	title.m_borderColor = Rgba8(0, 0, 0, 0);
	title.m_allignment = Vec2(.5f, .5f);
	title.m_textHeight = 100;
	title.m_textAlignment = Vec2(.5f, .75f);
	title.m_textColor = Rgba8::WHITE;
	Widget* titleWidget = new Widget(nullptr, title);
	m_attractWidgets.push_back(titleWidget);

	WidgetConfig pressSpace;
	pressSpace.m_name = "pressSpace";
	pressSpace.m_text = "Press Space";
	pressSpace.m_borderColor = Rgba8(0, 0, 0, 0);
	pressSpace.m_panelColor = Rgba8(0, 0, 0, 0);
	pressSpace.m_allignment = Vec2(.5f, .5f);
	pressSpace.m_textHeight = 50;
	pressSpace.m_textAlignment = Vec2(.5f, .3f);
	pressSpace.m_textColor = Rgba8::WHITE;
	Widget* pressSpaceWidget = new Widget(nullptr, pressSpace);
	m_attractWidgets.push_back(pressSpaceWidget);

	m_startedUp = true;
}

void StartScene::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		std::vector<Widget*>& currentWidgets = GetWidgetsForCurrentMenuState();
		for (int i = 0; i < (int)currentWidgets.size(); i++)
		{
			if (currentWidgets[i] != nullptr)
			{
				currentWidgets[i]->PollForButtonPress();
			}
		}
	}
	else if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		if (m_menuState == MenuState::ATTRACT)
		{
			g_theGame->SetDesiredGameState(GameState::GAME);

			WidgetConfig load;
			load.m_name = "load";
			load.m_text = "Loading...";
			load.m_borderColor = Rgba8(0, 0, 0, 0);
			load.m_panelColor = Rgba8(0, 0, 0, 255);
			load.m_allignment = Vec2(.5f, .5f);
			load.m_textHeight = 50;
			load.m_textAlignment = Vec2(.5f, .5f);
			load.m_textColor = Rgba8::WHITE;
			Widget* pressSpaceWidget = new Widget(nullptr, load);
			m_attractWidgets.push_back(pressSpaceWidget);

		}
	}
	//DebugAddMessage(Stringf("%.2f MS", g_theApp->m_lastFrameTime * 1000.0), 20.f, 0.f);
}

void StartScene::Render() const
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->BindShader(nullptr);

	g_theRenderer->SetModelConstants();
	g_theRenderer->SetPrimaryRenderTargetType(PrimaryRenderTargetType::SDR);
	g_theRenderer->BeginCamera(g_theGame->m_screenCamera);
	std::vector<Widget*> const& currentWidgets = GetWidgetsForCurrentMenuStateConst();
	for (int i = 0; i < (int)currentWidgets.size(); i++)
	{
		if (currentWidgets[i] != nullptr)
		{
			currentWidgets[i]->Render();
		}
	}
	g_theRenderer->EndCamera(g_theGame->m_screenCamera);
	
}

void StartScene::ShutDown()
{
	for (int i = 0; i < (int)m_attractWidgets.size(); i++)
	{
		delete m_attractWidgets[i];
	}
	for (int i = 0; i < (int)m_creditsWidgets.size(); i++)
	{
		delete m_creditsWidgets[i];
	}
	for (int i = 0; i < (int)m_mainWidgets.size(); i++)
	{
		delete m_mainWidgets[i];
	}
	for (int i = 0; i < (int)m_settingsWidgets.size(); i++)
	{
		delete m_settingsWidgets[i];
	}
}

std::vector<Widget*>& StartScene::GetWidgetsForCurrentMenuState()
{
	switch (m_menuState)
	{
	case MenuState::ATTRACT:
		return m_attractWidgets;
	case MenuState::MAIN:
		return m_mainWidgets;
	case MenuState::SETTINGS:
		return m_settingsWidgets;
	case MenuState::CREDITS:
		return m_creditsWidgets;
	default:
		ERROR_AND_DIE("INVALID MENU STATE");
	}
}

std::vector<Widget*> const& StartScene::GetWidgetsForCurrentMenuStateConst() const
{
	switch (m_menuState)
	{
	case MenuState::ATTRACT:
		return m_attractWidgets;
	case MenuState::MAIN:
		return m_mainWidgets;
	case MenuState::SETTINGS:
		return m_settingsWidgets;
	case MenuState::CREDITS:
		return m_creditsWidgets;
	default:
		ERROR_AND_DIE("INVALID MENU STATE");
	}
}
