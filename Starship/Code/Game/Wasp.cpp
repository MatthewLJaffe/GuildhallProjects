#include "Game/Wasp.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Bullet.hpp"

Wasp::Wasp(Game* game, const Vec2& pos)
	: Entity(game, pos)
{
	m_physicsRadius = WASP_PHYSICS_RADIUS;
	m_cosmeticRadius = WASP_COSMETIC_RADIUS;
	m_health = 2;
	m_typeID = WASP_ID;
	m_numVerts = NUM_WASP_VERTS;
	m_localVerts = new Vertex_PCU[m_numVerts];
	m_tempWorldVerts = new Vertex_PCU[m_numVerts];
	InitializeLocalVerts();
}

void Wasp::Update(float deltaSeconds)
{
	if (m_game->m_playerShip->IsAlive())
	{
		Vec2 dirToPlayer = m_game->m_playerShip->m_position - m_position;
		m_acceleration = (dirToPlayer).GetNormalized() * WASP_ACCELERATION_FORCE;
		m_velocity += m_acceleration * deltaSeconds;
		m_velocity.ClampLength(WASP_SPEED);
		m_orientationDegrees = dirToPlayer.GetOrientationDegrees();
	}
	m_position += m_velocity * deltaSeconds;
	Bullet* collidingBullet = m_game->GetBulletCollision(this);
	if (collidingBullet != nullptr)
	{
		m_game->DamageEntity(collidingBullet);
		m_game->DamageEntity(this);
	}
	PlayerShip* playerShip = m_game->GetPlayerCollision(this);
	if (playerShip != nullptr)
	{
		m_game->DamageEntity(playerShip);
	}
}

void Wasp::Render() const
{
	if (!IsAlive()) return;
	Vertex_PCU tempWorldVerts[NUM_WASP_VERTS];
	for (int i = 0; i < NUM_WASP_VERTS; i++)
	{
		tempWorldVerts[i] = m_localVerts[i];
	}
	TransformVertexArrayXY3D(NUM_WASP_VERTS, tempWorldVerts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_WASP_VERTS, tempWorldVerts);
}


void Wasp::Die()
{
	Entity::Die();
	g_theAudio->StartSound(SOUND_ID_EXPLOSION);
	for (int i = 0; i < 10; i++)
	{
		m_game->SpawnDebris(Vec2(m_position), Vec2(m_velocity), .3f, .7f, Rgba8(255, 255, 0, 255));
	}
	m_game->DecrementWaveEntities();
	if (g_randGen->RollRandomFloatZeroToOne() > .9f)
	{
		m_game->SpawnAmmoPickup(m_position);
	}
}

void Wasp::InitializeLocalVerts()
{
	m_localVerts[0] = Vertex_PCU(Vec3(0, 1.75, 0), Rgba8(255, 255, 0, 255));
	m_localVerts[1] = Vertex_PCU(Vec3(0, -1.75, 0), Rgba8(255, 255, 0, 255));
	m_localVerts[2] = Vertex_PCU(Vec3(1.75, 0, 0), Rgba8(255, 255, 0, 255));

	m_localVerts[3] = Vertex_PCU(Vec3(0, 1, 0), Rgba8(255, 255, 0, 255));
	m_localVerts[4] = Vertex_PCU(Vec3(-1.75, 0, 0), Rgba8(255, 255, 0, 255));
	m_localVerts[5] = Vertex_PCU(Vec3(0, -1, 0), Rgba8(255, 255, 0, 255));
}