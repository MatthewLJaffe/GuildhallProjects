#include "Game/Widget.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

Widget::Widget(Widget* parent, WidgetConfig const& config)
	:m_parent(parent)
	, m_config(config)
{
	Build();
}

Widget* Widget::AddChild(WidgetConfig const& config)
{
	Widget* childWidget = new Widget(this, config);
	m_children.push_back(childWidget);
	return childWidget;
}

void Widget::Build()
{
	m_panelVerts.clear();
	m_textVerts.clear();
	m_borderVerts.clear();
	AABB2 parentScreenBounds;
	if (m_parent == nullptr)
	{
		parentScreenBounds = AABB2(g_theGame->m_screenCamera.GetOrthoBottomLeft(), g_theGame->m_screenCamera.GetOrthoTopRight());
	}
	else
	{
		parentScreenBounds = m_parent->m_screenBounds;
	}
	Vec2 screenDimensions = Vec2(parentScreenBounds.GetDimensions().x * m_config.m_panelSize.x,
		parentScreenBounds.GetDimensions().y * m_config.m_panelSize.y);
	if (m_config.m_matchPanelPadding)
	{
		Vec2 dimensionDiff = parentScreenBounds.GetDimensions() - screenDimensions;
		if (dimensionDiff.x > dimensionDiff.y)
		{
			dimensionDiff.y = dimensionDiff.x;
		}
		else
		{
			dimensionDiff.x = dimensionDiff.y;
		}
		screenDimensions = parentScreenBounds.GetDimensions() - dimensionDiff;
	}

	if (m_config.m_forceSquareAspect)
	{
		float length = screenDimensions.x < screenDimensions.y ? screenDimensions.x : screenDimensions.y;
		screenDimensions.x = length;
		screenDimensions.y = length;

		screenDimensions *= g_theWindow->GetAspectRatio() * .5f;
	}

	Vec2 screenMins;
	screenMins.x = Lerp(parentScreenBounds.m_mins.x, parentScreenBounds.m_mins.x + (parentScreenBounds.GetDimensions().x - screenDimensions.x), m_config.m_allignment.x);
	screenMins.y = Lerp(parentScreenBounds.m_mins.y, parentScreenBounds.m_mins.y + (parentScreenBounds.GetDimensions().y - screenDimensions.y), m_config.m_allignment.y);
	m_screenBounds = AABB2(screenMins, screenMins + screenDimensions);
	if (m_config.m_borderSize != Vec2::ZERO)
	{
		AddVertsForAABB2D(m_borderVerts, m_screenBounds, m_config.m_borderColor);
	}
	AABB2 panelBounds = m_screenBounds.GetFractionOfBox(Vec2::ZERO + m_config.m_borderSize * .5f, Vec2::ONE - m_config.m_borderSize * .5f);
	float xPadding = m_screenBounds.GetDimensions().x - panelBounds.GetDimensions().x;
	float yPadding = m_screenBounds.GetDimensions().y - panelBounds.GetDimensions().y;
	float padding = xPadding > yPadding ? xPadding : yPadding;
	panelBounds.m_mins = m_screenBounds.m_mins + .5f * Vec2(padding * (2.f / g_theWindow->GetAspectRatio()), padding);
	panelBounds.m_maxs = m_screenBounds.m_maxs - .5f * Vec2(padding * (2.f / g_theWindow->GetAspectRatio()), padding);
	AddVertsForAABB2D(m_panelVerts, panelBounds, m_config.m_panelColor);
	AABB2 textBox = m_screenBounds.GetFractionOfBox(m_config.m_textBounds.m_mins, m_config.m_textBounds.m_maxs);
	if (m_config.m_text != "")
	{
		float scalingFactor = RangeMap(g_theWindow->GetAspectRatio(), 1.f, 4.f, .5f, 1.25f);
		g_theGame->font->AddVertsForTextInBox2D(m_textVerts, textBox, m_config.m_textHeight * scalingFactor, m_config.m_text, m_config.m_textColor,
			.5f, m_config.m_textAlignment);
	}

	if (m_config.m_orientation != 0.f)
	{
		Vec2 newIBasis = Vec2::MakeFromPolarDegrees(m_config.m_orientation);
		Vec2 newJBasis = newIBasis.GetRotated90Degrees();
		Vec2 textCenter = (textBox.m_mins + textBox.m_maxs) * .5f;
		Vec2 panelCenter = (panelBounds.m_mins + panelBounds.m_maxs) * .5f;
		Vec2 borderCenter = (m_screenBounds.m_mins + m_screenBounds.m_maxs) * .5f;
		TransformVertexArrayXY3D(m_textVerts.size(), m_textVerts.data(), Vec2(1.f, 0.f), Vec2(0.f, 1.f), -textCenter);
		TransformVertexArrayXY3D(m_textVerts.size(), m_textVerts.data(), newIBasis, newJBasis, textCenter);

		TransformVertexArrayXY3D(m_borderVerts.size(), m_borderVerts.data(), Vec2(1.f, 0.f), Vec2(0.f, 1.f), -borderCenter);
		TransformVertexArrayXY3D(m_borderVerts.size(), m_borderVerts.data(), newIBasis, newJBasis, borderCenter);

		TransformVertexArrayXY3D(m_panelVerts.size(), m_panelVerts.data(), Vec2(1.f, 0.f), Vec2(0.f, 1.f), -panelCenter);
		TransformVertexArrayXY3D(m_panelVerts.size(), m_panelVerts.data(), newIBasis, newJBasis, panelCenter);

	}

	for (int i = 0; i < (int)m_children.size(); i++)
	{
		m_children[i]->Build();
	}
}

Widget* Widget::GetMouseHoveredWidgetInHeirarchy()
{
	Vec2 mousePos = g_theInput->GetCursorNormalizedPosition();
	mousePos.x *= g_theGame->m_screenCamera.GetOrthoDimensions().x;
	mousePos.y *= g_theGame->m_screenCamera.GetOrthoDimensions().y;
	if (IsPointInsideAABB2D(mousePos, m_screenBounds))
	{
		for (int i = 0; i < (int)m_children.size(); i++)
		{
			Widget* hoveredChild = m_children[i]->GetMouseHoveredWidgetInHeirarchy();
			if (hoveredChild != nullptr)
			{
				return hoveredChild;
			}
		}
		if (m_enabled || m_pressEvent != "")
		{
			return this;
		}
	}
	return nullptr;
}

void Widget::PollForButtonPress()
{
	Widget* pressedWidget = GetMouseHoveredWidgetInHeirarchy();
	if (pressedWidget != nullptr)
	{
		g_theEventSystem->FireEvent(pressedWidget->m_pressEvent);
	}
}

void Widget::Render() const
{
	if (!m_enabled)
	{
		return;
	}
	g_theRenderer->SetModelConstants();
	//g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	if (m_borderVerts.size() > 0)
	{
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(m_borderVerts.size(), m_borderVerts.data());
	}
	g_theRenderer->BindTexture(m_config.m_texture);
	g_theRenderer->DrawVertexArray(m_panelVerts.size(), m_panelVerts.data());

	if (m_textVerts.size() > 0)
	{
		g_theRenderer->BindTexture(g_theGame->font->GetTexture());
		g_theRenderer->DrawVertexArray(m_textVerts.size(), m_textVerts.data());
	}

	for (int i = 0; i < (int)m_children.size(); i++)
	{
		m_children[i]->Render();
	}
}
