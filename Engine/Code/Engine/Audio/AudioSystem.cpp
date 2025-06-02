#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/DevConsole.hpp"

//-----------------------------------------------------------------------------------------------
// To disable audio entirely (and remove requirement for fmod.dll / fmod64.dll) for any game,
//	#define ENGINE_DISABLE_AUDIO in your game's Code/Game/EngineBuildPreferences.hpp file.
//
// Note that this #include is an exception to the rule "engine code doesn't know about game code".
//	Purpose: Each game can now direct the engine via #defines to build differently for that game.
//	Downside: ALL games must now have this Code/Game/EngineBuildPreferences.hpp file.
//
// SD1 NOTE: THIS MEANS *EVERY* GAME MUST HAVE AN EngineBuildPreferences.hpp FILE IN ITS CODE/GAME FOLDER!!
#include "Game/EngineBuildPreferences.hpp"
#if !defined( ENGINE_DISABLE_AUDIO )


//-----------------------------------------------------------------------------------------------
// Link in the appropriate FMOD static library (32-bit or 64-bit)
//
#if defined( _WIN64 )
#pragma comment( lib, "ThirdParty/fmod/fmod64_vc.lib" )
#else
#pragma comment( lib, "ThirdParty/fmod/fmod_vc.lib" )
#endif


//-----------------------------------------------------------------------------------------------
// Initialization code based on example from "FMOD Studio Programmers API for Windows"
//
AudioSystem::AudioSystem(AudioConfig& config)
	: m_fmodSystem( nullptr )
	, m_config(config)
{
}


//-----------------------------------------------------------------------------------------------
AudioSystem::~AudioSystem()
{
}


//------------------------------------------------------------------------------------------------
void AudioSystem::StartUp()
{
	FMOD_RESULT result;
	result = FMOD::System_Create( &m_fmodSystem );
	ValidateResult( result );

	result = m_fmodSystem->init( 512, FMOD_INIT_3D_RIGHTHANDED, nullptr );
	ValidateResult( result );
}


//------------------------------------------------------------------------------------------------
void AudioSystem::Shutdown()
{
	FMOD_RESULT result = m_fmodSystem->release();
	ValidateResult( result );

	m_fmodSystem = nullptr; // #Fixme: do we delete/free the object also, or just do this?
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::BeginFrame()
{
	m_fmodSystem->update();
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::EndFrame()
{
}


//-----------------------------------------------------------------------------------------------
SoundID AudioSystem::CreateOrGetSound( const std::string& soundFilePath, bool isSound3D)
{
	std::map< std::string, SoundID >::iterator found = m_registeredSoundIDs.find( soundFilePath );
	if( found != m_registeredSoundIDs.end() )
	{
		return found->second;
	}
	else
	{
		FMOD::Sound* newSound = nullptr;
		FMOD_MODE modeFlags = FMOD_DEFAULT;
		if (isSound3D)
		{
			modeFlags = FMOD_3D;
		}
		m_fmodSystem->createSound( soundFilePath.c_str(), modeFlags, nullptr, &newSound );
		if( newSound )
		{
			SoundID newSoundID = m_registeredSounds.size();
			m_registeredSoundIDs[ soundFilePath ] = newSoundID;
			m_registeredSounds.push_back( newSound );
			return newSoundID;
		}
	}

	return MISSING_SOUND_ID;
}


//-----------------------------------------------------------------------------------------------
SoundPlaybackID AudioSystem::StartSound( SoundID soundID, bool isLooped, float volume, float balance, float speed, bool isPaused, float positionOffset, bool sampleFFt )
{
	size_t numSounds = m_registeredSounds.size();
	if( soundID < 0 || soundID >= numSounds )
		return MISSING_SOUND_ID;

	FMOD::Sound* sound = m_registeredSounds[ soundID ];
	if( !sound )
		return MISSING_SOUND_ID;

	FMOD::Channel* channelAssignedToSound = nullptr;
	m_fmodSystem->playSound( sound, nullptr, isPaused, &channelAssignedToSound );
	if( channelAssignedToSound )
	{
		int loopCount = isLooped ? -1 : 0;
		unsigned int playbackMode = isLooped ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		float frequency;
		channelAssignedToSound->setMode(playbackMode);
		channelAssignedToSound->getFrequency( &frequency );
		channelAssignedToSound->setFrequency( frequency * speed );
		channelAssignedToSound->setVolume( volume );
		channelAssignedToSound->setPan( balance );
		channelAssignedToSound->setLoopCount( loopCount );
		if (positionOffset > 0)
		{
			channelAssignedToSound->setPosition((unsigned int)(positionOffset * 1000.f), FMOD_TIMEUNIT_MS);
		}
		if (sampleFFt)
		{
			FMOD::DSP* dsp = nullptr;
			m_fmodSystem->createDSPByType(FMOD_DSP_TYPE_FFT, &dsp);
			dsp->setActive(true);
			channelAssignedToSound->addDSP(0, dsp);
		}
	}

	return (SoundPlaybackID) channelAssignedToSound;
}

void AudioSystem::CreateFFTForSound(SoundPlaybackID soundID, int& numSamples, float*& fftBuffer)
{
	FMOD::Channel* soundChannel = (FMOD::Channel*)soundID;
	FMOD_DSP_PARAMETER_FFT* fftData = nullptr;
	FMOD::DSP* dsp = nullptr;
	soundChannel->getDSP(0, &dsp);
	dsp->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void**)&fftData, 0, 0, 0);
	if (fftData)
	{
		numSamples = fftData->length;
		fftBuffer = fftData->spectrum[0];
	}
}

float AudioSystem::GetDominantFreqForSound(SoundPlaybackID soundPlayback)
{
	FMOD::Channel* soundChannel = (FMOD::Channel*)soundPlayback;
	FMOD::DSP* dsp = nullptr;
	soundChannel->getDSP(0, &dsp);
	float dominantFreq = 0.f;
	dsp->getParameterFloat(FMOD_DSP_FFT_DOMINANT_FREQ, &dominantFreq, 0, 0);
	return dominantFreq;
}

float AudioSystem::GetSoundPlaybackPosition(SoundPlaybackID soundPlayback)
{
	FMOD::Channel* soundChannel = (FMOD::Channel*)soundPlayback;
	unsigned int soundPosMS = 0;
	soundChannel->getPosition(&soundPosMS, FMOD_TIMEUNIT_MS);
	float soundPositionSeconds = ((float)soundPosMS) / 1000.f;
	return soundPositionSeconds;
}

float AudioSystem::GetSoundLength(SoundID soundID)
{
	FMOD::Sound* sound = m_registeredSounds[(int)soundID];
	unsigned int soundLengthMS = 0;
	sound->getLength(&soundLengthMS, FMOD_TIMEUNIT_MS);
	return ((float)soundLengthMS) / 1000.f;
}

SoundPlaybackID AudioSystem::StartSoundAt(SoundID soundID, Vec3 soundPos, bool isLooped, float volume, float balance, float speed, bool isPaused)
{
	size_t numSounds = m_registeredSounds.size();
	if (soundID < 0 || soundID >= numSounds)
		return MISSING_SOUND_ID;

	FMOD::Sound* sound = m_registeredSounds[soundID];
	if (!sound)
		return MISSING_SOUND_ID;

	FMOD::Channel* channelAssignedToSound = nullptr;
	m_fmodSystem->playSound(sound, nullptr, isPaused, &channelAssignedToSound);

	if (channelAssignedToSound)
	{
		FMOD_VECTOR fmodPos = ConverGameToFModPos(soundPos);
		FMOD_VECTOR fmodVel;
		fmodVel.x = 0.f;
		fmodVel.y = 0.f;
		fmodVel.z = 0.f;
		channelAssignedToSound->set3DAttributes(&fmodPos, &fmodVel);

		int loopCount = isLooped ? -1 : 0;
		unsigned int playbackMode = isLooped ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		playbackMode |= FMOD_3D;
		float frequency;
		channelAssignedToSound->setMode(playbackMode);
		channelAssignedToSound->getFrequency(&frequency);
		channelAssignedToSound->setFrequency(frequency * speed);
		channelAssignedToSound->setVolume(volume);
		channelAssignedToSound->setPan(balance);
		channelAssignedToSound->setLoopCount(loopCount);
	}

	return (SoundPlaybackID)channelAssignedToSound;
}

void AudioSystem::SetSoundPosition(SoundPlaybackID sound, Vec3 soundPos)
{
	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*)sound;
	bool playing = false;
	channelAssignedToSound->isPlaying(&playing);

	if (channelAssignedToSound != nullptr && playing)
	{
		FMOD_VECTOR  fmodPos = ConverGameToFModPos(soundPos);
		FMOD_VECTOR fmodVel;
		fmodVel.x = 0.f;
		fmodVel.y = 0.f;
		fmodVel.z = 0.f;
		channelAssignedToSound->set3DAttributes(&fmodPos, &fmodVel);
	}
}

void AudioSystem::SetNumListeners(int newNumListeners)
{
	m_fmodSystem->set3DNumListeners(newNumListeners);
}

int AudioSystem::GetNumListeners()
{
	int numListeners = 0;
	m_fmodSystem->get3DNumListeners(&numListeners);
	return numListeners;
}

void AudioSystem::UpdateListener(int listenerIdx, Vec3 const& listenerPos, Vec3 const& listenerForward, Vec3 const& listenerUp)
{
	FMOD_VECTOR fmodPos = ConverGameToFModPos(listenerPos);
	FMOD_VECTOR fmodForward = ConverGameToFModPos(listenerForward);
	FMOD_VECTOR fmodUp = ConverGameToFModPos(listenerUp);
	FMOD_VECTOR fmodVel;
	fmodVel.x = 0.f;
	fmodVel.y = 0.f;
	fmodVel.z = 0.f;
	m_fmodSystem->set3DListenerAttributes(listenerIdx, &fmodPos, &fmodVel, &fmodForward, &fmodUp);

	FMOD_VECTOR outPos;
	FMOD_VECTOR outForward;
	FMOD_VECTOR outUp;
	FMOD_VECTOR outVel;

	m_fmodSystem->get3DListenerAttributes(listenerIdx, &outPos, &outVel, &outForward, &outUp);
}

//-----------------------------------------------------------------------------------------------
void AudioSystem::StopSound( SoundPlaybackID soundPlaybackID )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to stop sound on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	channelAssignedToSound->stop();
}


//-----------------------------------------------------------------------------------------------
// Volume is in [0,1]
//
void AudioSystem::SetSoundPlaybackVolume( SoundPlaybackID soundPlaybackID, float volume )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set volume on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	channelAssignedToSound->setVolume( volume );
}


//-----------------------------------------------------------------------------------------------
// Balance is in [-1,1], where 0 is L/R centered
//
void AudioSystem::SetSoundPlaybackBalance( SoundPlaybackID soundPlaybackID, float balance )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set balance on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	channelAssignedToSound->setPan( balance );
}


//-----------------------------------------------------------------------------------------------
// Speed is frequency multiplier (1.0 == normal)
//	A speed of 2.0 gives 2x frequency, i.e. exactly one octave higher
//	A speed of 0.5 gives 1/2 frequency, i.e. exactly one octave lower
//
void AudioSystem::SetSoundPlaybackSpeed( SoundPlaybackID soundPlaybackID, float speed )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set speed on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	float frequency;
	FMOD::Sound* currentSound = nullptr;
	channelAssignedToSound->getCurrentSound( &currentSound );
	if( !currentSound )
		return;

	int ignored = 0;
	currentSound->getDefaults( &frequency, &ignored );
	channelAssignedToSound->setFrequency( frequency * speed );
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::ValidateResult( FMOD_RESULT result )
{
	if( result != FMOD_OK )
	{
		ERROR_RECOVERABLE( Stringf( "Engine/Audio SYSTEM ERROR: Got error result code %i - error codes listed in fmod_common.h\n", (int) result ) );
	}
}

float AudioSystem::GetSoundFrequency(SoundPlaybackID soundPlaybackID)
{
	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*)soundPlaybackID;
	float outFreq = 0.f;
	channelAssignedToSound->getFrequency(&outFreq);
	return outFreq;
}



#endif // !defined( ENGINE_DISABLE_AUDIO )

FMOD_VECTOR ConverGameToFModPos(Vec3 const& gamePos)
{
	FMOD_VECTOR fModPos;
	fModPos.x = -gamePos.y;
	fModPos.y = gamePos.z;
	fModPos.z = -gamePos.x;
	return fModPos;
}
