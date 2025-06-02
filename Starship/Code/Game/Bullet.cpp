#include "Game/Bullet.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"

Bullet::Bullet(Game* game, Vec2 startPos, float rotation)
	: Entity(game, startPos)
{
	m_centerColor = Rgba8(255, 255, 0, 255);
	m_tailColor = Rgba8(255, 0, 0, 255);

	m_orientationDegrees = rotation;
	m_velocity.SetPolarDegrees(m_orientationDegrees, m_bulletSpeed);
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_typeID = BULLET_ID;
	m_numVerts = NUM_BULLET_VERTS;
	m_localVerts = new Vertex_PCU[m_numVerts];
	m_tempWorldVerts = new Vertex_PCU[m_numVerts];
	InitializeLocalVerts();
}

Bullet::Bullet(Game* game, Vec2 startPos, float rotation, float speed, bool targetPlayer, float scale, Rgba8 centerColor, Rgba8 tailColor)
	: Entity(game, startPos)
{
	m_bulletSpeed = speed;
	m_targetPlayer = targetPlayer;
	m_scale = scale;
	m_centerColor = centerColor;
	m_tailColor = tailColor;

	m_orientationDegrees = rotation;
	m_velocity.SetPolarDegrees(m_orientationDegrees, m_bulletSpeed);
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_typeID = BULLET_ID;
	m_numVerts = NUM_BULLET_VERTS;
	m_localVerts = new Vertex_PCU[m_numVerts];
	m_tempWorldVerts = new Vertex_PCU[m_numVerts];
	InitializeLocalVerts();
}

void Bullet::InitializeLocalVerts()
{
	m_localVerts[0] = Vertex_PCU(Vec3(0, .5f, 0), m_centerColor);
	m_localVerts[1] = Vertex_PCU(Vec3(0, -.5f, 0), m_centerColor);
	m_localVerts[2] = Vertex_PCU(Vec3(.5f, 0, 0), m_centerColor);

	m_localVerts[3] = Vertex_PCU(Vec3(0, .5f, 0), m_tailColor);
	m_localVerts[4] = Vertex_PCU(Vec3(-2, 0, 0), m_tailColor);
	m_localVerts[5] = Vertex_PCU(Vec3(0, -.5f, 0), m_tailColor);
	m_localVerts[4].m_color.a = 0;
}

void Bullet::Update(float deltaSeconds)
{
	if (!IsAlive()) return;
	m_position += m_velocity * deltaSeconds;
	m_currBulletLiveTime += deltaSeconds;
	Asteroid* collidingAsteroid = m_game->GetAsteroidCollision(this);
	if (collidingAsteroid != nullptr)
	{
		m_game->DamageEntity(collidingAsteroid);
		m_game->DamageEntity(this);
	}
	if (m_currBulletLiveTime >= m_bulletLiveTime || IsOffScreen())
	{
		DieWithoutParticles();
	}
	if (m_targetPlayer)
	{
		PlayerShip* playerShip = m_game->GetPlayerCollision(this);
		if (playerShip != nullptr)
		{
			m_game->DamageEntity(playerShip);
		}
	}
}

void Bullet::Render() const
{
	if (!IsAlive()) return;
	Vertex_PCU tempWorldVerts[NUM_BULLET_VERTS];
	for (int i = 0; i < NUM_BULLET_VERTS; i++)
	{
		tempWorldVerts[i] = m_localVerts[i];
	}
	TransformVertexArrayXY3D(NUM_BULLET_VERTS, tempWorldVerts, m_scale, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_BULLET_VERTS, tempWorldVerts);
}

void Bullet::DieWithoutParticles()
{
	m_isDead = true;
	m_isGarbage = true;
}

void Bullet::Die()
{
	g_theAudio->StartSound(SOUND_ID_HIT);
	for (int i = 0; i < 3; i++)
	{
		m_game->SpawnDebris(Vec2(m_position), Vec2(m_velocity * -.5f), .1f, .2f, Rgba8(255, 255, 0, 255));
	}
	m_isDead = true;
	m_isGarbage = true;
}