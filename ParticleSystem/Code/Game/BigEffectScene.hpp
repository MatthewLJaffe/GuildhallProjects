#pragma once
#include "Game/Scene.hpp"

class BigEffectScene : public Scene
{
	virtual void Update(float deltaSeconds) override;
	virtual void StartUp() override;
	virtual void SwitchOn() override;
	virtual void SwitchOff() override;

	Timer m_beatTimer;
	bool m_noiseOn = false;
	SoundPlaybackID m_musicPlayback;
	SoundID m_music = MISSING_SOUND_ID;
};