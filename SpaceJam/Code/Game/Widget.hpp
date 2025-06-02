#pragma once
#include "Game/GameCommon.hpp"


struct WidgetConfig
{
	std::string m_name = "";
	Vec2 m_panelSize = Vec2::ONE;
	Vec2 m_allignment = Vec2(.5f, .5f);
	std::string m_text = "";
	Vec2 m_textAlignment = Vec2(.5f, .5f);
	AABB2 m_textBounds = AABB2::ZERO_TO_ONE;
	Rgba8 m_panelColor = Rgba8::BLACK;
	float m_textHeight = 50.f;
	bool m_forceSquareAspect = false;
	Rgba8 m_borderColor = Rgba8::WHITE;
	float m_fontAspect = 1.f;
	Vec2 m_borderSize = Vec2(.025f, .025f);
	Rgba8 m_textColor = Rgba8::WHITE;
	Texture* m_texture = nullptr;
	float m_orientation = 0.f;
	bool m_matchPanelPadding = false;
};

class Widget
{
public:
	Widget(Widget* parent = nullptr, WidgetConfig const& config = WidgetConfig());
	Widget* AddChild(WidgetConfig const& config);
	void Build();
	WidgetConfig m_config;
	Widget* m_parent = nullptr;
	Widget* GetMouseHoveredWidgetInHeirarchy();
	void PollForButtonPress();
	AABB2 m_screenBounds;
	bool m_enabled = true;
	std::string m_pressEvent = "";
	std::vector<Widget*> m_children;
	std::vector<Vertex_PCU> m_panelVerts;
	std::vector<Vertex_PCU> m_borderVerts;
	std::vector<Vertex_PCU> m_textVerts;
	void Render() const;
};