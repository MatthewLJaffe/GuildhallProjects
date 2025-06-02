#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Entity.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Bullet.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Beetle.hpp"
#include "Game/Wasp.hpp"
#include "Game/Debris.hpp"
#include "Game/Scout.hpp"
#include "Game/Charger.hpp"
#include "Game/Shotgunner.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Missile.hpp"
#include "Game/LaserBeam.hpp"
#include "Game/AmmoPickup.hpp"

Game::Game(App* app)
	: m_theApp(app)
	, m_attractLeftShipStartPos(Vec2(200.f, 400.f))
	, m_attractLeftShipCurrPos(Vec2(100.f, 400.f))
	, m_attractLeftShipEndPos(Vec2(500.f, 400.f))
	, m_attractRightShipStartPos(Vec2(1400.f, 400.f))
	, m_attractRightShipCurrPos(Vec2(1400.f, 400.f))
	, m_attractRightShipEndPos(Vec2(1000.f, 400.f))
{
	m_worldCamera.m_mode = Camera::eMode_Orthographic;
	m_worldCamera.SetOrthographicView(Vec2::ZERO, Vec2(200.f, 100.f));

	m_screenCamera.m_mode = Camera::eMode_Orthographic;
	m_screenCamera.SetOrthographicView(Vec2::ZERO, Vec2(1600.f, 800.f));
}

Game::~Game() {}

UnitAmmount::UnitAmmount(int numEntities, unsigned char type)
{
	ammount = numEntities;
	unitType = type;
}


Wave::Wave(int numberOfUnits, UnitAmmount* unitAmounts)
	: numUnitTypes(numberOfUnits)
	, units(unitAmounts)
{ }

Wave::~Wave()
{
	delete[] units;
}

void Game::StartUp()
{
	m_beetles = new Entity* [MAX_BEETLES] {nullptr};
	m_wasps = new Entity* [MAX_WASPS] {nullptr};
	m_scouts = new Entity* [MAX_SCOUTS] {nullptr};
	m_chargers = new Entity* [MAX_CHARGERS] {nullptr};
	m_shotgunners = new Entity* [MAX_SHOTGUNNERS] {nullptr};
	m_missiles = new Entity* [MAX_MISSILES] {nullptr};
	m_ammoPickups = new  Entity* [MAX_AMMO_PICKUPS] {nullptr};

	m_attractScreen = true;
	//Initialize waves
	m_waves[0] = new Wave(3, new UnitAmmount[]{ UnitAmmount(2, 'B'), UnitAmmount(4, 'A'), UnitAmmount(1, 'W')});
	m_waves[1] = new Wave(4, new UnitAmmount[]{ UnitAmmount(5, 'A'), UnitAmmount(5, 'B'), UnitAmmount(4, 'W'), UnitAmmount(3, 'S') });
	m_waves[2] = new Wave(5, new UnitAmmount[]{ UnitAmmount(6, 'A'), UnitAmmount(4, 'W'), UnitAmmount(8, 'S'), UnitAmmount(1, 'C'), UnitAmmount(2, 's') });
	m_waves[3] = new Wave(5, new UnitAmmount[]{ UnitAmmount(8, 'A'), UnitAmmount(10, 'W'), UnitAmmount(10, 'S'), UnitAmmount(4, 'C'), UnitAmmount(4, 's') });
	m_waves[4] = new Wave(4, new UnitAmmount[]{ UnitAmmount(8, 'A'), UnitAmmount(10, 'S'), UnitAmmount(8, 'C'), UnitAmmount(8, 's') });


	//initialize verts for start screen

	//Ship
	m_attractShipVerts[0] = Vertex_PCU(Vec3(0, 2, 0), Rgba8(102, 153, 204, 255));
	m_attractShipVerts[1] = Vertex_PCU(Vec3(2, 1, 0), Rgba8(102, 153, 204, 255));
	m_attractShipVerts[2] = Vertex_PCU(Vec3(-2, 1, 0), Rgba8(102, 153, 204, 255));

	m_attractShipVerts[3] = Vertex_PCU(Vec3(-2, 1, 0), Rgba8(102, 153, 204, 255));
	m_attractShipVerts[4] = Vertex_PCU(Vec3(0, 1, 0), Rgba8(102, 153, 204, 255));
	m_attractShipVerts[5] = Vertex_PCU(Vec3(-2, -1, 0), Rgba8(102, 153, 204, 255));

	m_attractShipVerts[6] = Vertex_PCU(Vec3(0, 1, 0), Rgba8(102, 153, 204, 255));
	m_attractShipVerts[7] = Vertex_PCU(Vec3(-2, -1, 0), Rgba8(102, 153, 204, 255));
	m_attractShipVerts[8] = Vertex_PCU(Vec3(0, -1, 0), Rgba8(102, 153, 204, 255));

	m_attractShipVerts[9] = Vertex_PCU(Vec3(0, 1, 0), Rgba8(102, 153, 204, 255));
	m_attractShipVerts[10] = Vertex_PCU(Vec3(1, 0, 0), Rgba8(102, 153, 204, 255));
	m_attractShipVerts[11] = Vertex_PCU(Vec3(0, -1, 0), Rgba8(102, 153, 204, 255));

	m_attractShipVerts[12] = Vertex_PCU(Vec3(-2, -1, 0), Rgba8(102, 153, 204, 255));
	m_attractShipVerts[13] = Vertex_PCU(Vec3(2, -1, 0), Rgba8(102, 153, 204, 255));
	m_attractShipVerts[14] = Vertex_PCU(Vec3(0, -2, 0), Rgba8(102, 153, 204, 255));
	
	//Play Button
	m_attractPlayVerts[0] = Vertex_PCU(Vec3(1, 0, 0), Rgba8(61, 186, 86, 255));
	m_attractPlayVerts[1] = Vertex_PCU(Vec3(-1, 1, 0), Rgba8(61, 186, 86, 255));
	m_attractPlayVerts[2] = Vertex_PCU(Vec3(-1, -1, 0), Rgba8(61, 186, 86, 255));

	g_theAudio->StartSound(SOUND_ID_STARTUP);
}

void Game::StartScreenShake(float shakeTime, float shakeIntensity)
{
	if (m_totalShakeTime < shakeTime)
		m_totalShakeTime = shakeTime;
	if (m_currShakeTime < shakeTime)
		m_currShakeTime = shakeTime;
	if (m_shakeTrauma < shakeIntensity)
		m_shakeTrauma = shakeIntensity;
}

void Game::StartGame()
{
	m_playerShip = new PlayerShip(this, Vec2(100, 50));
	SpawnWave(m_currWave);
	m_attractScreen = false;
}

void Game::UpdateScreenShake(float deltaSeconds)
{
	if (m_currShakeTime <= 0.f)
	{
		m_shakeTrauma = 0.f;
		m_currShakeTime = 0.f;
		m_worldCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y));
		return;
	}
	m_currShakeTime -= deltaSeconds;
	float currIntensity = Lerp(0.f, m_shakeTrauma, m_currShakeTime / m_totalShakeTime);
	Vec2 randomBottomLeft(g_randGen->RollRandomFloatInRange(-1.f, 1.f), g_randGen->RollRandomFloatInRange(-1.f, 1.f));
	randomBottomLeft.SetLength(currIntensity);
	Vec2 randomTopRight = randomBottomLeft + Vec2(WORLD_SIZE_X, WORLD_SIZE_Y);
	m_worldCamera.SetOrthographicView(randomBottomLeft, randomTopRight);
}

void Game::RenderStartScreen() const
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	//left ship
	Vertex_PCU worldAttractShipLeft[NUM_SHIP_VERTS];
	for (int i = 0; i < NUM_SHIP_VERTS; i++)
	{
		worldAttractShipLeft[i] = m_attractShipVerts[i];
	}
	TransformVertexArrayXY3D(NUM_SHIP_VERTS, worldAttractShipLeft, 64.f, 0.f, m_attractLeftShipCurrPos);
	g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, worldAttractShipLeft);

	//play button
	Vertex_PCU worldPlayButton[3];
	for (int i = 0; i < 3; i++)
	{
		worldPlayButton[i] = m_attractPlayVerts[i];
		worldPlayButton[i].m_color.a = static_cast<unsigned char>(Clamp(m_attractScreenAlpha * 255.f, 0.f, 255.f));
	}
	TransformVertexArrayXY3D(3, worldPlayButton, 80.f, 0.f, Vec2(775.f, 400.f));
	g_theRenderer->DrawVertexArray(3, worldPlayButton);

	//right ship
	Vertex_PCU worldAttractShipRight[NUM_SHIP_VERTS];
	for (int i = 0; i < NUM_SHIP_VERTS; i++)
	{
		worldAttractShipRight[i] = m_attractShipVerts[i];
	}
	TransformVertexArrayXY3D(NUM_SHIP_VERTS, worldAttractShipRight, 64.f, 180.f,  m_attractRightShipCurrPos);
	g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, worldAttractShipRight);
}

Bullet* Game::GetBulletCollision(Entity* entity)
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] == nullptr) continue;
		if (DoDiscsOverlap(entity->m_position, entity->m_physicsRadius, m_bullets[i]->m_position, m_bullets[i]->m_physicsRadius))
		{
			if (m_bullets[i]->m_targetPlayer && entity->m_typeID == PLAYER_SHIP_ID)
				return m_bullets[i];
			if (!m_bullets[i]->m_targetPlayer && entity->m_typeID != PLAYER_SHIP_ID)
				return m_bullets[i];
		}
	}
	return nullptr;
}

PlayerShip* Game::GetPlayerCollision(Entity* entity)
{
	if (DoDiscsOverlap(entity->m_position, entity->m_physicsRadius, m_playerShip->m_position, m_playerShip->m_physicsRadius))
	{
		return m_playerShip;
	}
	return nullptr;
}

Asteroid* Game::GetAsteroidCollision(Entity* entity)
{
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] == nullptr) continue;
		if (DoDiscsOverlap(entity->m_position, entity->m_physicsRadius, m_asteroids[i]->m_position, m_asteroids[i]->m_physicsRadius))
		{
			return m_asteroids[i];
		}
	}
	return nullptr;
}

void Game::Update(float deltaSeconds)
{
	if (m_secondsPaused > 0.0)
	{
		m_secondsPaused -= static_cast<double>(deltaSeconds);
		deltaSeconds = 0.f;
	}
	if (m_attractScreen)
	{
		//animate play alpha
		if (m_attractScreenAlpha < .5f)
		{
			m_attractAlphaRate = 1.f;
			m_attractScreenAlpha = .5f;
		}
		else if (m_attractScreenAlpha > 1.f)
		{
			m_attractAlphaRate = -1.f;
			m_attractScreenAlpha = 1.f;
		}
		m_attractScreenAlpha += deltaSeconds * m_attractAlphaRate;

		//update attract screen ships coming in
		float shipT = SinDegrees(m_shipOscillateTheta);
		shipT = RangeMap(shipT, -1.f, 1.f, 0.f, 1.f);
		m_attractLeftShipCurrPos.x = Lerp(m_attractLeftShipStartPos.x, m_attractLeftShipEndPos.x, shipT);
		m_attractRightShipCurrPos.x = Lerp(m_attractRightShipStartPos.x, m_attractRightShipEndPos.x, shipT);
		m_shipOscillateTheta += deltaSeconds * 180.f;
		if (m_shipOscillateTheta > 360.f)
			m_shipOscillateTheta = 0.f;

		//check for start game
		if (g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('N'))
		{
			StartGame();
		}
		const XboxController controller = g_theInput->GetController(0);
		if (controller.IsConnected() && controller.WasButtonJustPressed(XboxController::A_BUTTON))
		{
			StartGame();
		}
	}
	else
	{
		UpdateEntities(deltaSeconds);
	}
	
	if (m_resetPending)
	{
		CheckForResetGame(deltaSeconds);
	}
	UpdateScreenShake(deltaSeconds);
}

void Game::UpdateEntities(float deltaSeconds)
{
	m_playerShip->Update(deltaSeconds);
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
			m_bullets[i]->Update(deltaSeconds);
	}
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr)
			m_asteroids[i]->Update(deltaSeconds);
	}
	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] != nullptr)
			m_beetles[i]->Update(deltaSeconds);
	}
	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] != nullptr)
			m_wasps[i]->Update(deltaSeconds);
	}
	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] != nullptr)
			m_debris[i]->Update(deltaSeconds);
	}
	for (int i = 0; i < MAX_SCOUTS; i++)
	{
		if (m_scouts[i] != nullptr)
			m_scouts[i]->Update(deltaSeconds);
	}
	for (int i = 0; i < MAX_CHARGERS; i++)
	{
		if (m_chargers[i] != nullptr)
			m_chargers[i]->Update(deltaSeconds);
	}
	for (int i = 0; i < MAX_SHOTGUNNERS; i++)
	{
		if (m_shotgunners[i] != nullptr)
			m_shotgunners[i]->Update(deltaSeconds);
	}
	for (int i = 0; i < MAX_MISSILES; i++)
	{
		if (m_missiles[i] != nullptr)
			m_missiles[i]->Update(deltaSeconds);
	}
	for (int i = 0; i < MAX_AMMO_PICKUPS; i++)
	{
		if (m_ammoPickups[i] != nullptr)
			m_ammoPickups[i]->Update(deltaSeconds);
	}
}

void Game::Render() const 
{
	if (m_attractScreen)
	{
		g_theRenderer->BeginCamera(m_screenCamera);
		RenderStartScreen();
		g_theRenderer->EndCamera(m_screenCamera);
	}
	else
	{
		//World Space Rendering
		g_theRenderer->BeginCamera(m_worldCamera);
		RenderEntities();
		if (g_theApp->m_debugMode)
		{
			RenderDebug();
		}
		g_theRenderer->EndCamera(m_worldCamera);

		//ScreenSpace Rendering
		g_theRenderer->BeginCamera(m_screenCamera);
		m_playerShip->RenderUI();
		g_theRenderer->EndCamera(m_screenCamera);

	}
}

void Game::RenderEntities() const
{
	m_playerShip->Render();
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
			m_bullets[i]->Render();
	}
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr)
			m_asteroids[i]->Render();
	}
	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] != nullptr)
			m_beetles[i]->Render();
	}
	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] != nullptr)
			m_wasps[i]->Render();
	}
	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] != nullptr)
			m_debris[i]->Render();
	}
	for (int i = 0; i < MAX_SCOUTS; i++)
	{
		if (m_scouts[i] != nullptr)
			m_scouts[i]->Render();
	}
	for (int i = 0; i < MAX_CHARGERS; i++)
	{
		if (m_chargers[i] != nullptr)
			m_chargers[i]->Render();
	}
	for (int i = 0; i < MAX_SHOTGUNNERS; i++)
	{
		if (m_shotgunners[i] != nullptr)
			m_shotgunners[i]->Render();
	}
	for (int i = 0; i < MAX_MISSILES; i++)
	{
		if (m_missiles[i] != nullptr)
			m_missiles[i]->Render();
	}
	for (int i = 0; i < MAX_AMMO_PICKUPS; i++)
	{
		if (m_ammoPickups[i] != nullptr)
			m_ammoPickups[i]->Render();
	}
}

void Game::DeleteGarbageEntities()
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] == nullptr) continue;
		if (m_bullets[i]->m_isGarbage)
		{
			delete m_bullets[i];
			m_bullets[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] == nullptr) continue;
		if (m_asteroids[i]->m_isGarbage)
		{
			delete m_asteroids[i];
			m_asteroids[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] == nullptr) continue;
		if (m_beetles[i]->m_isGarbage)
		{
			delete m_beetles[i];
			m_beetles[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] == nullptr) continue;
		if (m_wasps[i]->m_isGarbage)
		{
			delete m_wasps[i];
			m_wasps[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] == nullptr) continue;
		if (m_debris[i]->m_isGarbage)
		{
			delete m_debris[i];
			m_debris[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_SCOUTS; i++)
	{
		if (m_scouts[i] == nullptr) continue;
		if (m_scouts[i]->m_isGarbage)
		{
			delete m_scouts[i];
			m_scouts[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_CHARGERS; i++)
	{
		if (m_chargers[i] == nullptr) continue;
		if (m_chargers[i]->m_isGarbage)
		{
			delete m_chargers[i];
			m_chargers[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_SHOTGUNNERS; i++)
	{
		if (m_shotgunners[i] == nullptr) continue;
		if (m_shotgunners[i]->m_isGarbage)
		{
			delete m_shotgunners[i];
			m_shotgunners[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_MISSILES; i++)
	{
		if (m_missiles[i] == nullptr) continue;
		if (m_missiles[i]->m_isGarbage)
		{
			delete m_missiles[i];
			m_missiles[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_AMMO_PICKUPS; i++)
	{
		if (m_ammoPickups[i] == nullptr) continue;
		if (m_ammoPickups[i]->m_isGarbage)
		{
			delete m_ammoPickups[i];
			m_ammoPickups[i] = nullptr;
		}
	}
}

void Game::CreateBullet(Vec2 pos, float rotation)
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] == nullptr)
		{
			m_bullets[i] = new Bullet(this, pos, rotation);
			return;
		}
	}
	ERROR_RECOVERABLE("Cannot spawn a new bullet; are slots are full!");
}

void Game::CreateBullet(Vec2 pos, float rotation, float speed, bool targetPlayer, float scale, Rgba8 centerColor, Rgba8 tailColor)
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] == nullptr)
		{
			m_bullets[i] = new Bullet(this, pos, rotation, speed, targetPlayer, scale, centerColor, tailColor);
			return;
		}
	}
	ERROR_RECOVERABLE("Cannot spawn a new bullet; are slots are full!");
}


void Game::SpawnAsteroid()
{
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] == nullptr)
		{
			m_asteroids[i] = new Asteroid(this, GetPosOutsideScreen(ASTEROID_COSMETIC_RADIUS), new RandomNumberGenerator());
			return;
		}
	}
	ERROR_RECOVERABLE("Cannot spawn a new asteroid; are slots are full!");
}


void Game::SpawnBeetle()
{
	Vec2 pos = GetPosOutsideScreen(BEETLE_COSMETIC_RADIUS);
	
	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] == nullptr)
		{
			m_beetles[i] = new Beetle(this, pos);
			return;
		}
	}
}

void Game::SpawnWasp()
{
	Vec2 pos = GetPosOutsideScreen(WASP_COSMETIC_RADIUS);

	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] == nullptr)
		{
			m_wasps[i] = new Wasp(this, pos);
			return;
		}
	}
}

void Game::SpawnDebris(const Vec2& pos, const Vec2& startingVelocity, float minSize, float maxSize, const Rgba8& color)
{
	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] == nullptr)
		{
			m_debris[i] = new Debris(this, pos, startingVelocity, minSize, maxSize, color, new RandomNumberGenerator());
			return;
		}
	}
}

void Game::SpawnScout()
{
	Vec2 pos = GetPosOutsideScreen(SCOUT_COSMETIC_RADIUS);

	for (int i = 0; i < MAX_SCOUTS; i++)
	{
		if (m_scouts[i] == nullptr)
		{
			m_scouts[i] = new Scout(this, pos);
			return;
		}
	}
}

void Game::SpawnCharger()
{
	Vec2 pos = GetPosOutsideScreen(CHARGER_COSMETIC_RADIUS);

	for (int i = 0; i < MAX_CHARGERS; i++)
	{
		if (m_chargers[i] == nullptr)
		{
			m_chargers[i] = new Charger(this, pos);
			return;
		}
	}
}

void Game::SpawnShotgunner()
{
	Vec2 pos = GetPosOutsideScreen(SHOTGUNNER_COSMETIC_RADIUS);

	for (int i = 0; i < MAX_SHOTGUNNERS; i++)
	{
		if (m_shotgunners[i] == nullptr)
		{
			m_shotgunners[i] = new Shotgunner(this, pos);
			return;
		}
	}
}

Vec2 Game::GetPosOutsideScreen(float distanceFromScreen)
{
	Vec2 spawnPos;
	RandomNumberGenerator randomGen = RandomNumberGenerator();
	float random = randomGen.RollRandomFloatZeroToOne();
	//spawn top
	if (random < .25f)
	{
		spawnPos.x = randomGen.RollRandomFloatInRange(0.f, WORLD_SIZE_X);
		spawnPos.y = WORLD_SIZE_Y + distanceFromScreen;
	}
	//spawn Bottom
	else if (random > .25f && random < .5f)
	{
		spawnPos.x = randomGen.RollRandomFloatInRange(0.f, WORLD_SIZE_X);
		spawnPos.y = 0 - distanceFromScreen;
	}
	//spawn left
	else if (random > .5f && random < .75f)
	{
		spawnPos.x = -distanceFromScreen;
		spawnPos.y =  randomGen.RollRandomFloatInRange(0.f, WORLD_SIZE_Y);
	}
	//spawn right
	else
	{
		spawnPos.x = distanceFromScreen + WORLD_SIZE_X;
		spawnPos.y =  randomGen.RollRandomFloatInRange(0.f, WORLD_SIZE_Y);
	}
	return spawnPos;
}

void Game::SpawnWave(int waveIdx)
{
	m_currEntities = 0;
	g_theAudio->StartSound(SOUND_ID_SPAWN_WAVE);
	Wave* currWave = m_waves[waveIdx];
	for (int i = 0; i < currWave->numUnitTypes; i++)
	{
		if (currWave->units[i].unitType != 'A')
		{
			m_currEntities += currWave->units[i].ammount;
		}
		if (currWave->units[i].unitType == 'A')
		{
			int asteroidsToSpawn = currWave->units[i].ammount - m_currAsteroidsAlive;
			if (asteroidsToSpawn > 0)
				m_currAsteroidsAlive += asteroidsToSpawn;
			for (int numUnits = 0; numUnits <asteroidsToSpawn; numUnits++)
				SpawnAsteroid();
		}
		if (currWave->units[i].unitType == 'B')
		{
			for (int numUnits = 0; numUnits < currWave->units[i].ammount; numUnits++)
				SpawnBeetle();
		}
		if (currWave->units[i].unitType == 'W')
		{
			for (int numUnits = 0; numUnits < currWave->units[i].ammount; numUnits++)
				SpawnWasp();
		}
		if (currWave->units[i].unitType == 'S')
		{
			for (int numUnits = 0; numUnits < currWave->units[i].ammount; numUnits++)
				SpawnScout();
		}	
		if (currWave->units[i].unitType == 'C')
		{
			for (int numUnits = 0; numUnits < currWave->units[i].ammount; numUnits++)
				SpawnCharger();
		}
		if (currWave->units[i].unitType == 's')
		{
			for (int numUnits = 0; numUnits < currWave->units[i].ammount; numUnits++)
				SpawnShotgunner();
		}
	}
}

void Game::CheckForResetGame(float deltaSeconds)
{
	m_resetTime -= deltaSeconds;
	if (m_resetTime <= 0)
		g_theApp->ResetGame();
}


void Game::DamageEntity(Entity* entityToDamage, float amount)
{
	if (!entityToDamage->IsAlive())
	{
		return;
	}
	entityToDamage->m_health -= amount;
	if (entityToDamage->m_health <= 0)
	{
		entityToDamage->Die();
		if (entityToDamage->m_typeID != BULLET_ID && entityToDamage->m_typeID != PLAYER_SHIP_ID)
		{
			StartScreenShake(.5f, 1.f);
		}
	}
}


void Game::DecrementWaveEntities()
{
	m_currEntities--;
	if (m_currEntities == 0)
	{
		m_currWave++;
		if (m_currWave < NUM_WAVES)
		{
 			SpawnWave(m_currWave);
		}
		else
		{
			g_theAudio->StartSound(SOUND_ID_WIN);
			m_resetPending = true;
		}
	}
}

void Game::HandlePlayerLose()
{
	g_theAudio->StartSound(SOUND_ID_LOSE);
	m_resetPending = true;
}

bool Game::IsAttractScreen()
{
	return m_attractScreen;
}

void Game::PauseForSeconds(double seconds)
{
	if (m_secondsPaused < 0.0)
		m_secondsPaused = 0.0;
	if (m_secondsPaused < seconds)
		m_secondsPaused = seconds;
}

void Game::RenderDebug() const
{
	m_playerShip->RenderDebug();
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] == nullptr) continue;
		m_asteroids[i]->RenderDebug();
	}
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] == nullptr) continue;
		m_bullets[i]->RenderDebug();
	}
	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] == nullptr) continue;
		m_debris[i]->RenderDebug();
	}
	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] == nullptr) continue;
		m_beetles[i]->RenderDebug();
	}
	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] == nullptr) continue;
		m_wasps[i]->RenderDebug();
	}
	for (int i = 0; i < MAX_SCOUTS; i++)
	{
		if (m_scouts[i] == nullptr) continue;
		m_scouts[i]->RenderDebug();
	}
	for (int i = 0; i < MAX_CHARGERS; i++)
	{
		if (m_chargers[i] == nullptr) continue;
		m_chargers[i]->RenderDebug();
	}
	for (int i = 0; i < MAX_SHOTGUNNERS; i++)
	{
		if (m_shotgunners[i] == nullptr) continue;
		m_shotgunners[i]->RenderDebug();
	}
	for (int i = 0; i < MAX_MISSILES; i++)
	{
		if (m_missiles[i] == nullptr) continue;
		m_missiles[i]->RenderDebug();
	}
	for (int i = 0; i < MAX_AMMO_PICKUPS; i++)
	{
		if (m_ammoPickups[i] == nullptr) continue;
		m_ammoPickups[i]->RenderDebug();
	}
}

Vec2 Game::GetSeperationDir(Entity* entityToSeperate, float seperationRadius)
{
	Vec2 seperationDir;
	if (entityToSeperate->m_typeID == SCOUT_ID)
	{
		seperationDir += GetSeperationWithEntities(entityToSeperate, seperationRadius, MAX_BEETLES, m_beetles);
		seperationDir += GetSeperationWithEntities(entityToSeperate, seperationRadius, MAX_WASPS, m_wasps);
		seperationDir += GetSeperationWithEntities(entityToSeperate, seperationRadius, MAX_SCOUTS, m_scouts);
		seperationDir += GetSeperationWithEntities(entityToSeperate, seperationRadius, MAX_CHARGERS, m_chargers);
	}
	if (entityToSeperate->m_typeID == CHARGER_ID)
	{
		//seperationDir += GetSeperationWithEntities(entityToSeperate, seperationRadius, MAX_BEETLES, m_beetles);
		//seperationDir += GetSeperationWithEntities(entityToSeperate, seperationRadius, MAX_WASPS, m_wasps);
		//seperationDir += GetSeperationWithEntities(entityToSeperate, seperationRadius, MAX_SCOUTS, m_scouts);
		seperationDir += GetSeperationWithEntities(entityToSeperate, seperationRadius, MAX_CHARGERS, m_chargers);
	}
	if (entityToSeperate->m_typeID == SHOTGUNNER_ID)
	{
		seperationDir += GetSeperationWithEntities(entityToSeperate, seperationRadius, MAX_SHOTGUNNERS, m_shotgunners);
	}

	return seperationDir.GetNormalized();
}

float Game::GetDistanceToEdgeOfWorld(const Vec2& pos)
{
	Vec2 bottomLeft = m_worldCamera.GetOrthoBottomLeft();
	Vec2 topRight = m_worldCamera.GetOrthoTopRight();

	float distToLeft = pos.x - bottomLeft.x;
	float distToRight = topRight.x - pos.x;
	float distToBottom = pos.y - bottomLeft.y;
	float distToTop = topRight.y - pos.y;

	float minDistanceToEdge = 9999999.f;
	if (distToLeft < minDistanceToEdge)
		minDistanceToEdge = distToLeft;
	if (distToRight < minDistanceToEdge)
		minDistanceToEdge = distToRight;
	if (distToBottom < minDistanceToEdge)
		minDistanceToEdge = distToBottom;
	if (distToTop < minDistanceToEdge)
		minDistanceToEdge = distToTop;
	return minDistanceToEdge;
}

Entity** Game::GetTargetableEntities(int& outNumEntites)
{
	constexpr int MAX_ENTITIES_TO_QUERY = 100;
	outNumEntites = 0;
	Entity** targetableEntities = new Entity*[MAX_ENTITIES_TO_QUERY]{ nullptr };
	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] != nullptr)
		{
			targetableEntities[outNumEntites] = m_beetles[i];
			outNumEntites++;
			if (outNumEntites >= MAX_ENTITIES_TO_QUERY) 
				return targetableEntities;
		}
	}
	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] != nullptr)
		{
			targetableEntities[outNumEntites] = m_wasps[i];
			outNumEntites++;
			if (outNumEntites >= MAX_ENTITIES_TO_QUERY)
				return targetableEntities;
		}
	}
	for (int i = 0; i < MAX_SCOUTS; i++)
	{
		if (m_scouts[i] != nullptr)
		{
			targetableEntities[outNumEntites] = m_scouts[i];
			outNumEntites++;
			if (outNumEntites >= MAX_ENTITIES_TO_QUERY)
				return targetableEntities;
		}
	}
	for (int i = 0; i < MAX_SHOTGUNNERS; i++)
	{
		if (m_shotgunners[i] != nullptr)
		{
			targetableEntities[outNumEntites] = m_shotgunners[i];
			outNumEntites++;
			if (outNumEntites >= MAX_ENTITIES_TO_QUERY)
				return targetableEntities;
		}
	}
	for (int i = 0; i < MAX_CHARGERS; i++)
	{
		if (m_chargers[i] != nullptr)
		{
			targetableEntities[outNumEntites] = m_chargers[i];
			outNumEntites++;
			if (outNumEntites >= MAX_ENTITIES_TO_QUERY)
				return targetableEntities;
		}
	}
	return targetableEntities;
}

void Game::SpawnMissile(Vec2 position, Entity* target)
{
	for (int i = 0; i < MAX_MISSILES; i++)
	{
		if (m_missiles[i] == nullptr)
		{
			m_missiles[i] = new Missile(this, position, target, 2.f);
			g_theAudio->StartSound(SOUND_ID_THRUST); 
			return;
		}
	}
}

void Game::SpawnAmmoPickup(Vec2 position)
{
	for (int i = 0; i < MAX_AMMO_PICKUPS; i++)
	{
		if (m_ammoPickups[i] == nullptr)
		{
			m_ammoPickups[i] = new AmmoPickup(this, position);
			return;
		}
	}
}

void Game::ApplyLaserDamage(LaserBeam* laser, float deltaSeconds)
{
	ApplyLaserDamageToEntities(laser, MAX_BEETLES, m_beetles, deltaSeconds);
	ApplyLaserDamageToEntities(laser, MAX_WASPS, m_wasps, deltaSeconds);
	ApplyLaserDamageToEntities(laser, MAX_SCOUTS, m_scouts, deltaSeconds);
	ApplyLaserDamageToEntities(laser, MAX_SHOTGUNNERS, m_shotgunners, deltaSeconds);
	ApplyLaserDamageToEntities(laser, MAX_CHARGERS, m_chargers, deltaSeconds);
}

void Game::ApplyLaserDamageToEntities(LaserBeam* laser, int numEntities, Entity** entities, float deltaSeconds)
{
	for (int i = 0; i < numEntities; i++)
	{
		if (entities[i] != nullptr && entities[i]->IsAlive())
		{
			if (laser->IsInLaser(entities[i]))
			{
				DamageEntity(entities[i], laser->m_damagePerSecond * deltaSeconds);
			}
		}
	}
}

Entity* Game::GetMissileTarget(float& maxAlignmentWithPlayer)
{
	Entity* target = nullptr;
	GetMissileTargetFromEntities(MAX_BEETLES, m_beetles, maxAlignmentWithPlayer, target);
	GetMissileTargetFromEntities(MAX_WASPS, m_wasps, maxAlignmentWithPlayer, target);
	GetMissileTargetFromEntities(MAX_SCOUTS, m_scouts, maxAlignmentWithPlayer, target);
	GetMissileTargetFromEntities(MAX_SHOTGUNNERS, m_shotgunners, maxAlignmentWithPlayer, target);
	GetMissileTargetFromEntities(MAX_CHARGERS, m_chargers, maxAlignmentWithPlayer, target);
	return target;
}

void Game::GetMissileTargetFromEntities(int numEntities, Entity** entities, float& maxAlignmentWithPlayer, Entity*& bestTarget)
{
	Vec2 playerFoward = m_playerShip->GetForwardNormal();
	for (int i = 0; i < numEntities; i++)
	{
		if (entities[i] == nullptr || !entities[i]->IsAlive() || entities[i]->m_missileLockedOn)
		{
			continue;
		}
		Vec2 dirFromPlayerToEntity = entities[i]->m_position - m_playerShip->m_position;
		dirFromPlayerToEntity.Normalize();
		float alignemntWithPlayer = DotProduct2D(playerFoward, dirFromPlayerToEntity);
		if (alignemntWithPlayer > maxAlignmentWithPlayer)
		{
			maxAlignmentWithPlayer = alignemntWithPlayer;
			bestTarget = entities[i];
		}
	}
}

Vec2 Game::GetSeperationWithEntities(Entity* entityToSeperate, float seperationRadius, int numEntities, Entity** entityArray)
{
	Vec2 seperationWithEntities;
	float maxUnitWeight = 50.f;
	for (int i = 0; i < numEntities; i++)
	{
		if (entityArray[i] == entityToSeperate) continue;
		if (entityArray[i] != nullptr && entityArray[i]->IsAlive())
		{
			float unitDistance = GetDistance2D(entityToSeperate->m_position, entityArray[i]->m_position);
			if (unitDistance > seperationRadius) continue;
			float unitWeight = Min(seperationRadius / unitDistance, maxUnitWeight);
			Vec2 unitAvoidDir = entityToSeperate->m_position - entityArray[i]->m_position;
			unitAvoidDir.SetLength(unitWeight);
			seperationWithEntities += unitAvoidDir;
		}
	}
	return seperationWithEntities;
}

void Game::ShutDown()
{
	delete m_playerShip;
	m_playerShip = nullptr;
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr)
		{
			delete m_asteroids[i];
			m_asteroids[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			delete m_bullets[i];
			m_bullets[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] != nullptr)
		{
			delete m_beetles[i];
			m_beetles[i] = nullptr;
		}
	}
	if (m_beetles != nullptr)
	{
		delete m_beetles;
		m_beetles = nullptr;
	}

	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] != nullptr)
		{
			delete m_wasps[i];
			m_wasps[i] = nullptr;
		}
	}
	if (m_wasps != nullptr)
	{
		delete m_wasps;
		m_wasps = nullptr;
	}

	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] != nullptr)
		{
			delete m_debris[i];
			m_debris[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_SCOUTS; i++)
	{
		if (m_scouts[i] != nullptr)
		{
			delete m_scouts[i];
			m_scouts[i] = nullptr;
		}
	}
	if (m_scouts != nullptr)
	{
		delete m_scouts;
		m_scouts = nullptr;
	}

	for (int i = 0; i < MAX_CHARGERS; i++)
	{
		if (m_chargers[i] != nullptr)
		{
			delete m_chargers[i];
			m_chargers[i] = nullptr;
		}
	}
	if (m_chargers != nullptr)
	{
		delete m_chargers;
		m_chargers = nullptr;
	}

	for (int i = 0; i < MAX_SHOTGUNNERS; i++)
	{
		if (m_shotgunners[i] != nullptr)
		{
			delete m_shotgunners[i];
			m_shotgunners[i] = nullptr;
		}
	}
	if (m_shotgunners != nullptr)
	{
		delete m_shotgunners;
		m_shotgunners = nullptr;
	}

	for (int i = 0; i < MAX_MISSILES; i++)
	{
		if (m_missiles[i] != nullptr)
		{
			delete m_missiles[i];
			m_missiles[i] = nullptr;
		}
	}
	if (m_missiles != nullptr)
	{
		delete[] m_missiles;
		m_missiles = nullptr;
	}

	for (int i = 0; i < MAX_AMMO_PICKUPS; i++)
	{
		if (m_ammoPickups[i] != nullptr)
		{
			delete m_ammoPickups[i];
			m_ammoPickups[i] = nullptr;
		}
	}
	if (m_ammoPickups != nullptr)
	{
		delete[] m_ammoPickups;
		m_ammoPickups = nullptr;
	}

	for (int i = 0; i < NUM_WAVES; i++)
	{
		delete m_waves[i];
	}
}


