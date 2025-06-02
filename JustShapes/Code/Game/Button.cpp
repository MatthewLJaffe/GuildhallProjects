#include "Game/Button.hpp"
#include "Game/GameCommon.hpp"

Button::Button(GameState* gameState, EntityType entityType, Vec2 const& startPos, ButtonConfig config)
	: Entity(gameState, entityType, startPos)
	, m_config(config)
{
	m_texture = config.m_idleSprite;
	m_spriteBounds = AABB2(Vec2::ZERO, config.m_spriteDimensions);
	m_clickBounds = AABB2(m_position - .5f*m_config.m_spriteDimensions, m_position + .5f*m_config.m_spriteDimensions);
}

void Button::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	UpdateAnimation();
	if (m_clickBounds.IsPointInside(GetCursorPositionScreenSpace()))
	{
		if (!m_isPressed && g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
		{
			m_isPressed = true;
			m_texture = m_config.m_pressedSprite;
		}

		if (!m_hover)
		{
			m_hover = true;
			PlayAnimation("ButtonHover");
		}
	}
	else if (m_hover)
	{
		m_hover = false;
		PlayAnimation("ButtonReset");
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_LEFT_MOUSE))
	{
		m_isPressed = false;
		m_texture = m_config.m_idleSprite;
		if (m_clickBounds.IsPointInside(GetCursorPositionScreenSpace()))
		{
			g_theEventSystem->FireEvent(m_config.m_eventName);
			g_theGame->PlaySound(SOUND_ID_CLICK, SoundType::SFX);
		}
	}
}

Vec2 Button::GetCursorPositionScreenSpace()
{
	return g_theInput->GetCursorNormalizedPosition() * g_theGame->m_screenCamera.GetOrthoDimensions();
}