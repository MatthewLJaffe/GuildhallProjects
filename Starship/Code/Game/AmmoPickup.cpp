#include "Game/AmmoPickup.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Game.hpp"
#include "Game/LaserBeam.hpp"

AmmoPickup::AmmoPickup(Game* game, Vec2 startPos)
	: Entity(game, startPos)
{
	if (g_randGen->RollRandomFloatZeroToOne() > .5f)
	{
		m_ammoType = AMMO_MISSILE;
	}
	else
	{
		m_ammoType = AMMO_LASER;
	}
	m_cosmeticRadius = 1.5f;
	m_physicsRadius = 1.5f;
	m_typeID = AMMO_ID;
	m_numVerts = NUM_MISSILE_VERTS;
	m_localVerts = new Vertex_PCU[m_numVerts];
	m_tempWorldVerts = new Vertex_PCU[m_numVerts];
	InitializeLocalVerts();
}

void AmmoPickup::InitializeLocalVerts()
{
	Rgba8 tipColor = Rgba8(245, 66, 87, 255);
	Rgba8 baseColor = Rgba8(162, 173, 176, 255);
	Rgba8 tailColor = Rgba8(77, 58, 65, 255);

	m_localVerts[0] = Vertex_PCU(Vec3(1.5f, 0, 0), tipColor);
	m_localVerts[1] = Vertex_PCU(Vec3(1.f, .5f, 0), tipColor);
	m_localVerts[2] = Vertex_PCU(Vec3(1.f, -.5f, 0), tipColor);

	m_localVerts[3] = Vertex_PCU(Vec3(1.f, .5f, 0), baseColor);
	m_localVerts[4] = Vertex_PCU(Vec3(-1.f, .5f, 0), baseColor);
	m_localVerts[5] = Vertex_PCU(Vec3(-1.f, -.5f, 0), baseColor);

	m_localVerts[6] = Vertex_PCU(Vec3(1.f, -.5f, 0), baseColor);
	m_localVerts[7] = Vertex_PCU(Vec3(1.f, .5f, 0), baseColor);
	m_localVerts[8] = Vertex_PCU(Vec3(-1.f, -.5f, 0), baseColor);

	m_localVerts[9] = Vertex_PCU(Vec3(-1.f, .5f, 0), tailColor);
	m_localVerts[10] = Vertex_PCU(Vec3(-2.f, .75f, 0), tailColor);
	m_localVerts[11] = Vertex_PCU(Vec3(-1.f, 0.f, 0), tailColor);

	m_localVerts[12] = Vertex_PCU(Vec3(-1.f, -.5f, 0), tailColor);
	m_localVerts[13] = Vertex_PCU(Vec3(-1.f, 0.f, 0), tailColor);
	m_localVerts[14] = Vertex_PCU(Vec3(-2.f, -.75f, 0), tailColor);
}

void AmmoPickup::Update(float deltaSeconds)
{
	m_liveTime -= deltaSeconds;
	if (m_liveTime < 0.f)
	{
		Die();
		return;
	}
	if (m_liveTime <= 5.f)
	{
		m_alphaNormalized = Lerp(0.f, 1.f, m_liveTime / 5.f);
	}
	if (DoDiscsOverlap(m_position, m_physicsRadius, m_game->m_playerShip->m_position, m_game->m_playerShip->m_physicsRadius))
	{
		if (m_ammoType == AMMO_MISSILE)
		{
			m_game->m_playerShip->AddMissileAmmo(10);
		}
		else if (m_ammoType == AMMO_LASER)
		{
			m_game->m_playerShip->AddLaserAmmo(5.f);
		}
		Die();
	}
}

void AmmoPickup::Render() const
{
	unsigned char alpha = static_cast<unsigned char>(m_alphaNormalized * 255.f);
	if (m_ammoType == AMMO_LASER)
	{
		DebugDrawDisc(m_position, m_cosmeticRadius, Rgba8(196, 81, 81, alpha));
		DebugDrawLine(m_position - Vec2(1.f, 0), m_position + Vec2(1.f, 0), .25f,  Rgba8(12, 198, 240, alpha));
	}
	if (m_ammoType == AMMO_MISSILE)
	{
		DebugDrawDisc(m_position, m_cosmeticRadius, Rgba8(50, 141, 194, alpha));
		for (int i = 0; i < m_numVerts; i++)
		{
			m_tempWorldVerts[i] = m_localVerts[i];
			m_tempWorldVerts[i].m_color.a = alpha;
		}
		TransformVertexArrayXY3D(NUM_MISSILE_VERTS, m_tempWorldVerts, .5f, m_orientationDegrees, m_position);
		g_theRenderer->DrawVertexArray(NUM_MISSILE_VERTS, m_tempWorldVerts);
	}
}

