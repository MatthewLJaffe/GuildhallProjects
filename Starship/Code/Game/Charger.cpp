#include "Game/Charger.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Bullet.hpp"

Charger::Charger(Game* game, const Vec2& pos)
	: Entity(game, pos)
{
	m_cosmeticRadius = CHARGER_COSMETIC_RADIUS;
	m_physicsRadius = CHARGER_PHYSICS_RADIUS;
	m_health = 2;
	m_typeID = CHARGER_ID;

	m_numVerts = NUM_CHARGER_VERTS;
	m_localVerts = new Vertex_PCU[m_numVerts];
	m_tempWorldVerts = new Vertex_PCU[m_numVerts];
	InitializeLocalVerts();
}

void Charger::Update(float deltaSeconds)
{
	//set position
	Vec2 force = GetMovementDir();
	force.SetLength(m_acceleration * deltaSeconds);
	m_velocity += force;
	m_velocity = m_velocity.Clamp(m_maxSpeed);
	m_position += m_velocity * deltaSeconds;
	if (IsOffScreen())
		WrapAround();
	if (m_chasing)
	{
		Vec2 enemyToPlayerDisp = m_game->m_playerShip->m_position - m_position;
		m_orientationDegrees = enemyToPlayerDisp.GetOrientationDegrees();
		//fire
		FireAtPlayer(deltaSeconds);
	}
	else
	{
		m_orientationDegrees = m_velocity.GetOrientationDegrees();
	}
	CheckForCollision();
}

void Charger::Die()
{
	Entity::Die();
	g_theAudio->StartSound(SOUND_ID_EXPLOSION);
	for (int i = 0; i < 10; i++)
	{
		m_game->SpawnDebris(Vec2(m_position), Vec2(m_velocity), .3f, .7f, Rgba8(222, 63, 31, 255));
	}
	m_game->DecrementWaveEntities();
	if (g_randGen->RollRandomFloatZeroToOne() > .9f)
	{
		m_game->SpawnAmmoPickup(m_position);
	}
}

Vec2 Charger::GetMovementDir()
{
	Vec2 forceDir(0.f, 0.f);
	forceDir += m_game->GetSeperationDir(this, m_visionRadius) * m_seperateWeight;
	//"patrol state" move in random direction 
	if (!m_game->m_playerShip->IsAlive())
	{
		if (m_chasing)
		{
			m_chasing = false;
			m_randomMoveDir = Vec2(g_randGen->RollRandomFloatInRange(-1.f, 1.f), g_randGen->RollRandomFloatInRange(-1.f, 1.f));
			m_randomMoveDir.SetLength(1.f - m_seperateWeight);
			forceDir += m_randomMoveDir;
		}
		return m_randomMoveDir;
	}
	m_chasing = true;
	Vec2 enemyToPlayerDir = m_game->m_playerShip->m_position - m_position;
	enemyToPlayerDir.SetLength(1.f - m_seperateWeight);
	forceDir += enemyToPlayerDir;
	return forceDir;
}

void Charger::InitializeLocalVerts()
{
	m_localVerts[0] = Vertex_PCU(Vec3(1.25f, 1.25f, 0), Rgba8(222, 63, 31, 255));
	m_localVerts[1] = Vertex_PCU(Vec3(-1.25f, -1.5f, 0), Rgba8(222, 63, 31, 255));
	m_localVerts[2] = Vertex_PCU(Vec3(1.25f, -1.25f, 0), Rgba8(222, 63, 31, 255));

	m_localVerts[3] = Vertex_PCU(Vec3(1.25f, 1.25f, 0), Rgba8(222, 63, 31, 255));
	m_localVerts[4] = Vertex_PCU(Vec3(-1.25f, 1.5f, 0), Rgba8(222, 63, 31, 255));
	m_localVerts[5] = Vertex_PCU(Vec3(-1.25f, -1.5f, 0), Rgba8(222, 63, 31, 255));
}

void Charger::FireAtPlayer(float deltaSeconds)
{
	if (!m_game->m_playerShip->IsAlive() || GetDistance2D(m_game->m_playerShip->m_position, m_position) > m_attackRadius) return;

	if (m_currShootCooldown > 0)
	{
		m_currShootCooldown -= deltaSeconds;
		return;
	}

	//shoot
	m_currShootCooldown = m_shootCooldown;
	m_game->CreateBullet(m_position + Vec2::MakeFromPolarDegrees(m_orientationDegrees, 1.f), m_orientationDegrees, 15.5f, true, 1.5f, Rgba8(255, 255, 0, 255), Rgba8(222, 63, 31, 255));
}

void Charger::CheckForCollision()
{
	Bullet* collidingBullet = m_game->GetBulletCollision(this);
	if (collidingBullet != nullptr)
	{
		m_game->DamageEntity(collidingBullet);
		m_game->DamageEntity(this);
	}
}
