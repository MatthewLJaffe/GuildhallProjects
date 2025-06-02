#include "EndScene.hpp"

void EndScene::StartUp()
{
	if (m_startedUp)
	{
		return;
	}

	WidgetConfig winTitle;
	winTitle.m_name = "winTitle";
	winTitle.m_text = "You Win!";
	winTitle.m_borderColor = Rgba8(0, 0, 0, 0);
	winTitle.m_allignment = Vec2(.5f, .5f);
	winTitle.m_textHeight = 100;
	winTitle.m_textAlignment = Vec2(.5f, .75f);
	winTitle.m_textColor = Rgba8::WHITE;
	Widget* winTitleWidget = new Widget(nullptr, winTitle);
	m_winWidgets.push_back(winTitleWidget);

	WidgetConfig winPressSpace;
	winPressSpace.m_name = "pressSpace";
	winPressSpace.m_text = "Press Space";
	winPressSpace.m_borderColor = Rgba8(0, 0, 0, 0);
	winPressSpace.m_panelColor = Rgba8(0, 0, 0, 0);
	winPressSpace.m_allignment = Vec2(.5f, .5f);
	winPressSpace.m_textHeight = 50;
	winPressSpace.m_textAlignment = Vec2(.5f, .3f);
	winPressSpace.m_textColor = Rgba8::WHITE;
	Widget* winPressSpaceWidget = new Widget(nullptr, winPressSpace);
	m_winWidgets.push_back(winPressSpaceWidget);

	WidgetConfig loseTitle;
	loseTitle.m_name = "loseTitle";
	loseTitle.m_text = "You Lose";
	loseTitle.m_borderColor = Rgba8(0, 0, 0, 0);
	loseTitle.m_allignment = Vec2(.5f, .5f);
	loseTitle.m_textHeight = 100;
	loseTitle.m_textAlignment = Vec2(.5f, .75f);
	loseTitle.m_textColor = Rgba8::WHITE;
	Widget* loseTitleWidget = new Widget(nullptr, loseTitle);
	m_loseWidgets.push_back(loseTitleWidget);

	WidgetConfig losePressSpace;
	losePressSpace.m_name = "pressSpace";
	losePressSpace.m_text = "Press Space";
	losePressSpace.m_borderColor = Rgba8(0, 0, 0, 0);
	losePressSpace.m_panelColor = Rgba8(0, 0, 0, 0);
	losePressSpace.m_allignment = Vec2(.5f, .5f);
	losePressSpace.m_textHeight = 50;
	losePressSpace.m_textAlignment = Vec2(.5f, .3f);
	losePressSpace.m_textColor = Rgba8::WHITE;
	Widget* losePressSpaceWidget = new Widget(nullptr, losePressSpace);
	m_loseWidgets.push_back(losePressSpaceWidget);
	
	m_startedUp = true;
}

void EndScene::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		g_theApp->ResetGame();
	}
}

void EndScene::Render() const
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->BindShader(nullptr);

	g_theRenderer->SetModelConstants();
	g_theRenderer->SetPrimaryRenderTargetType(PrimaryRenderTargetType::SDR);
	g_theRenderer->BeginCamera(g_theGame->m_screenCamera);
	std::vector<Widget*> const& currentWidgets = m_wonGame ? m_winWidgets : m_loseWidgets;
	for (int i = 0; i < (int)currentWidgets.size(); i++)
	{
		if (currentWidgets[i] != nullptr)
		{
			currentWidgets[i]->Render();
		}
	}
	g_theRenderer->EndCamera(g_theGame->m_screenCamera);
}

void EndScene::ShutDown()
{
	for (int i = 0; i < (int)m_loseWidgets.size(); i++)
	{
		delete m_loseWidgets[i];
		m_loseWidgets[i] = nullptr;
	}
}
