#include "Game/Prop.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"

Prop::Prop(Game* game, Vec3 const& startPos, bool isPhysicsObject)
	: Entity(game, startPos)
	, m_isPhysicsObject(isPhysicsObject)
{
	if (m_isPhysicsObject)
	{
		m_physicsObject = g_theParticleSystem->AddParticlePhysicsObject(m_position, 2.f, 500.f, 2.f, false);
	}
}

Prop::~Prop()
{
	if (m_physicsObject != nullptr)
	{
		delete m_physicsObject;
	}
}

void Prop::Update(float deltaSeconds)
{
	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees.m_yaw += m_angularVelocity.m_yaw * deltaSeconds; 
	m_orientationDegrees.m_pitch += m_angularVelocity.m_pitch * deltaSeconds;
	m_orientationDegrees.m_roll += m_angularVelocity.m_roll * deltaSeconds;
	if (m_hasLifetime)
	{
		m_liveTime -= deltaSeconds;
		if (m_liveTime < 0.f)
		{
			m_isAlive = false;
		}
	}
	if (m_isPhysicsObject)
	{
		m_physicsObject->SetPosition(m_position);
	}
}

void Prop::Render() const
{
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray(m_vertexes.size(), m_vertexes.data());
}
