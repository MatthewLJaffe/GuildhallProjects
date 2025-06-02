#pragma once
#include "Game/Scene.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorUID.hpp"

class Actor;
class Prop;
class Widget;

enum class TrackLayerType
{
	BASE,
	LASER,
	MISSILE,
	COUNT
};

enum class SpawnType
{
	NEAR,
	FAR,
	RANDOM
};

struct TrackLayer
{
	SoundID m_soundID;
	SoundPlaybackID m_soundPlayback;
	TrackLayerType m_trackType;
	int m_numberOfBeats = 32;
};

struct BeatSound
{
	float m_beatTimeInSeconds;
	SoundID m_sound;
	float m_volume = 1.f;
};

struct UnitRateSpawn
{
	float m_chance = 0.f;
	std::string m_name = "";
	SpawnType m_spawnType;
};

struct UnitBurstSpawn
{
	int m_number = 0;
	std::string m_name = "";
	SpawnType m_spawnType;
};

struct UnitSpawner
{
	int m_beat = 0;
	float m_unitsPerBeat = -1;
	std::vector<UnitRateSpawn> m_rateSpawner;
	std::vector<UnitBurstSpawn> m_burstSpawner;
	SoundID m_trackSwitch = MISSING_SOUND_ID;
	int m_trackSwitchBeats = 32;
};

class GameScene : public Scene
{
public:
	
	void StartUp() override;
	void Update(float deltaSeconds) override;
	void Render() const override;
	void UpdateBeatTimer();
	void UpdateBeatUI();
	Actor& SpawnActor(SpawnInfo const& spawnInfo);
	Actor* ConstructNewActor(SpawnInfo const& spawnInfo, ActorUID actorUID);
	void RemoveActor(Actor* actorToRemove);
	void AddActorToRelevantLists(Actor* actorToAdd);
	void AddActorToList(Actor* actorToAdd, std::vector<Actor*>& actorList);
	void RemoveActorFromList(Actor* actorToRemove, std::vector<Actor*>& actorList);
	void DeleteDeadActors();
	void UpdateActorCollision(float deltaSeconds);
	void ResolveActorCollision(Actor* actor1, Actor* actor2);
	void SpawnBob();

	void SpawnBigAsteroid();
	void SpawnSmallAsteroid();
	void SpawnMissileAmmo();
	void SpawnMegaLaserAmmo();

	void LoseGame();
	void WinGame();


	Actor* GetActorByUID(const ActorUID uid);
	void SetTrackLayerVolume(TrackLayerType trackToSet, float newVolume);
	SoundPlaybackID GetTrackLayerPlaybackID(TrackLayerType trackType);
	bool IsTimeAtBeatFraction(float beatFractionInSeconds);
	void QueueBeatSound(BeatSound const& beatSound);
	void PlayBeatSounds();
	float GetCurrentTimeFromLastBeat() const;
	void RespawnPlayer();
	void RenderGameUI() const;
	void AddGameWidgets();
	void ShutDown() override;
	std::vector<Actor*> m_damageableActors;
	std::vector<Actor*> m_collisionActors;
	std::vector<Actor*> m_allActors;
	std::vector<Actor*> m_newlySpawnedActors;
	std::vector<Actor*> m_thralls;
	std::vector<Actor*> m_obstacles;

	std::vector<Widget*> m_allWidgets;

	float m_evenBeatRadius = 20.f;
	float m_oddBeatRadius = 20.f;
	Rgba8 m_evenBeatColor;
	Rgba8 m_oddBeatColor;

	float m_offBeatTimeAllowed = .1f;
	FloatRange m_beatRadius  = FloatRange(20.f, 60.f);
	float m_innerCrosshairAlpha = 0.f;

	bool m_isFrameBeat = false;
	bool m_isFrameOnBeatToPlayer = false;
	int m_currentBeats = 0;
	float m_offsetInMS = 42.f;
	float m_currentTrackPlaybackTime = 0;
	float m_prevFrameTrackPlaybackTime = 0.f;
	float m_mainTrackLength = 0.f;
	unsigned int m_salt = 1;
	Prop* m_debugGrid = nullptr;
	Widget* m_healthBarWidget = nullptr;
	Widget* m_energyBarWidget = nullptr;

	bool m_demoStarted = true;

	//spawner
	void LoadEnemySpawnerFromXML(std::string const& enemySpawnerFilePath);
	void UpdateEnemySpawner();
	void SpawnUnitFromCurrentRate();
	std::vector<UnitSpawner> m_unitSpawners;
	float m_owedUnitsThisBeat = 0.f;
	float m_currentUnitSpawnRate = 0.f;
	std::vector<UnitRateSpawn> m_currentUnitRateSpawner;

private:
	std::vector<TrackLayer> m_trackLayers;
	std::vector<BeatSound> m_queuedOnBeatSounds;
	void ActivateNewlySpawnedActors();
	void RandomActorSpawnNear(std::string const& actorName);
	void RandomActorSpawnFar(std::string const& actorName);
	void RandomActorSpawn(std::string const& actorName);

	void StopAllAudio();
	SpawnType GetSpawnTypeFromStr(std::string const& spawnType);
	void SpawnActorWithSpawnType(std::string const& unitName, SpawnType unitSpawnType);
	//debug texture
	Texture* m_debugSkyboxTexture = nullptr;
};