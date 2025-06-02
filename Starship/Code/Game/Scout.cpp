#include "Game/Scout.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Bullet.hpp"

Scout::Scout(Game* game, const Vec2& pos)
	: Entity(game, pos)
{
	m_cosmeticRadius = SCOUT_COSMETIC_RADIUS;
	m_physicsRadius = SCOUT_PHYSICS_RADIUS;
	m_health = 2;
	m_typeID = SCOUT_ID;

	m_numVerts = NUM_SCOUT_VERTS;
	m_localVerts = new Vertex_PCU[m_numVerts];
	m_tempWorldVerts = new Vertex_PCU[m_numVerts];
	InitializeLocalVerts();
}


void Scout::Update(float deltaSeconds)
{
	m_velocity = GetMovement();
	if (m_game->m_playerShip->IsAlive())
		m_orientationDegrees = (m_game->m_playerShip->m_position - m_position).GetOrientationDegrees();
	else
		m_orientationDegrees = m_velocity.GetOrientationDegrees();
	m_position += m_velocity * deltaSeconds;
	if (IsOffScreen())
		WrapAround();
	if (m_velocity.GetLength() < .1f)
		FireAtPlayer(deltaSeconds);
	CheckForCollision();
}

void Scout::Die()
{
	Entity::Die();
	g_theAudio->StartSound(SOUND_ID_EXPLOSION);
	for (int i = 0; i < 10; i++)
	{
		m_game->SpawnDebris(Vec2(m_position), Vec2(m_velocity), .3f, .7f, Rgba8(143, 218, 219, 255));
	}
	m_game->DecrementWaveEntities();
	if (g_randGen->RollRandomFloatZeroToOne() > .9f)
	{
		m_game->SpawnAmmoPickup(m_position);
	}
}

void Scout::CheckForCollision()
{
	Bullet* collidingBullet = m_game->GetBulletCollision(this);
	if (collidingBullet != nullptr)
	{
		m_game->DamageEntity(collidingBullet);
		m_game->DamageEntity(this);
	}
}

void Scout::InitializeLocalVerts()
{
	m_localVerts[0] = Vertex_PCU(Vec3(1.25f, 1.25f, 0), Rgba8(143, 218, 219, 255));
	m_localVerts[1] = Vertex_PCU(Vec3(-1.25f, -1.5f, 0), Rgba8(143, 218, 219, 255));
	m_localVerts[2] = Vertex_PCU(Vec3(1.25f, -1.25f, 0), Rgba8(143, 218, 219, 255));

	m_localVerts[3] = Vertex_PCU(Vec3(1.25f, 1.25f, 0), Rgba8(143, 218, 219, 255));
	m_localVerts[4] = Vertex_PCU(Vec3(-1.25f, 1.5f, 0), Rgba8(143, 218, 219, 255));
	m_localVerts[5] = Vertex_PCU(Vec3(-1.25f, -1.5f, 0), Rgba8(143, 218, 219, 255));
}

Vec2 Scout::GetMovement()
{
	Vec2 velocity;
	//move in direction if the player is dead
	if (!m_game->m_playerShip->IsAlive())
	{
		velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, 1.f - m_seperateWeight);
		velocity += m_game->GetSeperationDir(this, m_visionRadius) * m_seperateWeight;
		velocity.SetLength(m_maxSpeed);
		return velocity;
	}
	//if player is too close run away
	float playerAvoidDistance = 20.f;
	float exitFleeDistance = 40.f;
	Vec2 playerToScoutDisp = m_position - m_game->m_playerShip->m_position;
	if ( playerToScoutDisp.GetLength() < playerAvoidDistance || (m_isFleeing && playerToScoutDisp.GetLength() < exitFleeDistance) )
	{
		m_isFleeing = true;
		velocity += playerToScoutDisp.GetNormalized() * (1.f - m_seperateWeight);
	}
	else 
	{
		m_isFleeing = false;
		//move towards player if too close to edge of screen
		if (m_game->GetDistanceToEdgeOfWorld(m_position) < m_preferedDistanceFromOffScreen)
		{
			Vec2 scoutToPlayerDisp = -playerToScoutDisp.GetNormalized();
			velocity += scoutToPlayerDisp * (1.f - m_seperateWeight);
		}
	}
	velocity += m_game->GetSeperationDir(this, m_visionRadius) * m_seperateWeight;
	velocity.SetLength(m_maxSpeed);
	return velocity;
}

void Scout::FireAtPlayer(float deltaSeconds)
{
	if (!m_game->m_playerShip->IsAlive()) return;

	if (m_currShootCooldown > 0)
	{
		m_currShootCooldown -= deltaSeconds;
		return;
	}

	//shoot
	m_currShootCooldown = m_shootCooldown;
	m_game->CreateBullet(m_position + Vec2::MakeFromPolarDegrees(m_orientationDegrees, 1.f), m_orientationDegrees, 8.f, true, 1.f, Rgba8(255, 255, 0, 255), Rgba8(143, 218, 219, 255));
}
