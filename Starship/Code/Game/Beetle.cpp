#include "Game/Beetle.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Bullet.hpp"

Beetle::Beetle(Game* game, const Vec2& pos)
	: Entity(game, pos)
{
	m_physicsRadius = BEETLE_PHYSICS_RADIUS;
	m_cosmeticRadius = BEETLE_COSMETIC_RADIUS;
	m_health = 2;
	m_typeID = BEETLE_ID;
	m_numVerts = NUM_BEETLE_VERTS;
	m_localVerts = new Vertex_PCU[m_numVerts];
	m_tempWorldVerts = new Vertex_PCU[m_numVerts];
	InitializeLocalVerts();
}

void Beetle::Update(float deltaSeconds)
{
	if (m_game->m_playerShip->IsAlive())
	{
		m_velocity = (m_game->m_playerShip->m_position - m_position).GetNormalized() * BEETLE_SPEED;
		m_orientationDegrees = m_velocity.GetOrientationDegrees();
	}
	m_position += m_velocity * deltaSeconds;
	Bullet* collidingBullet = m_game->GetBulletCollision(this);
	if (collidingBullet != nullptr)
	{
		m_game->DamageEntity(collidingBullet);
		m_game->DamageEntity(this);
	}
	PlayerShip* playerShip  = m_game->GetPlayerCollision(this);
	if (playerShip != nullptr)
	{
		m_game->DamageEntity(playerShip);
	}
}

void Beetle::Render() const
{
	if (!IsAlive()) return;
	Vertex_PCU tempWorldVerts[NUM_BEETLE_VERTS];
	for (int i = 0; i < NUM_BEETLE_VERTS; i++)
	{
		tempWorldVerts[i] = m_localVerts[i];
	}
	TransformVertexArrayXY3D(NUM_BEETLE_VERTS, tempWorldVerts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_BEETLE_VERTS, tempWorldVerts);
}


void Beetle::Die()
{
	Entity::Die();
	g_theAudio->StartSound(SOUND_ID_EXPLOSION);
	for (int i = 0; i < 10; i++)
	{
		m_game->SpawnDebris(Vec2(m_position), Vec2(m_velocity), .3f, .7f, Rgba8(100, 160, 60, 255));
	}
	m_game->DecrementWaveEntities();
	if (g_randGen->RollRandomFloatZeroToOne() > .9f)
	{
		m_game->SpawnAmmoPickup(m_position);
	}
}

void Beetle::InitializeLocalVerts()
{
	m_localVerts[0] = Vertex_PCU(Vec3(1.25f, 1.25f, 0), Rgba8(100, 160, 60, 255));
	m_localVerts[1] = Vertex_PCU(Vec3(-1.25f, -1.5f, 0), Rgba8(100, 160, 60, 255));
	m_localVerts[2] = Vertex_PCU(Vec3(1.25f, -1.25f, 0), Rgba8(100, 160, 60, 255));

	m_localVerts[3] = Vertex_PCU(Vec3(1.25f, 1.25f, 0), Rgba8(100, 160, 60, 255));
	m_localVerts[4] = Vertex_PCU(Vec3(-1.25f, 1.5f, 0), Rgba8(100, 160, 60, 255));
	m_localVerts[5] = Vertex_PCU(Vec3(-1.25f, -1.5f, 0), Rgba8(100, 160, 60, 255));

}