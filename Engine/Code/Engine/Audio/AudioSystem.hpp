#pragma once


//-----------------------------------------------------------------------------------------------
#include "ThirdParty/fmod/fmod.hpp"
#include <string>
#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
typedef size_t SoundID;
typedef size_t SoundPlaybackID;
constexpr size_t MISSING_SOUND_ID = (size_t)(-1); // for bad SoundIDs and SoundPlaybackIDs
struct Vec3;


//-----------------------------------------------------------------------------------------------
class AudioSystem;


/////////////////////////////////////////////////////////////////////////////////////////////////
struct AudioConfig
{

};

class AudioSystem
{
public:
	AudioSystem(AudioConfig& audioConfig);
	virtual ~AudioSystem();

public:
	void						StartUp();
	void						Shutdown();
	virtual void				BeginFrame();
	virtual void				EndFrame();

	virtual SoundID				CreateOrGetSound( const std::string& soundFilePath, bool isSound3D = false);
	virtual SoundPlaybackID		StartSound( SoundID soundID, bool isLooped=false, float volume=1.f, float balance=0.0f, float speed=1.0f, bool isPaused=false, float startOffset=0, bool sampleFFT = false);
	virtual SoundPlaybackID		StartSoundAt(SoundID soundID, Vec3 soundPos, bool isLooped = false, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false);
	void						SetSoundPosition(SoundPlaybackID sound, Vec3 soundPos);
	void						SetNumListeners(int newNumListeners);
	int							GetNumListeners();
	void						UpdateListener(int listenerIdx, Vec3 const& listenerPos, Vec3 const& listenerForward, Vec3 const& listenerUp);

	virtual void				StopSound( SoundPlaybackID soundPlaybackID );
	virtual void				SetSoundPlaybackVolume( SoundPlaybackID soundPlaybackID, float volume );	// volume is in [0,1]
	virtual void				SetSoundPlaybackBalance( SoundPlaybackID soundPlaybackID, float balance );	// balance is in [-1,1], where 0 is L/R centered
	virtual void				SetSoundPlaybackSpeed( SoundPlaybackID soundPlaybackID, float speed );		// speed is frequency multiplier (1.0 == normal)

	virtual void				ValidateResult( FMOD_RESULT result );
	float						GetSoundFrequency(SoundPlaybackID soundPlaybackID);
	void						CreateFFTForSound(SoundPlaybackID soundPlayback, int& numSamples, float*& fftBuffer);
	float						GetDominantFreqForSound(SoundPlaybackID soundPlayback);
	float						GetSoundPlaybackPosition(SoundPlaybackID soundPlayback);
	float						GetSoundLength(SoundID sound);

protected:
	AudioConfig							m_config;
	FMOD::System*						m_fmodSystem;
	std::map< std::string, SoundID >	m_registeredSoundIDs;
	std::vector< FMOD::Sound* >			m_registeredSounds;
};

FMOD_VECTOR ConverGameToFModPos(Vec3 const& gamePos);

