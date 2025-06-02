#pragma once
class SpriteSheet;
class SpriteDefinition;
class Shader;

enum class SpriteAnimPlaybackType
{
	ONCE,		//for 5 frame anim plays 0, 1, 2, 3, 4, 4, 4...
	LOOP,		//for 5 frame anim plays 0, 1, 2, 3, 4, 0, 1, 2, 3, 4...
	PINGPONG	//for 5 frame anim plays 0, 1, 2, 3, 4, 3, 2, 1, 0, 1...
};

class SpriteAnimDefinition
{
public:
	SpriteAnimDefinition( const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, 
		float durationSeconds, SpriteAnimPlaybackType playBackType=SpriteAnimPlaybackType::LOOP, Shader const* shader = nullptr);

	int GetNumFrames();
	float GetAnimDurationSeconds() const;
	Shader const* GetShader();
	const SpriteDefinition& GetSpriteDefAtTime( float seconds ) const;

private:
	const SpriteSheet& m_spriteSheet;
	int m_startSpriteIndex = 0;
	int m_endSpriteIndex = 0;
	float m_duraitonSeconds = 0;
	Shader const* m_shader = nullptr;
	SpriteAnimPlaybackType m_playbackType = SpriteAnimPlaybackType::LOOP;
};