#include "TankProjectile.hpp"
#include "Game/Model.hpp"
#include "Game/Model.hpp"

TankProjectile::TankProjectile(Game* game, Vec3 const& startPos, bool isBlueTeam)
	: Entity(game, startPos)
	, m_isBlueTeam(isBlueTeam)
{
	m_hasLifetime = true;
	m_startPos = startPos;
	if (m_isBlueTeam)
	{
		AddVertsForSphere3D(m_vertexes, Vec3::ZERO, .25f, Rgba8(70, 70, 255, 255));
	}
	else
	{
		AddVertsForSphere3D(m_vertexes, Vec3::ZERO, .25f, Rgba8(255, 70, 70, 255));
	}
}

TankProjectile::~TankProjectile()
{
	delete m_physicsObject;
}

void TankProjectile::Update(float deltaSeconds)
{
	if (GetDistanceSquared3D(m_startPos, m_position) > m_travelDistance * m_travelDistance)
	{
		if (m_physicsObject == nullptr)
		{
			m_tankToDestroy->m_isActive = false;
			m_physicsObject = g_theParticleSystem->AddParticlePhysicsObject(m_position - Vec3(0.f, 0.f, .75f), 2.5f, 800.f);
			if (m_isBlueTeam)
			{
				g_theParticleSystem->PlayParticleEffectByFileName("Data/Saves/ParticleEffects/RedTankDestroyEffect.xml", m_tankToDestroy->m_position, m_tankToDestroy->m_orientationDegrees, 2.f);
				g_theParticleSystem->PlayParticleEffectByFileName("Data/Saves/ParticleEffects/EnergyExplosion.xml", m_position, EulerAngles(), 2.f);
			}
			else
			{
				g_theParticleSystem->PlayParticleEffectByFileName("Data/Saves/ParticleEffects/BlueTankDestroyEffect.xml", m_tankToDestroy->m_position, m_tankToDestroy->m_orientationDegrees, 2.f);
				g_theParticleSystem->PlayParticleEffectByFileName("Data/Saves/ParticleEffects/Explosion.xml", m_position, EulerAngles(), 2.f);
			}
		}
	}
	else
	{
		m_position += m_velocity * deltaSeconds;
		m_orientationDegrees.m_yaw += m_angularVelocity.m_yaw * deltaSeconds;
		m_orientationDegrees.m_pitch += m_angularVelocity.m_pitch * deltaSeconds;
		m_orientationDegrees.m_roll += m_angularVelocity.m_roll * deltaSeconds;
	}

	if (m_hasLifetime)
	{
		m_liveTime -= deltaSeconds;
		if (m_liveTime < 0.f)
		{
			m_isAlive = false;
		}
	}
}

void TankProjectile::Render() const
{
	if (GetDistanceSquared3D(m_startPos, m_position) > m_travelDistance * m_travelDistance)
	{
		return;
	}

	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(m_vertexes.size(), m_vertexes.data());
}
