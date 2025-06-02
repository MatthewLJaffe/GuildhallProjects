#include "Game/GameScene.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Game/Model.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Prop.hpp"
#include "Game/ParticleEntity.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Game/Actor.hpp"
#include "Game/Thrall.hpp"
#include "Game/Widget.hpp"
#include "Game/EndScene.hpp"

void GameScene::StartUp()
{
	if (m_startedUp)
	{
		return;
	}
	m_debugSkyboxTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/cubemapNoSun.png");
	LoadEnemySpawnerFromXML("Data/Definitions/EnemySpawner.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/BobShipExplosion.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/SmallEnergyExplosion.xml");

	bool useFastDefinitions = g_gameConfigBlackboard.GetValue("useFastDefinitions", false);
	if (useFastDefinitions) 
	{
		ActorDefinition::LoadActorDefinitionsFromXML("Data/Definitions/ActorDefinitions_Fast.xml");
	}
	else
	{
		ActorDefinition::LoadActorDefinitionsFromXML("Data/Definitions/ActorDefinitions.xml");
	}

	SpawnInfo playerShipSpawn(Vec3::ZERO, Mat44(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName("Player"));
	g_theGame->m_playerShip = dynamic_cast<PlayerShip*>(&SpawnActor(playerShipSpawn));
	g_theParticleSystem->m_config.m_playerCamera = &g_theGame->m_playerShip->m_camera;
	//m_debugGrid = new Prop(Vec3::ZERO);
	//m_debugGrid->CreateGrid();

	for (int i = 0; i < 25; i++)
	{
		SpawnBigAsteroid();
	}
	for (int i = 0; i < 500; i++)
	{
		SpawnSmallAsteroid();
	}

	/*
	for (int i = 0; i < 30; i++)
	{
		SpawnMissileAmmo();
		SpawnMegaLaserAmmo();
	}
	*/
	/*
	SpawnInfo spawnInfo(Vec3(10.f, 0.f, 0.f), Mat44(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName("Battery"));
	SpawnActor(spawnInfo);

	SpawnInfo missileSpawnInfo(Vec3(10.f, 0.f, 5.f), Mat44(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName("MissilePickup"));
	SpawnActor(missileSpawnInfo);

	SpawnInfo healthSpawnInfo(Vec3(10.f, 0.f, 10.f), Mat44(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName("HealthPickup"));
	SpawnActor(healthSpawnInfo);
	*/

	TrackLayer base;
	base.m_soundID = g_theAudio->CreateOrGetSound("Data/Audio/Tracks/SpaceJamIntro-2.mp3");
	base.m_trackType = TrackLayerType::BASE;
	m_mainTrackLength = g_theAudio->GetSoundLength(base.m_soundID) / BEAT_SPEED_MULT;
	base.m_numberOfBeats = (int)(m_mainTrackLength / BEAT_TIME);
	m_trackLayers.push_back(base);

	TrackLayer laser;
	laser.m_soundID = g_theAudio->CreateOrGetSound("Data/Audio/LaserTrackV1Shortened.mp3");
	laser.m_trackType = TrackLayerType::LASER;
	m_trackLayers.push_back(laser);

	TrackLayer missile;
	missile.m_soundID = g_theAudio->CreateOrGetSound("Data/Audio/Tracks/RocketsTrack-2.mp3");
	missile.m_trackType = TrackLayerType::MISSILE;
	m_trackLayers.push_back(missile);

	for (int i = 0; i < (int)TrackLayerType::COUNT; i++)
	{
		m_trackLayers[i].m_soundPlayback = g_theAudio->StartSound(m_trackLayers[i].m_soundID, true, 0.f, .0f, BEAT_SPEED_MULT);
	}
	SetTrackLayerVolume(TrackLayerType::BASE, 1.f);

	AddGameWidgets();
	m_startedUp = true;
}

void GameScene::Update(float deltaSeconds)
{
	
	if (m_isFrameBeat && m_demoStarted)
	{
		UpdateEnemySpawner();
	}
	UpdateBeatTimer();

	if (g_theInput->WasKeyJustPressed('B'))
	{
		m_isFrameBeat = true;
		m_demoStarted = true;
		m_currentBeats = 0;
		m_prevFrameTrackPlaybackTime = 0.f;
		m_currentTrackPlaybackTime = 0.f;
		g_theAudio->StopSound(m_trackLayers[(int)TrackLayerType::MISSILE].m_soundPlayback);
		m_trackLayers[(int)TrackLayerType::MISSILE].m_soundPlayback = g_theAudio->StartSound(m_trackLayers[(int)TrackLayerType::MISSILE].m_soundID, true, 0.f);

	}
	if (g_theInput->WasKeyJustPressed('R'))
	{
		for (int i = 0; i < 1; i++)
		{
			RandomActorSpawnNear("Artillery");
		}
	}
	if (g_theInput->WasKeyJustPressed('G'))
	{
		for (int i = 0; i < 1; i++)
		{
			RandomActorSpawnNear("Gunner");
		}
	}
	if (g_theInput->WasKeyJustPressed('E'))
	{
		for (int i = 0; i < 1; i++)
		{
			RandomActorSpawnNear("Imperial");
		}
	}
	if (g_theInput->WasKeyJustPressed('I'))
	{
		g_theGame->m_playerShip->m_isInvincible = !g_theGame->m_playerShip->m_isInvincible;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		WinGame();
	}

	ActivateNewlySpawnedActors();
	DeleteDeadActors();
	for (int i = 0; i < m_allActors.size(); i++)
	{
		if (!IsValidActor(m_allActors[i]))
		{
			continue;
		}
		m_allActors[i]->Update(deltaSeconds);
	}
	UpdateActorCollision(deltaSeconds);
	UpdateBeatUI();
	PlayBeatSounds();

	//DebugAddMessage(Stringf("%.2f MS", g_theApp->m_lastFrameTime * 1000.0), 20.f, 0.f);
}

void GameScene::Render() const
{
	Camera* playerCam = g_theGame->GetPlayerCamera();
	g_theRenderer->SetPrimaryRenderTargetType(PrimaryRenderTargetType::HDR);
	//World Space Rendering
	g_theRenderer->BeginCamera(*playerCam);

	//debug draw skybox
	std::vector<Vertex_PCU> skyboxVerts;
	AddVertsForCubeMapSkyBox(skyboxVerts, g_theGame->m_playerShip->GetWorldPosition(), 200.f, Rgba8(150, 150, 150));

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(m_debugSkyboxTexture);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->DrawVertexArray(skyboxVerts.size(), skyboxVerts.data());

	LightConstants lightConstants;
	float sunIntensity = .5f;
	lightConstants.AmbientIntensity = 1.f - sunIntensity;
	lightConstants.SunDirection = g_theGame->m_sunOrientaiton.GetIFwd();
	lightConstants.SunIntensity = sunIntensity;
	lightConstants.worldEyePosition = playerCam->m_position;
	lightConstants.useNormalMapDebugFlag = 0;
	g_theRenderer->SetLightingConstants(lightConstants);

	for (size_t i = 0; i < (int)m_allActors.size(); i++)
	{
		if (!IsValidActor(m_allActors[i]))
		{
			continue;
		}
		m_allActors[i]->Render();
	}

	//ScreenSpace Rendering
	
	g_theParticleSystem->Render();
	g_theRenderer->RenderEmissive();
	g_theRenderer->CompositeHDR();
	g_theRenderer->EndCamera(*playerCam);

	g_theRenderer->BeginCamera(g_theGame->m_screenCamera);
	RenderGameUI();
	g_theRenderer->EndCamera(g_theGame->m_screenCamera);

}

void GameScene::UpdateBeatTimer()
{
	//DebugAddMessage(Stringf("Current Beat %d", m_currentBeats), 20.f, 0.f);
	if (m_currentTrackPlaybackTime == 0.f)
	{
		m_prevFrameTrackPlaybackTime = 0.f;
	}
	else
	{
		m_prevFrameTrackPlaybackTime = m_currentTrackPlaybackTime;
	}
	m_currentTrackPlaybackTime = g_theAudio->GetSoundPlaybackPosition(GetTrackLayerPlaybackID(TrackLayerType::BASE)) / BEAT_SPEED_MULT;
	int currentBeat =  RoundDownToInt(m_currentTrackPlaybackTime / BEAT_TIME);
	int prevBeat = RoundDownToInt(m_prevFrameTrackPlaybackTime / BEAT_TIME);
	if (currentBeat != prevBeat && currentBeat < m_trackLayers[(int)TrackLayerType::BASE].m_numberOfBeats)
	{
		m_isFrameBeat = true;
		m_currentBeats++;
		
	}
	else
	{
		m_isFrameBeat = false;
	}
	float currentBeatTimeToPlayer = m_currentTrackPlaybackTime - (m_offsetInMS / 1000.f);
	float currentBeatsToPlayer = currentBeatTimeToPlayer / BEAT_TIME;
	float beatFractionToPlayer = currentBeatsToPlayer - RoundDownToInt(currentBeatsToPlayer);
	if (beatFractionToPlayer > .5f)
	{
		beatFractionToPlayer = 1.f - beatFractionToPlayer;
	}
	float secondsFromBeatToPlayer = beatFractionToPlayer * BEAT_TIME;
	if (secondsFromBeatToPlayer < m_offBeatTimeAllowed)
	{
		m_isFrameOnBeatToPlayer = true;
	}
	else
	{
		m_isFrameOnBeatToPlayer = false;
	}
}

void GameScene::UpdateBeatUI()
{
	
	float beatPercentage = GetCurrentTimeFromLastBeat() / BEAT_TIME;

	Rgba8 primaryBeatColor;
	float primaryBeatRadius = 0.f;
	Rgba8 secondaryBeatColor;
	float secondaryBeatRadius = 0.f;

	float lagDuration = m_offBeatTimeAllowed * 2.f;

	if (GetCurrentTimeFromLastBeat() < lagDuration)
	{
		secondaryBeatRadius = m_beatRadius.m_min;
		float t = GetCurrentTimeFromLastBeat() / lagDuration;
		secondaryBeatColor = Rgba8::WHITE;
		float alpha = Lerp(1.f, 0.f, t);
		secondaryBeatColor.a = (unsigned char)((float)secondaryBeatColor.a * alpha);
	}
	else
	{
		secondaryBeatColor.a = 0;
	}
	primaryBeatColor = Rgba8::WHITE;
	float primaryAlpha = Clamp(Lerp(0.f, 1.f, beatPercentage * 2.f), 0.f, 1.f);
	primaryBeatColor.a = (unsigned char)((float)primaryBeatColor.a * primaryAlpha);
	primaryBeatRadius = Lerp(m_beatRadius.m_max, m_beatRadius.m_min, beatPercentage);

	if (m_currentBeats % 2 == 0)
	{
		m_evenBeatColor = primaryBeatColor;
		m_evenBeatRadius = primaryBeatRadius;
		m_oddBeatColor = secondaryBeatColor;
		m_oddBeatRadius = secondaryBeatRadius;
	}
	else
	{
		m_oddBeatColor = primaryBeatColor;
		m_oddBeatRadius = primaryBeatRadius;
		m_evenBeatColor = secondaryBeatColor;
		m_evenBeatRadius = secondaryBeatRadius;
	}

	m_healthBarWidget->m_config.m_panelSize.x = (g_theGame->m_playerShip->m_currentHealth / g_theGame->m_playerShip->m_definition->m_health);
	if (m_healthBarWidget->m_config.m_panelSize.x < 0.f)
	{
		m_healthBarWidget->m_config.m_panelSize.x = 0.f;
	}
	m_healthBarWidget->Build();

	m_energyBarWidget->m_config.m_panelSize.y = (g_theGame->m_playerShip->m_primaryCurrentEnergy / g_theGame->m_playerShip->m_primaryMaxEnergy);
	if (m_energyBarWidget->m_config.m_panelSize.y < 0.f)
	{
		m_energyBarWidget->m_config.m_panelSize.y = 0.f;
	}
	m_energyBarWidget->Build();
}

Actor& GameScene::SpawnActor(SpawnInfo const& spawnInfo)
{
	unsigned int actorSalt = m_salt;
	m_salt++;
	
	for (int i = 0; i < (int)m_allActors.size(); i++)
	{
		if (m_allActors[i] == nullptr)
		{
			ActorUID newUID = ActorUID(actorSalt, (unsigned int)i);
			m_allActors[i] = ConstructNewActor(spawnInfo, newUID);
			m_allActors[i]->m_isActive = false;
			AddActorToRelevantLists(m_allActors[i]);
			return *m_allActors[i];
		}
	}

	ActorUID newUID = ActorUID(actorSalt, (unsigned int)m_allActors.size());
	Actor* spawnedActor = ConstructNewActor(spawnInfo, newUID);
	spawnedActor->m_isActive = false;
	m_allActors.push_back(spawnedActor);
	AddActorToRelevantLists(spawnedActor);
	return *m_allActors[newUID.GetIndex()];
}

void GameScene::RemoveActor(Actor* actorToRemove)
{
	if (actorToRemove->m_definition->m_collisionType != CollisionType::NONE)
	{
		RemoveActorFromList(actorToRemove, m_collisionActors);
	}
	if (actorToRemove->m_definition->m_damageable)
	{
		RemoveActorFromList(actorToRemove, m_damageableActors);
	}
	if (actorToRemove->m_definition->m_actorType == ActorType::THRALL)
	{
		RemoveActorFromList(actorToRemove, m_thralls);
	}
	if (actorToRemove->m_definition->m_collisionType != CollisionType::NONE && !actorToRemove->m_definition->m_isTrigger)
	{
		if (actorToRemove->m_definition->m_faction == Faction::ENEMY)
		{
			RemoveActorFromList(actorToRemove, m_obstacles);
		}
	}
}

void GameScene::AddActorToList(Actor* actorToAdd, std::vector<Actor*>& actorList)
{
	for (int i = 0; i < (int)actorList.size(); i++)
	{
		if (actorList[i] == nullptr)
		{
			actorList[i] = actorToAdd;
			return;
		}
	}
	actorList.push_back(actorToAdd);
}

void GameScene::RemoveActorFromList(Actor* actorToRemove, std::vector<Actor*>& actorList)
{
	for (int i = 0; i < (int)actorList.size(); i++)
	{
		if (actorList[i] == actorToRemove)
		{
			actorList[i] = nullptr;
		}
	}
}

void GameScene::DeleteDeadActors()
{
	for (int i = 0; i < (int)m_allActors.size(); i++)
	{
		if (m_allActors[i] != nullptr && !m_allActors[i]->m_isAlive)
		{
			RemoveActor(m_allActors[i]);
			delete m_allActors[i];
			m_allActors[i] = nullptr;
		}
	}
}

void GameScene::AddActorToRelevantLists(Actor* actorToAdd)
{
	if (actorToAdd == nullptr)
	{
		return;
	}
	AddActorToList(actorToAdd, m_newlySpawnedActors);
	if (actorToAdd->m_definition->m_collisionType != CollisionType::NONE)
	{
		AddActorToList(actorToAdd, m_collisionActors);
	}
	if (actorToAdd->m_definition->m_damageable)
	{
		AddActorToList(actorToAdd, m_damageableActors);
	}
	if (actorToAdd->m_definition->m_actorType == ActorType::THRALL)
	{
		AddActorToList(actorToAdd, m_thralls);
	}
	if (actorToAdd->m_definition->m_collisionType != CollisionType::NONE && !actorToAdd->m_definition->m_isTrigger)
	{
		if (actorToAdd->m_definition->m_faction == Faction::ENEMY)
		{
			AddActorToList(actorToAdd, m_obstacles);
		}
	}
}

void GameScene::UpdateActorCollision(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	for (int lIdx = 0; lIdx < (int)m_collisionActors.size() - 1; lIdx++)
	{
		if (!IsValidActor(m_collisionActors[lIdx]))
		{
			continue;
		}
		for (int rIdx = lIdx + 1; rIdx < (int)m_collisionActors.size(); rIdx++)
		{
			if (!IsValidActor(m_collisionActors[rIdx]))
			{
				continue;
			}
			Actor* rActor = m_collisionActors[rIdx];
			Actor* lActor = m_collisionActors[lIdx];
			if (rActor->m_definition->m_faction == lActor->m_definition->m_faction && (!rActor->m_definition->m_collidesWithFaction || !lActor->m_definition->m_collidesWithFaction))
			{
				continue;
			}

			//true if actors overlapped, also pushed actors out of eachother if they did overlap (right now only spheres)
			if (rActor->ActorOverlapHandled(*lActor))
			{
				ResolveActorCollision(lActor, rActor);
			}
		}
	}
}

void GameScene::ResolveActorCollision(Actor* actor1, Actor* actor2)
{
	if (actor1->m_definition->m_damageable && actor2->m_definition->m_overlapDamage > 0.f)
	{
		actor1->TakeDamage(actor2->m_definition->m_overlapDamage);
	}
	if (actor2->m_definition->m_damageable && actor1->m_definition->m_overlapDamage > 0.f)
	{
		actor2->TakeDamage(actor1->m_definition->m_overlapDamage);
	}
	if (actor1->m_definition->m_dieOnOverlap)
	{
		actor1->Die();
	}
	if (actor2->m_definition->m_dieOnOverlap)
	{
		actor2->Die();
	}

	if (actor1->m_definition->m_actorType == ActorType::PLAYER && actor2->m_definition->m_missleAmmoToDrop > 0)
	{
		g_theGame->m_playerShip->m_missileAmmo += actor2->m_definition->m_missleAmmoToDrop;
		actor2->Die();
	}
	if (actor1->m_definition->m_actorType == ActorType::PLAYER && actor2->m_definition->m_megaLaserAmmoToDrop > 0.f)
	{
		g_theGame->m_playerShip->m_megaLaserAmmo += actor2->m_definition->m_megaLaserAmmoToDrop;
		actor2->Die();
	}
	if (actor1->m_definition->m_actorType == ActorType::PLAYER && actor2->m_definition->m_healthToRestore > 0.f)
	{
		g_theGame->m_playerShip->m_currentHealth += actor2->m_definition->m_healthToRestore;
		g_theGame->m_playerShip->m_currentHealth = Clamp(g_theGame->m_playerShip->m_currentHealth, 0.f, 100.f);
		actor2->Die();
	}

	if (actor2->m_definition->m_actorType == ActorType::PLAYER && actor1->m_definition->m_missleAmmoToDrop > 0)
	{
		g_theGame->m_playerShip->m_missileAmmo += actor1->m_definition->m_missleAmmoToDrop;
		actor1->Die();
	}
	if (actor2->m_definition->m_actorType == ActorType::PLAYER && actor1->m_definition->m_megaLaserAmmoToDrop > 0.f)
	{
		g_theGame->m_playerShip->m_megaLaserAmmo += actor1->m_definition->m_megaLaserAmmoToDrop;
		actor1->Die();
	}
	if (actor2->m_definition->m_actorType == ActorType::PLAYER && actor1->m_definition->m_healthToRestore > 0.f)
	{
		g_theGame->m_playerShip->m_currentHealth += actor1->m_definition->m_healthToRestore;
		g_theGame->m_playerShip->m_currentHealth = Clamp(g_theGame->m_playerShip->m_currentHealth, 0.f, 100.f);
		actor1->Die();
	}
}

void GameScene::SpawnBob()
{
	Vec3 randomPos = Vec3(g_randGen->RollRandomFloatInRange(-50.f, 50.f), g_randGen->RollRandomFloatInRange(-50.f, 50.f), g_randGen->RollRandomFloatInRange(-50.f, 50.f));
	EulerAngles randomRot = EulerAngles(g_randGen->RollRandomFloatInRange(0.f, 360.f), g_randGen->RollRandomFloatInRange(0.f, -90.f), g_randGen->RollRandomFloatInRange(0.f, 0.f));
	SpawnInfo testShipSpawnInfo(randomPos, randomRot.GetAsMatrix_IFwd_JLeft_KUp(), randomRot.GetIFwd() * 5.f, ActorDefinition::GetActorDefinitionFromName("Bob"));
	SpawnActor(testShipSpawnInfo);
}

void GameScene::RandomActorSpawnNear(std::string const& actorName)
{
	Vec3 randomSpawnDir = g_theGame->m_playerShip->GetForwardNormal();
	FloatRange spawnDirYawRotation(-80.f, 80.f);
	FloatRange spawnDirPitchRotation(-35.f, 35.f);
	FloatRange spawnDistance(50.f, 60.f);

	EulerAngles spawnRotation(g_randGen->RollRandomFloatInRange(spawnDirYawRotation), g_randGen->RollRandomFloatInRange(spawnDirPitchRotation), 0.f);
	Mat44 spawnRotationAsMat = spawnRotation.GetAsMatrix_IFwd_JLeft_KUp();
	randomSpawnDir = spawnRotationAsMat.TransformVectorQuantity3D(randomSpawnDir);
	Vec3 randomSpawnDisplacment = g_randGen->RollRandomFloatInRange(spawnDistance) * randomSpawnDir;

	Vec3 randomPos = g_theGame->m_playerShip->GetWorldPosition() + randomSpawnDisplacment;
	SpawnInfo testActorSpawnInfo(randomPos, Mat44(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName(actorName));
	SpawnActor(testActorSpawnInfo);
}

void GameScene::RandomActorSpawnFar(std::string const& actorName)
{
	Vec3 randomSpawnDir = g_theGame->m_playerShip->GetForwardNormal();
	FloatRange spawnDirYawRotation(-25.f, 25.f);
	FloatRange spawnDirPitchRotation(-25.f, 25.f);
	FloatRange spawnDistance(80.f, 100.f);

	EulerAngles spawnRotation(g_randGen->RollRandomFloatInRange(spawnDirYawRotation), g_randGen->RollRandomFloatInRange(spawnDirPitchRotation), 0.f);
	Mat44 spawnRotationAsMat = spawnRotation.GetAsMatrix_IFwd_JLeft_KUp();
	randomSpawnDir = spawnRotationAsMat.TransformVectorQuantity3D(randomSpawnDir);
	Vec3 randomSpawnDisplacment = g_randGen->RollRandomFloatInRange(spawnDistance) * randomSpawnDir;

	Vec3 randomPos = g_theGame->m_playerShip->GetWorldPosition() + randomSpawnDisplacment;
	SpawnInfo spawnInfo(randomPos, Mat44(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName(actorName));
	SpawnActor(spawnInfo);
}

void GameScene::RandomActorSpawn(std::string const& actorName)
{
	Vec3 spawnPos(g_randGen->RollRandomFloatInRange(-200.f, 200.f), g_randGen->RollRandomFloatInRange(-200.f, 200.f), g_randGen->RollRandomFloatInRange(-200.f, 200.f));
	SpawnInfo spawnInfo(spawnPos, Mat44(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName(actorName));
	SpawnActor(spawnInfo);
}


void GameScene::LoseGame()
{
	g_theGame->SetDesiredGameState(GameState::END_MENU);
	g_theGame->m_gameScene->StopAllAudio();
	EndScene* endScene = dynamic_cast<EndScene*>(g_theGame->GetGameScene(GameState::END_MENU));
	endScene->m_wonGame = false;
}

void GameScene::WinGame()
{
	g_theGame->SetDesiredGameState(GameState::END_MENU);
	g_theGame->m_gameScene->StopAllAudio();
	EndScene* endScene = dynamic_cast<EndScene*>(g_theGame->GetGameScene(GameState::END_MENU));
	endScene->m_wonGame = true;
}

void GameScene::StopAllAudio()
{
	for (int i = 0; i < (int)m_trackLayers.size(); i++)
	{
		g_theAudio->StopSound(m_trackLayers[i].m_soundPlayback);
	}
}


void GameScene::SetTrackLayerVolume(TrackLayerType trackToSet, float newVolume)
{
	g_theAudio->SetSoundPlaybackVolume(m_trackLayers[(int)trackToSet].m_soundPlayback, newVolume);
}

SoundPlaybackID GameScene::GetTrackLayerPlaybackID(TrackLayerType trackType)
{
	return m_trackLayers[(int)trackType].m_soundPlayback;
}

Actor* GameScene::ConstructNewActor(SpawnInfo const& spawnInfo, ActorUID actorUID)
{
	if (spawnInfo.m_definition->m_actorType == ActorType::THRALL)
	{
		return new Thrall(spawnInfo, actorUID);
	}
	if (spawnInfo.m_definition->m_actorType == ActorType::PLAYER)
	{
		return new PlayerShip(spawnInfo, actorUID);
	}
	return new Actor(spawnInfo, actorUID);
}

float GameScene::GetCurrentTimeFromLastBeat() const
{
	float currentTrackBeatsDecimal = m_currentTrackPlaybackTime / BEAT_TIME;
	float currentBeatFraction = currentTrackBeatsDecimal - (float)RoundDownToInt(currentTrackBeatsDecimal);
	return currentBeatFraction * BEAT_TIME;
}

void GameScene::QueueBeatSound(BeatSound const& beatSound)
{
	//play the sound immediately if it is already on beat otherwise, queue it up to align with the next beat
	if (IsTimeAtBeatFraction(beatSound.m_beatTimeInSeconds))
	{
		//g_theAudio->StartSound(beatSound.m_sound);
		GameSound gameSound;
		gameSound.m_soundID = beatSound.m_sound;
		gameSound.m_cooldownTimer = Timer(.1f, g_theApp->m_clock);
		gameSound.m_volume = beatSound.m_volume;
		g_theGame->PlayGameSound(gameSound);
		return;
	}

	for (int i = 0; i < (int)m_queuedOnBeatSounds.size(); i++)
	{
		if (m_queuedOnBeatSounds[i].m_sound == MISSING_SOUND_ID)
		{
			m_queuedOnBeatSounds[i] = beatSound;
			return;
		}
	}
	m_queuedOnBeatSounds.push_back(beatSound);
}

bool GameScene::IsTimeAtBeatFraction(float beatFractionInSeconds)
{
	int currentFrameBeatFractions = RoundDownToInt(m_currentTrackPlaybackTime / beatFractionInSeconds);
	int prevFrameBeatFractions = RoundDownToInt(m_prevFrameTrackPlaybackTime / beatFractionInSeconds);
	if (currentFrameBeatFractions != prevFrameBeatFractions)
	{
		return true;
	}
	return false;
}

void GameScene::SpawnBigAsteroid()
{
	ActorDefinition const* bigAsteroidDef = ActorDefinition::GetActorDefinitionFromName("BigAsteroid");
	if (bigAsteroidDef == nullptr)
	{
		return;
	}
	Vec3 randomPos = Vec3(g_randGen->RollRandomFloatInRange(-300.f, 300.f), g_randGen->RollRandomFloatInRange(-300.f, 300.f), g_randGen->RollRandomFloatInRange(-300.f, 300.f));
	SpawnInfo spawnInfo(randomPos, Mat44(), Vec3::ZERO, bigAsteroidDef);
	Actor& asteroid = SpawnActor(spawnInfo);
	asteroid.m_rotationSpeed = EulerAngles(g_randGen->RollRandomFloatInRange(-4.f, 4.f), g_randGen->RollRandomFloatInRange(-4.f, 4.f), g_randGen->RollRandomFloatInRange(-4.f, 4.f));
}

void GameScene::SpawnSmallAsteroid()
{
	ActorDefinition const* smallAsteroidDef = ActorDefinition::GetActorDefinitionFromName("SmallAsteroid2C");
	if (smallAsteroidDef == nullptr)
	{
		return;
	}
	Vec3 randomPos = Vec3(g_randGen->RollRandomFloatInRange(-300.f, 300.f), g_randGen->RollRandomFloatInRange(-300.f, 300.f), g_randGen->RollRandomFloatInRange(-300.f, 300.f));
	SpawnInfo spawnInfo(randomPos, Mat44(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName("SmallAsteroid2C"));
	Actor& asteroid = SpawnActor(spawnInfo);
	asteroid.m_rotationSpeed = EulerAngles(g_randGen->RollRandomFloatInRange(-15.f, 15.f), g_randGen->RollRandomFloatInRange(-15.f, 15.f), g_randGen->RollRandomFloatInRange(-15.f, 15.f));
	asteroid.SetLocalScale(Vec3(g_randGen->RollRandomFloatInRange(.8f, 1.2f), g_randGen->RollRandomFloatInRange(.8f, 1.2f), g_randGen->RollRandomFloatInRange(.8f, 1.2f)));
}

void GameScene::SpawnMissileAmmo()
{
	ActorDefinition const* missileAmmo = ActorDefinition::GetActorDefinitionFromName("MissilePickup");
	if (missileAmmo == nullptr)
	{
		return;
	}
	Vec3 randomPos = Vec3(g_randGen->RollRandomFloatInRange(-200.f, 200.f), g_randGen->RollRandomFloatInRange(-200.f, 200.f), g_randGen->RollRandomFloatInRange(-200.f, 200.f));
	SpawnInfo spawnInfo(randomPos, Mat44(), Vec3::ZERO, missileAmmo);
	SpawnActor(spawnInfo);
}

void GameScene::SpawnMegaLaserAmmo()
{
	ActorDefinition const* battery = ActorDefinition::GetActorDefinitionFromName("Battery");
	if (battery == nullptr)
	{
		return;
	}
	Vec3 randomPos = Vec3(g_randGen->RollRandomFloatInRange(-200.f, 200.f), g_randGen->RollRandomFloatInRange(-200.f, 200.f), g_randGen->RollRandomFloatInRange(-200.f, 200.f));
	SpawnInfo spawnInfo(randomPos, Mat44(), Vec3::ZERO, battery);
	SpawnActor(spawnInfo);
}



void GameScene::PlayBeatSounds()
{
	for (int i = 0; i < m_queuedOnBeatSounds.size(); i++)
	{
		if (m_queuedOnBeatSounds[i].m_sound == MISSING_SOUND_ID)
		{
			continue;
		}
		if (IsTimeAtBeatFraction(m_queuedOnBeatSounds[i].m_beatTimeInSeconds))
		{
			GameSound gameSound;
			gameSound.m_soundID = m_queuedOnBeatSounds[i].m_sound;
			gameSound.m_cooldownTimer = Timer(.1f, g_theApp->m_clock);
			gameSound.m_volume = m_queuedOnBeatSounds[i].m_volume;
			g_theGame->PlayGameSound(gameSound);
			//g_theAudio->StartSound(m_queuedOnBeatSounds[i].m_sound, false, m_queuedOnBeatSounds[i].m_volume);
			m_queuedOnBeatSounds[i].m_sound = MISSING_SOUND_ID;
		}
	}
}

void GameScene::RespawnPlayer()
{
	RemoveActor(g_theGame->m_playerShip);
	int indexToRemoveAt = g_theGame->m_playerShip->m_uid.GetIndex();
	delete m_allActors[indexToRemoveAt];
	m_allActors[indexToRemoveAt] = nullptr;
	SpawnInfo playerShipSpawn(Vec3::ZERO, Mat44(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName("Player"));
	g_theGame->m_playerShip = dynamic_cast<PlayerShip*>(&SpawnActor(playerShipSpawn));
	g_theParticleSystem->KillAllEmitters();
}

void GameScene::AddGameWidgets()
{
	WidgetConfig healthBorderConfig;
	healthBorderConfig.m_allignment = Vec2(.925f, .925f);
	healthBorderConfig.m_borderColor = Rgba8::WHITE;
	healthBorderConfig.m_panelColor = Rgba8::WHITE;
	healthBorderConfig.m_panelSize = Vec2(.3f, .03f);
	healthBorderConfig.m_borderSize = Vec2::ZERO;
	healthBorderConfig.m_name = "healthBarBorder";
	Widget* healthBorder = new Widget(nullptr, healthBorderConfig);

	WidgetConfig healthBarConfig;
	healthBarConfig.m_allignment = Vec2(.5, .5f);
	healthBarConfig.m_borderSize = Vec2::ZERO;
	healthBarConfig.m_panelSize = Vec2(.99f, .99f);
	healthBarConfig.m_panelColor = Rgba8::BLACK;
	healthBarConfig.m_matchPanelPadding = true;
	Widget* healthOuter = healthBorder->AddChild(healthBarConfig);

	healthBarConfig.m_allignment = Vec2(0.f, .5f);
	healthBarConfig.m_panelSize = Vec2(1.f, 1.f);
	healthBarConfig.m_panelColor = Rgba8::RED;
	healthBarConfig.m_matchPanelPadding = false;
	m_healthBarWidget = healthOuter->AddChild(healthBarConfig);

	m_allWidgets.push_back(healthBorder);


	WidgetConfig energyBorderConfig;
	energyBorderConfig.m_allignment = Vec2(.5325f, .5f);
	energyBorderConfig.m_panelColor = Rgba8(255,255,255,75);
	energyBorderConfig.m_panelSize = Vec2(.005f, .05f);
	energyBorderConfig.m_borderSize = Vec2::ZERO;
	healthBorderConfig.m_name = "energyBarBorder";
	Widget* energyBarBorder = new Widget(nullptr, energyBorderConfig);

	WidgetConfig energyBarConfig;
	energyBarConfig.m_allignment = Vec2(.5, .5f);
	energyBarConfig.m_borderSize = Vec2::ZERO;
	energyBarConfig.m_panelSize = Vec2(.95f, .95f);
	energyBarConfig.m_panelColor = Rgba8(0,0,0,75);
	energyBarConfig.m_matchPanelPadding = true;
	Widget* energyOuter = energyBarBorder->AddChild(energyBarConfig);

	energyBarConfig.m_allignment = Vec2(.5f, .0f);
	energyBarConfig.m_panelSize = Vec2(1.f, 1.f);
	energyBarConfig.m_panelColor = Rgba8(0,255,255,75);
	energyBarConfig.m_matchPanelPadding = false;
	m_energyBarWidget = energyOuter->AddChild(energyBarConfig);

	m_allWidgets.push_back(energyBarBorder);
}

void GameScene::ShutDown()
{
	for (int i = 0; i < (int)m_allActors.size(); i++)
	{
		delete m_allActors[i];
		m_allActors[i] = nullptr;
	}
	for (int i = 0; i < (int)m_allWidgets.size(); i++)
	{
		delete m_allWidgets[i];
		m_allWidgets[i] = nullptr;
	}
	g_theParticleSystem->KillAllEmitters();
}



void GameScene::LoadEnemySpawnerFromXML(std::string const& enemySpawnerFilePath)
{
	XmlDocument actorDefDocument;
	GUARANTEE_OR_DIE(actorDefDocument.LoadFile(enemySpawnerFilePath.c_str()) == 0, std::string("Failed to load ") + std::string("Data/Definitions/EnemySpawner.xml"));
	XmlElement* rootElement = actorDefDocument.FirstChildElement("EnemySpawner");
	GUARANTEE_OR_DIE(rootElement != nullptr, std::string("Could not get the root EnemySpawner element from ") + std::string("Data/Definitions/EnemySpawner.xml"));
	for (XmlElement const* currElement = rootElement->FirstChildElement("Spawner");
		currElement != nullptr; currElement = currElement->NextSiblingElement("Spawner"))
	{
		int currentSpawnIdx = (int)m_unitSpawners.size();
		m_unitSpawners.emplace_back();
		UnitSpawner& currentSpawner = m_unitSpawners[currentSpawnIdx];

		int beat = ParseXmlAttribute(*currElement, "beat", 0);
		currentSpawner.m_beat = beat;

		XmlElement const* rate = currElement->FirstChildElement("Rate");
		if (rate != nullptr)
		{
			currentSpawner.m_unitsPerBeat = ParseXmlAttribute(*rate, "unitsPerBeat", -1.f);
			for (XmlElement const* currRateUnit = rate->FirstChildElement("RateUnit");
				currRateUnit != nullptr; currRateUnit = currRateUnit->NextSiblingElement("RateUnit"))
			{
				int currentRateIndex = (int)currentSpawner.m_rateSpawner.size();
				currentSpawner.m_rateSpawner.emplace_back();
				UnitRateSpawn& currentUnitRateSpawn = currentSpawner.m_rateSpawner[currentRateIndex];
				currentUnitRateSpawn.m_chance = ParseXmlAttribute(*currRateUnit, "chance", 0.f);
				currentUnitRateSpawn.m_name = ParseXmlAttribute(*currRateUnit, "name", "");
				std::string spawnTypeAsStr = ParseXmlAttribute(*currRateUnit, "spawnType", "RANDOM");
				currentUnitRateSpawn.m_spawnType = GetSpawnTypeFromStr(spawnTypeAsStr);
			}
		}

		XmlElement const* burst = currElement->FirstChildElement("Burst");
		if (burst != nullptr)
		{
			for (XmlElement const* currBurstUnit = burst->FirstChildElement("BurstUnit");
				currBurstUnit != nullptr; currBurstUnit = currBurstUnit->NextSiblingElement("BurstUnit"))
			{
				int currentBurstIndex = (int)currentSpawner.m_burstSpawner.size();
				currentSpawner.m_burstSpawner.emplace_back();
				UnitBurstSpawn& currentUnitBurstSpawn = currentSpawner.m_burstSpawner[currentBurstIndex];
				currentUnitBurstSpawn.m_number = ParseXmlAttribute(*currBurstUnit, "number", 0);
				currentUnitBurstSpawn.m_name = ParseXmlAttribute(*currBurstUnit, "name", "");
				std::string spawnTypeAsStr = ParseXmlAttribute(*currBurstUnit, "spawnType", "RANDOM");
				currentUnitBurstSpawn.m_spawnType = GetSpawnTypeFromStr(spawnTypeAsStr);
			}
		}

		XmlElement const* trackSwitch = currElement->FirstChildElement("TrackSwitch");
		if (trackSwitch != nullptr)
		{
			std::string trackSwitchPath = ParseXmlAttribute(*trackSwitch, "path", "");
			if (trackSwitchPath != "")
			{
				currentSpawner.m_trackSwitch = g_theAudio->CreateOrGetSound(trackSwitchPath);
			}
			currentSpawner.m_trackSwitchBeats = ParseXmlAttribute(*trackSwitch, "beats", 32);
		}

	}
}

void GameScene::UpdateEnemySpawner()
{
	if (!m_demoStarted)
	{
		return;
	}
	if ((int)m_unitSpawners.size() == 0)
	{
		return;
	}

	while (m_unitSpawners.size() > 0 && m_unitSpawners[0].m_beat <= m_currentBeats)
	{
		//track switch
		if (m_unitSpawners[0].m_trackSwitch != MISSING_SOUND_ID)
		{
			g_theAudio->StopSound(m_trackLayers[(int)TrackLayerType::BASE].m_soundPlayback);
			m_trackLayers[(int)TrackLayerType::BASE].m_soundID = m_unitSpawners[0].m_trackSwitch;
			m_trackLayers[(int)TrackLayerType::BASE].m_soundPlayback = g_theAudio->StartSound(m_unitSpawners[0].m_trackSwitch, true);
			m_trackLayers[(int)TrackLayerType::BASE].m_numberOfBeats = m_unitSpawners[0].m_trackSwitchBeats;
		}

		//rate spawn
		if (!m_unitSpawners[0].m_rateSpawner.empty())
		{
			m_currentUnitRateSpawner = m_unitSpawners[0].m_rateSpawner;
			if (m_unitSpawners[0].m_unitsPerBeat >= 0.f)
			{
				m_currentUnitSpawnRate = m_unitSpawners[0].m_unitsPerBeat;
			}
		}
	

		//burst spawn
		if (!m_unitSpawners[0].m_burstSpawner.empty())
		{
			std::vector<UnitBurstSpawn>& currentBurstSpawner = m_unitSpawners[0].m_burstSpawner;
			for (int i = 0; i < (int)currentBurstSpawner.size(); i++)
			{
				for (int n = 0; n < currentBurstSpawner[i].m_number; n++)
				{
					SpawnActorWithSpawnType(currentBurstSpawner[i].m_name, currentBurstSpawner[i].m_spawnType);
					//RandomActorSpawnFar(currentBurstSpawner[i].m_name);
				}
			}
		}
		m_unitSpawners.erase(m_unitSpawners.begin());
	}
	m_owedUnitsThisBeat += m_currentUnitSpawnRate;
	while (m_owedUnitsThisBeat >= 1.f)
	{
		SpawnUnitFromCurrentRate();
		m_owedUnitsThisBeat--;
	}
}

void GameScene::SpawnUnitFromCurrentRate()
{
	float randomChance = g_randGen->RollRandomFloatZeroToOne();
	for (int i = 0; i < m_currentUnitRateSpawner.size(); i++)
	{
		if (randomChance <= m_currentUnitRateSpawner[i].m_chance)
		{
			SpawnActorWithSpawnType(m_currentUnitRateSpawner[i].m_name, m_currentUnitRateSpawner[i].m_spawnType);
			break;
			//RandomActorSpawnNear(m_currentUnitRateSpawner[i].m_name);
		}
		randomChance -= m_currentUnitRateSpawner[i].m_chance;
	}
}

void GameScene::RenderGameUI() const
{
	for (int i = 0; i < m_allWidgets.size(); i++)
	{
		if (m_allWidgets[i] != nullptr && m_allWidgets[i]->m_enabled)
		{
			m_allWidgets[i]->Render();
		}
	}

	std::vector<Vertex_PCU> ringVerts;
	AddVertsForRing2D(ringVerts, g_theGame->m_screenCamera.GetOrthoDimensions() * .5f + g_theGame->m_playerShip->m_mouseScreenSpaceOffset, m_evenBeatRadius, 4.f, m_evenBeatColor, 64);
	AddVertsForRing2D(ringVerts, g_theGame->m_screenCamera.GetOrthoDimensions() * .5f + g_theGame->m_playerShip->m_mouseScreenSpaceOffset, m_oddBeatRadius, 4.f, m_oddBeatColor, 64);
	AddVertsForRing2D(ringVerts, g_theGame->m_screenCamera.GetOrthoDimensions() * .5f + g_theGame->m_playerShip->m_mouseScreenSpaceOffset, m_beatRadius.m_min, 2.f, Rgba8::CYAN, 64);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexArray(ringVerts.size(), ringVerts.data());

	if (m_currentBeats % 2 == 0)
	{
		float chSize = 8.f;
		std::vector<Vertex_PCU> crosshairVerts;
		AddVertsForLine2D(crosshairVerts, g_theGame->m_screenCamera.GetOrthoDimensions() * .5f + Vec2(0.f, chSize), g_theGame->m_screenCamera.GetOrthoDimensions() * .5f + Vec2(0.f, -chSize), 1.f, Rgba8::CYAN);
		AddVertsForLine2D(crosshairVerts, g_theGame->m_screenCamera.GetOrthoDimensions() * .5f + Vec2(chSize, 0.f), g_theGame->m_screenCamera.GetOrthoDimensions() * .5f + Vec2(-chSize, 0.f), 1.f, Rgba8::CYAN);
		g_theRenderer->DrawVertexArray(crosshairVerts.size(), crosshairVerts.data());
	}
}

void GameScene::ActivateNewlySpawnedActors()
{
	for (int i = 0; i < m_newlySpawnedActors.size(); i++)
	{
		m_newlySpawnedActors[i]->Activate();
	}
	m_newlySpawnedActors.clear();
}




Actor* GameScene::GetActorByUID(const ActorUID uid)
{
	if (uid == ActorUID::INVALID)
	{
		return nullptr;
	}
	int index = uid.GetIndex();
	if (!IsValidActor(m_allActors[index]))
	{
		return nullptr;
	}
	if (uid == m_allActors[index]->m_uid)
	{
		return m_allActors[index];
	}
	return nullptr;
}

SpawnType GameScene::GetSpawnTypeFromStr(std::string const& spawnType)
{
	std::string spawnTypeLower = ToLower(spawnType);
	if (spawnTypeLower == "near")
	{
		return SpawnType::NEAR;
	}
	if (spawnTypeLower == "far")
	{
		return SpawnType::FAR;
	}

	return SpawnType::RANDOM;
}

void GameScene::SpawnActorWithSpawnType(std::string const& unitName, SpawnType unitSpawnType)
{
	if (unitSpawnType == SpawnType::NEAR)
	{
		RandomActorSpawnNear(unitName);
	}
	else if (unitSpawnType == SpawnType::FAR)
	{
		RandomActorSpawnFar(unitName);
	}
	else
	{
		RandomActorSpawn(unitName);
	}
}

