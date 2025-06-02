#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

SpriteAnimDefinition::SpriteAnimDefinition(const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlaybackType playBackType, Shader const* shader)
	: m_spriteSheet(sheet)
	, m_startSpriteIndex(startSpriteIndex)
	, m_endSpriteIndex(endSpriteIndex)
	, m_duraitonSeconds(durationSeconds)
	, m_playbackType(playBackType)
	, m_shader(shader)
{

}

float SpriteAnimDefinition::GetAnimDurationSeconds() const
{
	return m_duraitonSeconds;
}

Shader const* SpriteAnimDefinition::GetShader()
{
	return m_shader;
}

int SpriteAnimDefinition::GetNumFrames()
{
	return (m_endSpriteIndex - m_startSpriteIndex) + 1;
}

const SpriteDefinition& SpriteAnimDefinition::GetSpriteDefAtTime(float seconds) const
{
	if (m_duraitonSeconds == 0.f)
	{
		return m_spriteSheet.GetSpriteDef(0);
	}

	float t = seconds / m_duraitonSeconds;
	if (t > 1.f)
	{
		if (m_playbackType == SpriteAnimPlaybackType::LOOP)
		{
			
			t = t - (float)RoundDownToInt(t);
		}
		else if (m_playbackType == SpriteAnimPlaybackType::PINGPONG)
		{
			int repeatNum = RoundDownToInt(t);
			if (repeatNum % 2 == 0)
			{
				t = t - (float)repeatNum;
			}
			else
			{
				float remainder = t - (float)repeatNum;
				t = 1 - remainder;
			}
		}
		else
		{
			t = 1.f;
		}
	}

	int currSpriteIdx = (int)( roundf(Lerp( (float)m_startSpriteIndex, (float)m_endSpriteIndex, t) ) );
	return m_spriteSheet.GetSpriteDef(currSpriteIdx);
}
