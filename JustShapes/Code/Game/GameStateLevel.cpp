#include "GameStateLevel.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Button.hpp"
#include "Game/Player.hpp"
#include "Game/Boss1.hpp"
#include "Game/Tasks.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Game/BoxHazard.hpp"
#include "Game/ConeHazardSpawner.hpp"
#include "Game/ExplodingProjectile.hpp"
#include "Game/Wisp.hpp"
#include "Game/Scythe.hpp"
#include "Game/Boss3.hpp"
#include "Engine/Renderer/Window.hpp"

GameStateLevel::GameStateLevel(GameStateType gameType, SoundID levelMusic)
	: m_levelMusic(levelMusic)
	, GameState(gameType)
{
}

void GameStateLevel::StartUp()
{
}

void GameStateLevel::OnEnable()
{

}

void GameStateLevel::OnDisable()
{
	m_needsReset = true;
}

void GameStateLevel::Update(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		TogglePause();
	}
	if (m_levelTimer.HasPeriodElapsed())
	{
		ShowWinLevel();
	}
	if (!m_winLevel && !g_theApp->m_isPaused)
	{
		UpdateTaskQueue();
	}
	for (int i = 0; i < (int)m_allEntities.size(); i++)
	{
		if (m_allEntities[i] != nullptr && !m_allEntities[i]->m_isGarbage)
		{
			if ((m_winLevel || g_theApp->m_isPaused) && m_allEntities[i]->m_entityType != EntityType::BUTTON)
			{
				continue;
			}
			m_allEntities[i]->Update(deltaSeconds);
		}
	}
	DeleteGarbageEntities();	
	UpdateCollision();
	m_prevFrameBeats = RoundDownToInt(GetTotalBeatsElapsed());
}

void GameStateLevel::UpdateTaskQueue()
{
	if (m_beatsToSkipAhead > 0.f)
	{
		m_levelTimer.m_startTime -= m_beatsToSkipAhead * m_beatTime;
		while (m_beatsToSkipAhead > 0.f && m_taskQueue.size() > 0)
		{
			m_beatsToSkipAhead -= m_taskQueue.front()->m_numBeats;
			m_taskBeatsElapsed += m_taskQueue.front()->m_numBeats;
			m_taskQueue.front()->DebugSkip();
			m_taskQueue.pop_front();
		}
	}

	if (GetTotalBeatsElapsed() >= m_beatsToPauseAt && m_doDebugPause)
	{
		m_doDebugPause = false;
		g_theApp->Pause();
	}

	int thisFrameBeats;
	if (IsBeatThisFrame(thisFrameBeats))
	{
		//DebugAddMessage(Stringf("Beats: %d", thisFrameBeats), 30.f, m_beatTime - .005f);
		//DebugAddMessage(Stringf("Task Beats: %d", thisFrameTaskBeats), 30.f, m_beatTime - .005f);
	}

	if (m_taskQueue.size() == 0)
	{
		return;
	}

	//taskQueue not initialized yet
	if (m_currentTask == nullptr)
	{
		m_currentTask = m_taskQueue.front();
		m_currentTask->Begin();
	}

	if (m_currentTask->m_numBeats <= GetCurrentTaskBeats())
	{
		m_currentTask->End();
		m_taskBeatsElapsed += m_currentTask->m_numBeats;
		m_taskQueue.pop_front();
		delete m_currentTask;
		m_currentTask = nullptr;

		if (m_taskQueue.size() == 0)
		{
			return;
		}
		m_currentTask = m_taskQueue.front();
		m_currentTask->Begin();
	}
	m_currentTask->Tick();
}

void GameStateLevel::Render()
{
	GameState::Render();

	g_theRenderer->BeginCamera(g_theGame->m_screenCamera);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	if (g_theApp->m_isPaused)
	{
		std::vector<Vertex_PCU> loseMenuVerts;
		AABB2 loseMenuBounds;
		loseMenuBounds.m_mins = GetScreenDimensions() * .25f;
		loseMenuBounds.m_maxs = GetScreenDimensions() * .75f;
		g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PauseBorder.png"));
		AddVertsForAABB2D(loseMenuVerts, loseMenuBounds, Rgba8::WHITE);
		g_theRenderer->DrawVertexArray(loseMenuVerts.size(), loseMenuVerts.data());
	}

	if (m_winLevel)
	{
		std::vector<Vertex_PCU> winMenuVerts;
		AABB2 winMenuBounds;
		winMenuBounds.m_mins = GetScreenDimensions() * .25f;
		winMenuBounds.m_maxs = GetScreenDimensions() * .75f;
		g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/LevelCompleteBorder.png"));
		AddVertsForAABB2D(winMenuVerts, winMenuBounds, Rgba8::WHITE);
		g_theRenderer->DrawVertexArray(winMenuVerts.size(), winMenuVerts.data());
		if (GetPlayer()->m_lives == 8)
		{
			g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/S.png"));
		}
		else if (GetPlayer()->m_lives >= 6)
		{
			g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/A.png"));
		}
		else if (GetPlayer()->m_lives >= 4)
		{
			g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/B.png"));
		}
		else if (GetPlayer()->m_lives >= 2)
		{
			g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/C.png"));
		}
		else
		{
			g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/D.png"));
		}

		std::vector<Vertex_PCU> rankVerts;
		AABB2 rankBounds;
		rankBounds.m_mins = winMenuBounds.m_mins + Vec2(winMenuBounds.GetDimensions().x * .5f, winMenuBounds.GetDimensions().y * .6f);
		rankBounds.m_maxs = rankBounds.m_mins + Vec2(128.f, 128.f);
		AddVertsForAABB2D(rankVerts, rankBounds, Rgba8::WHITE);
		g_theRenderer->DrawVertexArray(rankVerts.size(), rankVerts.data());
	}


	RenderProgressUI();

	for (int i = 0; i < (int)m_screenEntities.size(); i++)
	{
		if (m_screenEntities[i] != nullptr && !m_screenEntities[i]->m_isGarbage)
		{
			m_screenEntities[i]->Render();
		}
	}
	g_theRenderer->EndCamera(g_theGame->m_screenCamera);

}

void GameStateLevel::AddArms(float rotationSpeed)
{
	int numArms = 6;

	for (int i = 0; i < numArms; i++)
	{
		ProjectileConfig projectileConfig;

		projectileConfig.m_startOrientaiton = 360.f/(float)numArms * i;
		projectileConfig.m_rotationSpeed = rotationSpeed;
		Vec2 boxDimensions(15.f, .2f);
		projectileConfig.m_becomeHazardTime = .5f;
		projectileConfig.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(16.f * boxDimensions.x, 16.f * boxDimensions.y));
		projectileConfig.m_normalizedPivot = Vec2(0.f, .5f);
		projectileConfig.m_animation = "SpawnBoss3Box";
		projectileConfig.m_liveTime = 999.f;
		Vec2 boxPosition = GetWorldScreenDimensions() * .5f;
		BoxHazard* arm = new BoxHazard(this, EntityType::ARMS, boxPosition, projectileConfig);
		arm->m_sortOrder = 8;
		AddEntity(arm);
	}
}

void GameStateLevel::RenderLives()
{
	AABB2 screenCameraBounds(g_theGame->m_screenCamera.GetOrthoBottomLeft(), g_theGame->m_screenCamera.GetOrthoTopRight());
	std::vector<Vertex_PCU> livesVerts;
	for (int i = 0; i < GetPlayer()->m_maxLives; i++)
	{
		bool renderOpaque = false;
		if (i + 1 > GetPlayer()->m_lives)
		{
			renderOpaque = true;
		}
		Vec2 livesSquareDimensions = Vec2(16.f, 16.f);
		Vec2 livesPos(screenCameraBounds.m_maxs.x - 2 * livesSquareDimensions.x * (i + 1), screenCameraBounds.m_maxs.y * .965f);
		if (renderOpaque)
		{
			AddVertsForAABB2D(livesVerts, AABB2(livesPos - livesSquareDimensions * .5f, livesPos + livesSquareDimensions * .5f), Rgba8(0, 229, 249, 100));
		}
		else
		{
			AddVertsForAABB2D(livesVerts, AABB2(livesPos - livesSquareDimensions * .5f, livesPos + livesSquareDimensions * .5f), Rgba8(0, 229, 249, 255));
		}
	}
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(livesVerts.size(), livesVerts.data());
}

void GameStateLevel::RenderProgressUI()
{
	g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/ProgressUIBar.png"));
	std::vector<Vertex_PCU> progressUIVerts;
	//Vec2 worldMins = Vec2(112.f, 224.f);
	//Vec2 worldMaxs = Vec2(368.f, 240.f);
	Vec2 worldMins = Vec2(g_theGame->m_worldCamera.GetOrthoDimensions().x * .15f, g_theGame->m_worldCamera.GetOrthoDimensions().y * .93f);
	Vec2 worldMaxs = Vec2(g_theGame->m_worldCamera.GetOrthoDimensions().x * .85f, g_theGame->m_worldCamera.GetOrthoDimensions().y * 1.f);
	Vec2 screenMins = WorldCoordinatesToScreenCoordinates(worldMins);
	Vec2 screenMaxs = WorldCoordinatesToScreenCoordinates(worldMaxs);
	AddVertsForAABB2D(progressUIVerts, AABB2(screenMins, screenMaxs), Rgba8::WHITE);
	g_theRenderer->DrawVertexArray(progressUIVerts.size(), progressUIVerts.data());
	std::vector<Vertex_PCU> progressIndicatorVerts;
	Vec2 progressSize = WorldCoordinatesToScreenCoordinates(Vec2(6.f, 6.f));
	if (!m_levelTimer.IsStopped())
	{
		m_progressPos.x = RangeMap(m_levelTimer.GetElapsedFraction(), 0.f, 1.f, screenMins.x + 12, screenMaxs.x - 12);
		m_progressPos.y = (screenMins.y + screenMaxs.y) * .5f;
	}
	AddVertsForAABB2D(progressIndicatorVerts, AABB2(m_progressPos - .5f * progressSize, m_progressPos + .5f * progressSize), Rgba8::WHITE);
	g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/ProgressUIMarker.png"));
	g_theRenderer->DrawVertexArray(progressIndicatorVerts.size(), progressIndicatorVerts.data());
}

void GameStateLevel::ShowWinLevel()
{
	if (m_winLevel)
	{
		return;
	}
	m_levelTimer.Stop();
	g_theGame->PauseMusic();
	m_winLevel = true;
	if (GetPlayer()->m_lives >= 7)
	{
		g_theGame->PlaySound(g_theAudio->CreateOrGetSound("Data/Audio/Excellent.wav"), SoundType::SFX, false);
	}
	else if (GetPlayer()->m_lives <= 1)
	{
		g_theGame->PlaySound(g_theAudio->CreateOrGetSound("Data/Audio/Barely.wav"), SoundType::SFX, false);
	}
	ButtonConfig buttonConfig;
	buttonConfig.m_clickDimensions = Vec2(200.f, 100.f);
	buttonConfig.m_eventName = "Back";
	buttonConfig.m_idleSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MainMenuButton1.png");
	buttonConfig.m_pressedSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MainMenuButton2.png");
	buttonConfig.m_pressedNoise = SOUND_ID_CLICK;
	buttonConfig.m_spriteDimensions = Vec2(200.f, 100.f);
	buttonConfig.m_cellHeight = 25.f;
	Vec2 mainMenuButtonPos = Vec2(GetScreenDimensions().x * .4f, GetScreenDimensions().y * .4f);
	Button* mainMenuButton = new Button(this, EntityType::BUTTON, mainMenuButtonPos, buttonConfig);
	AddEntity(mainMenuButton);

	buttonConfig.m_clickDimensions = Vec2(200.f, 100.f);
	buttonConfig.m_eventName = "NextLevelPressed";
	g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_NextLevelPressed);
	buttonConfig.m_idleSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/NextLevelButton1.png");
	buttonConfig.m_pressedSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/NextLevelButton2.png");
	buttonConfig.m_pressedNoise = SOUND_ID_CLICK;
	buttonConfig.m_spriteDimensions = Vec2(200.f, 100.f);
	buttonConfig.m_cellHeight = 25.f;
	Vec2 nextLevelButtonPos = mainMenuButtonPos + Vec2::RIGHT * 300.f;
	Button* nextLevelButton = new Button(this, EntityType::BUTTON, nextLevelButtonPos, buttonConfig);
	AddEntity(nextLevelButton);
}

void GameStateLevel::TogglePause()
{
	g_theApp->Pause();

	if (g_theApp->m_isPaused)
	{
		g_theGame->PauseMusic();
		ButtonConfig buttonConfig;
		buttonConfig.m_clickDimensions = Vec2(200.f, 100.f);
		buttonConfig.m_eventName = "Back";
		buttonConfig.m_idleSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MainMenuButton1.png");
		buttonConfig.m_pressedSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/MainMenuButton2.png");
		buttonConfig.m_pressedNoise = SOUND_ID_CLICK;
		buttonConfig.m_spriteDimensions = Vec2(200.f, 100.f);
		buttonConfig.m_cellHeight = 25.f;
		Vec2 mainMenuButtonPos = Vec2(GetScreenDimensions().x * .4f, GetScreenDimensions().y * .4f);
		Button* mainMenuButton = new Button(this, EntityType::BUTTON, mainMenuButtonPos, buttonConfig);
		AddEntity(mainMenuButton);

		buttonConfig.m_clickDimensions = Vec2(200.f, 100.f);
		buttonConfig.m_eventName = "Resume";
		g_theEventSystem->SubscribeEventCallbackFunction(buttonConfig.m_eventName, Event_ResumeLevel);
		buttonConfig.m_idleSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/ResumeButton1.png");
		buttonConfig.m_pressedSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/ResumeButton2.png");
		buttonConfig.m_pressedNoise = SOUND_ID_CLICK;
		buttonConfig.m_spriteDimensions = Vec2(200.f, 100.f);
		buttonConfig.m_cellHeight = 25.f;
		Vec2 retryButtonPos = mainMenuButtonPos + Vec2::RIGHT * 300.f;
		Button* retryButton = new Button(this, EntityType::BUTTON, retryButtonPos, buttonConfig);
		AddEntity(retryButton);
	}
	else
	{
		g_theGame->ResumeMusic();
		RemoveAllButtons();
	}

}

void GameStateLevel::UpdateCollision()
{
	Player* player = GetPlayer();
	for (int i = 0; i < m_hazardEntities.size(); i++)
	{
		if (m_hazardEntities[i] != nullptr && m_hazardEntities[i]->m_isHazard && m_hazardEntities[i]->OverlapsPlayer(player))
		{
			player->LoseLife();
		}
	}
}

void GameStateLevel::SpawnProjectilesInCircle(Vec2 spawnPoint, int numProjectiles, EntityConfig& entityConfig, float speed, float acceleration, float thetaOffset)
{
	float thetaStep = 360.f / (float)numProjectiles;
	float theta = thetaOffset;
	for (int i = 0; i < numProjectiles; i++)
	{
		Vec2 spawnDir = Vec2::MakeFromPolarDegrees(theta);
		entityConfig.m_startVelocity = spawnDir * speed;
		entityConfig.m_startAcceleration = spawnDir * acceleration;
		AddEntity(new Entity(this, EntityType::ENEMY_PROJECTILE, spawnPoint, entityConfig));
		theta += thetaStep;
	}
}

void GameStateLevel::SpawnProjectilesInSpiral(Vec2 spawnPoint, int numProjectiles, float timeBetweenProjectiles, EntityConfig& bulletConfig, float speed, float acceleration, bool clockwise, float numCircles)
{
	float thetaStep = (360.f * numCircles) / (float)numProjectiles;
	float theta = 0.f;
	if (clockwise)
	{
		thetaStep *= -1.f;
		theta = 180.f;
	}
	float distanceOut = 12.f;
	for (int i = 0; i < numProjectiles; i++)
	{
		Vec2 spawnDir = Vec2::MakeFromPolarDegrees(theta);
		bulletConfig.m_startVelocity = spawnDir * speed;
		bulletConfig.m_startAcceleration = spawnDir * acceleration;
		bulletConfig.m_hideTime = (float)i * timeBetweenProjectiles;
		AddEntity(new Entity(this, EntityType::ENEMY_PROJECTILE, spawnPoint + Vec2::MakeFromPolarDegrees(theta) * distanceOut, bulletConfig));
		theta += thetaStep;
	}
}

void GameStateLevel::SpawnEntityInRandomPosOnScreen(EntityConfig& config, EntityType type, Vec2 normalizedScreenPadding)
{
	AABB2 cameraBounds(g_theGame->m_worldCamera.GetOrthoBottomLeft(), g_theGame->m_worldCamera.GetOrthoTopRight());
	Vec2 paddingDimensions = cameraBounds.GetDimensions();
	paddingDimensions.x *= normalizedScreenPadding.x;
	paddingDimensions.y *= normalizedScreenPadding.y;
	Vec2 spawnPos = cameraBounds.GetRandomPointInsideBox(g_randGen, paddingDimensions);
	AddEntity(new Entity(this, type, spawnPos, config));
}

Vec2 GameStateLevel::GetRandomPosBelowScreen(float padding)
{
	AABB2 cameraBounds(g_theGame->m_worldCamera.GetOrthoBottomLeft(), g_theGame->m_worldCamera.GetOrthoTopRight());
	Vec2 RandomPosBelowScreen;
	RandomPosBelowScreen.x = g_randGen->RollRandomFloatInRange(cameraBounds.m_mins.x + padding, cameraBounds.m_maxs.x - padding);
	RandomPosBelowScreen.y = cameraBounds.m_mins.y - 2 * padding;
	return RandomPosBelowScreen;
}

void GameStateLevel::SpawnEntityAtPos(EntityConfig& config, EntityType type, Vec2 pos)
{
	AddEntity(new Entity(this, type, pos, config));
}

void GameStateLevel::SpawnScytheAtPos(EntityConfig& config, EntityType type, Vec2 pos)
{
	AddEntity(new Scythe(this, type, pos, config));
}

Vec2 GameStateLevel::RollRandomPosOnScreen(Vec2 normalizedScreenPadding)
{
	AABB2 cameraBounds(g_theGame->m_worldCamera.GetOrthoBottomLeft(), g_theGame->m_worldCamera.GetOrthoTopRight());
	Vec2 paddingDimensions = cameraBounds.GetDimensions();
	paddingDimensions.x *= normalizedScreenPadding.x;
	paddingDimensions.y *= normalizedScreenPadding.y;
	Vec2 randomPos = cameraBounds.GetRandomPointInsideBox(g_randGen, paddingDimensions);
	return randomPos;
}

void GameStateLevel::SpawnRandomBarHazard(bool alwaysVertical)
{
	AABB2 cameraBounds(g_theGame->m_worldCamera.GetOrthoBottomLeft(), g_theGame->m_worldCamera.GetOrthoTopRight());
	Vec2 paddingDimensions = cameraBounds.GetDimensions() * .15f;
	Vec2 barPos = cameraBounds.GetRandomPointInsideBox(g_randGen, paddingDimensions);
	Vec2 screenCenter = cameraBounds.GetCenter();
	Vec2 barSize;


	if (alwaysVertical)
	{
		barSize = Vec2(1.f, 50.f);
	}
	//spawn horizontal
	else if (fabsf(screenCenter.x - barPos.x) * .50f < fabsf(screenCenter.y - barPos.y))
	{
		barSize = Vec2(50.f, 1.f);
	}
	//spawnVertical
	else
	{
		barSize = Vec2(1.f, 50.f);
	}
	ProjectileConfig config;
	config.m_animation = "SpawnBar";
	config.m_liveTime = 2.f;
	config.m_becomeHazardTime = 1.f;
	config.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(16.f * barSize.x, 16.f * barSize.y));
	AddEntity(new BoxHazard(this, EntityType::ENEMY_PROJECTILE, barPos, config));
}

void GameStateLevel::SpawnRandomDiscHazard()
{
	AABB2 cameraBounds(g_theGame->m_worldCamera.GetOrthoBottomLeft(), g_theGame->m_worldCamera.GetOrthoTopRight());
	Vec2 paddingDimensions = cameraBounds.GetDimensions() * .1f;
	Vec2 discPos = cameraBounds.GetRandomPointInsideBox(g_randGen, paddingDimensions);
	ProjectileConfig config;
	config.m_animation = "SpawnCircleBullet";
	config.m_liveTime = 2.f;
	config.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(64, 64));
	config.m_collisionRadius = 32.f;
	config.m_becomeHazardTime = 1.f;
	AddEntity(new EnemyProjectile(this, EntityType::ENEMY_PROJECTILE, discPos, config));
}

void GameStateLevel::SpawnDiscHazardOnPlayer()
{
	Vec2 discPos = GetPlayer()->GetPosition();
	ProjectileConfig config;
	config.m_animation = "SpawnCircleBullet";
	config.m_liveTime = 2.f;
	config.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(64, 64));
	config.m_collisionRadius = 32.f;
	config.m_becomeHazardTime = 1.f;
	AddEntity(new EnemyProjectile(this, EntityType::ENEMY_PROJECTILE, discPos, config));
}

void GameStateLevel::SpawnWispOffscreen()
{
	AABB2 cameraBounds(g_theGame->m_worldCamera.GetOrthoBottomLeft(), g_theGame->m_worldCamera.GetOrthoTopRight());
	Vec2 paddingDimensions = cameraBounds.GetDimensions() * .1f;
	Vec2 wispPos = cameraBounds.GetRandomPointOutsideBox(16.f, g_randGen);

	AddEntity(new Wisp(this, EntityType::WISP, wispPos, EntityConfig::GetEntityConfigByName("Wisp")));
}

void GameStateLevel::SpawnConeHazard()
{
	Vec2 centerPos = g_theGame->m_worldCamera.GetCameraPos();
	Vec2 cameraDimensions = g_theGame->m_worldCamera.GetOrthoDimensions();

	Vec2 coneDirection;
	int randInt = g_randGen->RollRandomIntInRange(0, 3);
	if (randInt == 0)
	{
		coneDirection = Vec2::RIGHT;
	}
	else if (randInt == 1)
	{
		coneDirection = Vec2::UP;
	}
	else if (randInt == 2)
	{
		coneDirection = Vec2::LEFT;
	}
	else if (randInt == 3)
	{
		coneDirection = Vec2::DOWN;
	}
	Vec2 spawnPos;
	ProjectileConfig config;
	if (fabsf(coneDirection.x) > 0.f)
	{
		spawnPos.y = g_randGen->RollRandomFloatInRange(centerPos.y - cameraDimensions.y * .25f, centerPos.y + cameraDimensions.y * .25f);
		spawnPos.x = centerPos.x - coneDirection.x * cameraDimensions.x * .6f;
		config.m_maxConeAngle = 20.f;
	}
	else
	{
		spawnPos.x = g_randGen->RollRandomFloatInRange(centerPos.x - cameraDimensions.x * .25f, centerPos.x + cameraDimensions.x * .25f);
		spawnPos.y = centerPos.y - coneDirection.y * cameraDimensions.y * .6f;
		config.m_maxConeAngle = 40.f;
	}

	config.m_animation = "SpawnConeHazard";
	config.m_liveTime = 1.f;
	config.m_becomeHazardTime = 2.5f;
	config.m_startOrientaiton = coneDirection.GetOrientationDegrees();
	AddEntity(new ConeHazardSpawner(this, EntityType::ENEMY_PROJECTILE, spawnPos, config));
}

void GameStateLevel::SpawnHeartbeatIndicator()
{
	Vec2 centerPos = g_theGame->m_worldCamera.GetCameraPos();
	Vec2 cameraDimensions = g_theGame->m_worldCamera.GetOrthoDimensions();

	Vec2 coneDirection = Vec2::LEFT;
	Vec2 spawnPos = GetBoss3()->GetPosition();
	ProjectileConfig config;
	config.m_maxConeAngle = 20.f;



	config.m_animation = "SpawnConeHazard";
	config.m_liveTime = 1.f;
	config.m_becomeHazardTime = 2.5f;
	config.m_startOrientaiton = coneDirection.GetOrientationDegrees();
	AddEntity(new ConeHazardSpawner(this, EntityType::ENEMY_PROJECTILE, spawnPos, config, false));
}

void GameStateLevel::SpawnBossExplosion()
{
	int bulletsToFire = 300;
	float fireTime = 3.f;
	float fireRate = fireTime / (float)bulletsToFire;

	for (int i = 0; i < bulletsToFire; i++)
	{

		Vec2 position = GetBoss1()->GetPosition() + g_randGen->RollRandomNormalizedVec2() * g_randGen->RollRandomFloatInRange(0.f, 32.f);
		ProjectileConfig config;
		config.m_animation = "ExplodeParticle";
		config.m_liveTime = 1.f;
		config.m_becomeHazardTime = 999.f;
		config.m_hideTime = (float)i * fireRate;
		config.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(32.f, 32.f));
		AddEntity(new EnemyProjectile(this, EntityType::ENEMY_PROJECTILE, position, config));
	}
}

void GameStateLevel::SpawnExplodingProjectile()
{
	AABB2 cameraBounds(g_theGame->m_worldCamera.GetOrthoBottomLeft(), g_theGame->m_worldCamera.GetOrthoTopRight());
	Vec2 paddingDimensions = cameraBounds.GetDimensions() * .3f;
	Vec2 bulletPos = cameraBounds.GetRandomPointInsideBox(g_randGen, paddingDimensions);

	ProjectileConfig config;
	config.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/SpikeyBall.png");
	config.m_animation = "ExplodingProjectile";
	config.m_collisionRadius = 9.f;
	config.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(18.f, 18.f));
	config.m_liveTime = 1.4f;
	AddEntity(new ExplodingProjectile(this, EntityType::ENEMY_PROJECTILE, bulletPos, config));
}

void GameStateLevel::BeatDropKillAllScythes()
{
	for (int i = 0; i < (int)m_allEntitiesByType[(int)EntityType::SCYTHE].size(); i++)
	{
		Scythe* scythe = dynamic_cast<Scythe*>(m_allEntitiesByType[(int)EntityType::SCYTHE][i]);
		if (scythe != nullptr)
		{
			scythe->DestroyScythe();
		}
	}
}

void GameStateLevel::DestroyBobbingProjectiles()
{
	for (int i = 0; i < (int)m_allEntitiesByType[(int)EntityType::BOBBING_PROJECTILE].size(); i++)
	{
		m_allEntitiesByType[(int)EntityType::BOBBING_PROJECTILE][i]->DestroyEntity();
	}
}

void GameStateLevel::DestroyWisps()
{
	for (int i = 0; i < (int)m_allEntitiesByType[(int)EntityType::WISP].size(); i++)
	{
		if (m_allEntitiesByType[(int)EntityType::WISP][i] != nullptr)
		{
			m_allEntitiesByType[(int)EntityType::WISP][i]->DestroyEntity();
		}
	}
}

void GameStateLevel::RemoveAllArms()
{
	for (int i = 0; i < (int)m_allEntitiesByType[(int)EntityType::ARMS].size(); i++)
	{
		if (m_allEntitiesByType[(int)EntityType::ARMS][i] != nullptr)
		{
			m_allEntitiesByType[(int)EntityType::ARMS][i]->DestroyEntity();
		}
	}
}

void GameStateLevel::RemoveAllButtons()
{
	for (int i = 0; i < (int)m_allEntitiesByType[(int)EntityType::BUTTON].size(); i++)
	{
		if (m_allEntitiesByType[(int)EntityType::BUTTON][i] != nullptr)
		{
			m_allEntitiesByType[(int)EntityType::BUTTON][i]->DestroyEntity();
		}
	}
}

void GameStateLevel::SpawnArcProjectiles(Vec2 coneOrigin, float coneOrientation, float coneMaxAngle)
{
	int bulletsToFire = 100;
	float fireTime = 1.f;
	float fireRate = fireTime / (float)bulletsToFire;
	float innerConeAngle = coneMaxAngle * .1f;
	float backAndForthTimes = 3.f;
	float sinPeriod = 180.f;

	for (int i = 0; i < bulletsToFire; i++)
	{
		float minOuterAngle = coneOrientation - coneMaxAngle * .5f;
		float maxOuterAngle = coneOrientation + coneMaxAngle * .5f;

		//get back and forth T in 0 to 1
		float backAndForthT = ((float)i / bulletsToFire) * backAndForthTimes;
		backAndForthT = backAndForthT - (float)RoundDownToInt(backAndForthT);

		float posInArcZeroTo1 = SinDegrees(sinPeriod * backAndForthT);
		float maxInnerAngle = Lerp(minOuterAngle + innerConeAngle, maxOuterAngle, posInArcZeroTo1);
		float minInnerAngle = maxInnerAngle - innerConeAngle;

		float angle = Lerp(minInnerAngle, maxInnerAngle, g_randGen->RollRandomFloatZeroToOne());
		float distanceOut = g_randGen->RollRandomFloatInRange(16.f, 64.f);

		Vec2 position = coneOrigin + Vec2::MakeFromPolarDegrees(angle, distanceOut);
		ProjectileConfig config;
		config.m_animation = "SquareProjectile";
		config.m_collisionRadius = 8.f;
		config.m_becomeHazardTime = .0f;
		config.m_hideTime = (float)i * fireRate;
		config.m_rotationSpeed = 60.f;
		config.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(32.f, 32.f));
		float minVelocity = 64.f;
		float maxVelocity = 200.f;
		config.m_velocity = Vec2::MakeFromPolarDegrees(angle) * g_randGen->RollRandomFloatInRange(minVelocity, maxVelocity);
		config.m_acceleration = config.m_velocity.GetNormalized() * 128.f;
		AddEntity(new EnemyProjectile(this, EntityType::ENEMY_PROJECTILE, position, config));
	}
}

Boss1* GameStateLevel::GetBoss1()
{
	if (m_allEntitiesByType[(int)EntityType::BOSS_1].size() == 0)
	{
		return nullptr;
	}
	return dynamic_cast<Boss1*>(m_allEntitiesByType[(int)EntityType::BOSS_1][0]);
}

float GameStateLevel::GetTotalBeatsElapsed()
{
	float beatsElapsed = m_levelTimer.GetElapsedTime() / m_beatTime;
	return beatsElapsed;
}

float GameStateLevel::GetCurrentTaskBeats()
{
	return GetTotalBeatsElapsed() - m_taskBeatsElapsed;
}

void GameStateLevel::SetPause(bool isPaused)
{
	if (isPaused)
	{
		g_theGame->PauseMusic();
	}
	else
	{
		g_theGame->ResumeMusic();
	}
}

bool GameStateLevel::IsBeatThisFrame(int& out_beatNumber)
{
	out_beatNumber = RoundDownToInt(GetTotalBeatsElapsed());
	if (out_beatNumber != m_prevFrameBeats)
	{
		return true;
	}
	return false;
}

bool Event_ResetLevel(EventArgs& args)
{
	UNUSED(args);
	g_theGame->m_currentGameState->m_needsReset = true;
	return false;
}

bool Event_ResumeLevel(EventArgs& args)
{
	UNUSED(args);
	GameStateLevel* level = dynamic_cast<GameStateLevel*>(g_theGame->m_currentGameState);
	if (level)
	{
		level->TogglePause();
	}
	return false;
}

bool Event_NextLevelPressed(EventArgs& args)
{
	UNUSED(args);
	g_theGame->AdvanceLevel();
	return false;
}

