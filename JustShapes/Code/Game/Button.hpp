#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/AABB2.hpp"

class Texture;


struct ButtonConfig
{
	Texture* m_idleSprite = nullptr;
	Texture* m_pressedSprite = nullptr;
	SoundID m_pressedNoise = MISSING_SOUND_ID;
	std::string m_eventName;
	std::string m_text;
	Vec2 m_clickDimensions = Vec2::ZERO;
	Vec2 m_spriteDimensions = Vec2::ZERO;
	float m_cellHeight = 10.f;
};

class Button : public Entity
{
public:
	Button(GameState* gameState, EntityType entityType, Vec2 const& startPos, ButtonConfig config);
	void Update(float deltaSeconds) override;
	ButtonConfig m_config;
	Vec2 GetCursorPositionScreenSpace();
private:
	bool m_isPressed = false;
	bool m_hover = false;
	AABB2 m_clickBounds = AABB2::ZERO_TO_ONE;
};

