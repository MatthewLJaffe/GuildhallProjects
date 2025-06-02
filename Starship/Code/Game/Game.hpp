#pragma once
class PlayerShip;
class Asteroid;
class Entity;
class Bullet;
class Beetle;
class Wasp;
class Debris;
class Scout;
class LaserBeam;

#include "Game/GameCommon.hpp"
struct UnitAmmount
{
	UnitAmmount(int numEntities, unsigned char unitType);
	unsigned char unitType;
	int ammount;
};


struct Wave
{
	Wave(int numUnits, UnitAmmount* unitAmounts);
	~Wave();
	UnitAmmount* units = nullptr;
	int numUnitTypes = 0;
};

constexpr int MAX_ENTITIES_TO_SHAKE = 50;

class Game
{
public:
	Game(App* app);
	~Game();
	Bullet* GetBulletCollision(Entity* entity);
	Asteroid* GetAsteroidCollision(Entity* entity);
	PlayerShip* GetPlayerCollision(Entity* entity);
	void StartUp();
	void StartScreenShake(float shakeTime, float shakeIntensity);
	void Update(float deltaSeconds);
	void UpdateEntities(float deltaTime);
	void Render() const;
	void RenderEntities() const;
	void DeleteGarbageEntities();
	void SpawnDebris(const Vec2& pos, const Vec2& startingVelocity, float minSize, float maxSize, const Rgba8& color);
	void ShutDown();
	void CreateBullet(Vec2 pos, float rotation, float speed, bool targetPlayer, float scale, Rgba8 centerColor, Rgba8 tailColor);
	void CreateBullet(Vec2 pos, float rotation);
	void DamageEntity(Entity* entityToDamage, float amount = 1.f);
	void DecrementWaveEntities();
	void HandlePlayerLose();
	bool IsAttractScreen();
	void PauseForSeconds(double seconds);
	Vec2 GetSeperationDir(Entity* entityToSeperate, float m_seperationRadius);
	float GetDistanceToEdgeOfWorld(const Vec2& pos);
	Entity** GetTargetableEntities(int& outNumEntites);
	void SpawnMissile(Vec2 position, Entity* target);
	void SpawnAmmoPickup(Vec2 position);
	void ApplyLaserDamage(LaserBeam* laser, float deltaSeconds);
	void ApplyLaserDamageToEntities(LaserBeam* laser, int numEntities, Entity** entities, float deltaSeconds);
	Entity* GetMissileTarget(float& maxAlignmentWithPlayer);
	void GetMissileTargetFromEntities(int numEntities, Entity** entities, float& maxAlignmentWithPlayer, Entity*& bestTarget);

	PlayerShip* m_playerShip = nullptr;
	Asteroid* m_asteroids[MAX_ASTEROIDS] = { nullptr };
	Bullet* m_bullets[MAX_BULLETS] = { nullptr };
	Entity** m_beetles;
	Entity** m_wasps;
	Entity** m_scouts;
	Entity** m_chargers;
	Entity** m_shotgunners;
	Entity** m_missiles;
	Entity** m_ammoPickups;
	Debris* m_debris[MAX_DEBRIS] = { nullptr };
	Wave* m_waves[NUM_WAVES];
	int m_currAsteroidsAlive = 0;


private:
	void StartGame();
	void UpdateScreenShake(float deltaSeconds);
	void RenderStartScreen() const;
	void SpawnAsteroid();
	void SpawnBeetle();
	void SpawnWasp();
	void SpawnScout();
	void SpawnCharger();
	void SpawnShotgunner();
	void RenderDebug() const;
	Vec2 GetPosOutsideScreen(float distanceFromScreen);
	void SpawnWave(int waveIdx);
	void CheckForResetGame(float deltaSeconds);
	Vec2 GetSeperationWithEntities(Entity* entityToSeperate, float seperationRadius, int numEntities, Entity** entityArray);
	App* m_theApp;
	int m_currEntities = 0;
	int m_currWave = 0;
	bool m_attractScreen = true;
	Vertex_PCU m_attractShipVerts[NUM_SHIP_VERTS];
	Vertex_PCU m_attractPlayVerts[3];
	float m_attractScreenAlpha = 1.f;
	float m_attractAlphaRate = .5f;
	float m_shipOscillateTheta = 0.f;
	Vec2 m_attractLeftShipStartPos;
	Vec2 m_attractLeftShipCurrPos;
	Vec2 m_attractLeftShipEndPos;
	Vec2 m_attractRightShipStartPos;
	Vec2 m_attractRightShipCurrPos;
	Vec2 m_attractRightShipEndPos;
	bool m_resetPending = false;
	float m_resetTime = 3.f;
	Camera m_worldCamera;
	Camera m_screenCamera;
	float m_currShakeTime = 0.f;
	float m_totalShakeTime = 0.f;
	float m_shakeTrauma = 0.f;
	double m_secondsPaused = 0.f;
};

