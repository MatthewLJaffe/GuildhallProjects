#include "Game/GameCommon.hpp"
#include "Game/AnimationDefinition.hpp"

std::vector<AnimationDefinition const*> AnimationDefinition::s_animationDefinitions;

void AnimationDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument gameConfigDocument;
	GUARANTEE_OR_DIE(gameConfigDocument.LoadFile(path) == 0, Stringf("Failed to load %s", path));
	XmlElement* rootElement = gameConfigDocument.FirstChildElement("AnimationDefinitions");
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root GameConfig element from %s", path));
	for (XmlElement const* currElement = rootElement->FirstChildElement("AnimationDefinition"); currElement != nullptr; currElement = currElement->NextSiblingElement("AnimationDefinition"))
	{
		AnimationDefinition* animDef = new AnimationDefinition();
		animDef->LoadFromXmlElement(*currElement);
		s_animationDefinitions.push_back(animDef);
	}
}

void AnimationDefinition::ClearDefinitions()
{
	for (int i = 0; i < (int)s_animationDefinitions.size(); i++)
	{
		delete s_animationDefinitions[i];
		s_animationDefinitions[i] = nullptr;
	}
	s_animationDefinitions.clear();
}

AnimationDefinition const* AnimationDefinition::GetByName(std::string const& name)
{
	for (int i = 0; i < (int)s_animationDefinitions.size(); i++)
	{
		if (s_animationDefinitions[i]->m_name == name)
		{
			return s_animationDefinitions[i];
		}
	}
	return nullptr;
}

bool AnimationDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", "NO NAME");
	m_playbackMode = ParsePlaybackMode(element);
	ParseTracks(element);
	return true;
}

float AnimationDefinition::GetAnimationLength() const
{
	float length = 0.f;
	for (int i = 0; i < m_tracks.size(); i++)
	{
		float trackLength = m_tracks[i].m_keyframes[m_tracks[i].m_keyframes.size() - 1].m_time;
		if (trackLength > length)
		{
			length = trackLength;
		}
	}
	return length;
}

void AnimationDefinition::ParseTracks(XmlElement const& element)
{
	for (XmlElement const* currTrack = element.FirstChildElement("Track"); currTrack != nullptr; currTrack = currTrack->NextSiblingElement("Track"))
	{
		Track track;
		std::string spriteSheetPath = ParseXmlAttribute(*currTrack, "spriteSheet", "");
		if (spriteSheetPath != "")
		{
			IntVec2 spriteSheetDimensions = ParseXmlAttribute(*currTrack, "spriteSheetDimensions", IntVec2::ZERO);
			if (spriteSheetDimensions != IntVec2::ZERO)
			{
				track.m_spriteSheet = g_theRenderer->CreateOrGetSpriteSheetFromFile(spriteSheetPath.c_str(), spriteSheetDimensions);
			}
			else
			{
				ERROR_RECOVERABLE(Stringf("Did not specify spriteSheetDimensions for Animation %s", m_name.c_str()));
			}
		}
		track.m_interpolateSprites = ParseXmlAttribute(*currTrack, "interpolateSprites", true);
		for (XmlElement const* currKeyframe = currTrack->FirstChildElement("Keyframe"); currKeyframe != nullptr; currKeyframe = currKeyframe->NextSiblingElement("Keyframe"))
		{
			Keyframe keyFrame;
			keyFrame.m_time = ParseXmlAttribute(*currKeyframe, "time", 0.f);
			keyFrame.m_easingMode = ParseEasingMode(*currKeyframe);
			if (keyFrame.m_easingMode == EasingMode::CUSTOM)
			{
				keyFrame.m_easingCurve = FloatCurve::GetFloatCurveFromName(ParseXmlAttribute(*currKeyframe, "easingMode", ""));
			}

			Vec2 notFoundVector(-999.f, -999.f);
			keyFrame.m_localPosition = ParseXmlAttribute(*currKeyframe, "localPosition", notFoundVector);
			if (keyFrame.m_localPosition != notFoundVector)
			{
				keyFrame.m_changeLocalPosition = true;
			}

			float notFoundFloat = -9999999.f;
			keyFrame.m_rotation = ParseXmlAttribute(*currKeyframe, "rotation", notFoundFloat);
			if (keyFrame.m_rotation != notFoundFloat)
			{
				keyFrame.m_changeRotation = true;
			}

			keyFrame.m_scale = ParseXmlAttribute(*currKeyframe, "scale", notFoundVector);
			if (keyFrame.m_scale != notFoundVector)
			{
				keyFrame.m_changeScale = true;
			}

			Rgba8 notFoundColor = Rgba8::MAGENTA;
			keyFrame.m_color = ParseXmlAttribute(*currKeyframe, "color", notFoundColor);
			if (keyFrame.m_color == notFoundColor)
			{
				keyFrame.m_changeColor = false;
			}
			else
			{
				keyFrame.m_changeColor = true;
			}

			IntVec2 notFoundSpriteCoords(-1, -1);
			keyFrame.m_spriteCoords = ParseXmlAttribute(*currKeyframe, "spriteCoords", notFoundSpriteCoords);
			if (keyFrame.m_spriteCoords != notFoundSpriteCoords && track.m_spriteSheet != nullptr)
			{
				keyFrame.m_changeSprite = true;
			}
			track.m_keyframes.push_back(keyFrame);
		}
		m_tracks.push_back(track);
	}
}

PlaybackMode AnimationDefinition::ParsePlaybackMode(XmlElement const& element)
{
	std::string mode = ParseXmlAttribute(element, "playbackMode", "");
	if (mode == "Once")
	{
		return PlaybackMode::ONCE;
	}
	else if (mode == "Loop")
	{
		return PlaybackMode::LOOP;
	}
	return PlaybackMode::ONCE;
}

EasingMode AnimationDefinition::ParseEasingMode(XmlElement const& element)
{
	std::string mode = ParseXmlAttribute(element, "easingMode", "");
	if (mode == "")
	{
		return EasingMode::LINEAR;
	}
	if (mode == "SmoothStart2")
	{
		return EasingMode::SMOOTH_START_2;
	}
	if (mode == "SmoothStart3")
	{
		return EasingMode::SMOOTH_START_3;
	}
	if (mode == "SmoothStart4")
	{
		return EasingMode::SMOOTH_START_4;
	}
	if (mode == "SmoothStart5")
	{
		return EasingMode::SMOOTH_START_5;
	}
	if (mode == "SmoothStart6")
	{
		return EasingMode::SMOOTH_START_6;
	}

	if (mode == "SmoothStop2")
	{
		return EasingMode::SMOOTH_STOP_2;
	}
	if (mode == "SmoothStop3")
	{
		return EasingMode::SMOOTH_STOP_3;
	}
	if (mode == "SmoothStop4")
	{
		return EasingMode::SMOOTH_STOP_4;
	}
	if (mode == "SmoothStop5")
	{
		return EasingMode::SMOOTH_STOP_5;
	}
	if (mode == "SmoothStop6")
	{
		return EasingMode::SMOOTH_STOP_6;
	}

	if (mode == "SmoothStep3")
	{
		return EasingMode::SMOOTH_STEP_3;
	}
	if (mode == "SmoothStep5")
	{
		return EasingMode::SMOOTH_STEP_5;
	}

	if (mode == "Hesitate3")
	{
		return EasingMode::HESITATE_3;
	}
	if (mode == "Hesitate5")
	{
		return EasingMode::HESITATE_5;
	}

	if (mode == "Linear")
	{
		return EasingMode::LINEAR;
	}

	return EasingMode::CUSTOM;
}

float Keyframe::GetEasingOutput(float normalizedT) const
{
	switch (m_easingMode)
	{
	case EasingMode::CUSTOM:
		return m_easingCurve->EvaluateAt(normalizedT);
	case EasingMode::LINEAR:
		return normalizedT;
	case EasingMode::HESITATE_3:
		return Hesitate3(normalizedT);
	case EasingMode::HESITATE_5:
		return Hesitate5(normalizedT);

	case EasingMode::SMOOTH_START_2:
		return SmoothStart2(normalizedT);
	case EasingMode::SMOOTH_START_3:
		return SmoothStart3(normalizedT);
	case EasingMode::SMOOTH_START_4:
		return SmoothStart4(normalizedT);
	case EasingMode::SMOOTH_START_5:
		return SmoothStart5(normalizedT);
	case EasingMode::SMOOTH_START_6:
		return SmoothStart6(normalizedT);

	case EasingMode::SMOOTH_STEP_3:
		return SmoothStep3(normalizedT);
	case EasingMode::SMOOTH_STEP_5:
		return SmoothStep5(normalizedT);

	case EasingMode::SMOOTH_STOP_2:
		return SmoothStop2(normalizedT);
	case EasingMode::SMOOTH_STOP_3:
		return SmoothStop3(normalizedT);
	case EasingMode::SMOOTH_STOP_4:
		return SmoothStop4(normalizedT);
	case EasingMode::SMOOTH_STOP_5:
		return SmoothStop5(normalizedT);
	case EasingMode::SMOOTH_STOP_6:
		return SmoothStop6(normalizedT);
	}
	return normalizedT;
}
