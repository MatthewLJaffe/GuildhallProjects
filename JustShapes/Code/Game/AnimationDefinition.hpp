#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Math/FloatCurve.hpp"

enum class PlaybackMode
{
	ONCE,
	LOOP,
	COUNT
};

enum class EasingMode
{
	LINEAR,
	SMOOTH_START_2,
	SMOOTH_START_3,
	SMOOTH_START_4,
	SMOOTH_START_5,
	SMOOTH_START_6,
	SMOOTH_STOP_2,
	SMOOTH_STOP_3,
	SMOOTH_STOP_4,
	SMOOTH_STOP_5,
	SMOOTH_STOP_6,
	SMOOTH_STEP_3,
	SMOOTH_STEP_5,
	HESITATE_3,
	HESITATE_5,
	CUSTOM
};

struct Keyframe
{
	float m_time = 0.f;
	EasingMode m_easingMode = EasingMode::LINEAR;
	FloatCurve* m_easingCurve = nullptr;
	Vec2 m_localPosition = Vec2::ZERO;
	float m_rotation = 0.f;
	Vec2 m_scale = Vec2::ONE;
	Rgba8 m_color = Rgba8::WHITE;
	IntVec2 m_spriteCoords = IntVec2(-1, -1);
	Vec2 m_spriteDimensions = Vec2::ZERO;
	float GetEasingOutput(float normalizedT) const;

	bool m_changeLocalPosition = false;
	bool m_changeRotation = false;
	bool m_changeScale = false;
	bool m_changeColor = false;
	bool m_changeSprite = false;
};

struct Track
{
	std::vector<Keyframe> m_keyframes;
	SpriteSheet* m_spriteSheet = nullptr;
	bool m_interpolateSprites = true;
};

class AnimationDefinition
{
public:
	std::string m_name;
public:
	static void InitializeDefinitions(const char* path);
	static void ClearDefinitions();
	static AnimationDefinition const* GetByName(std::string const& name);
	bool LoadFromXmlElement(XmlElement const& element);
	float GetAnimationLength() const;
	std::vector<Track> m_tracks;
	PlaybackMode m_playbackMode = PlaybackMode::ONCE;

private:
	void ParseTracks(XmlElement const& element);
	PlaybackMode ParsePlaybackMode(XmlElement const& element);
	EasingMode ParseEasingMode(XmlElement const& element);
	static std::vector<AnimationDefinition const*> s_animationDefinitions;
};