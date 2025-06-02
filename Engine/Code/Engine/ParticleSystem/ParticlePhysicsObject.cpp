#include "Engine/ParticleSystem/ParticlePhysicsObject.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"

ParticlePhysicsObject::ParticlePhysicsObject(int physicsObjectGPUIndex)
	: m_physicsObjectGPUIndex(physicsObjectGPUIndex)
{
}

void ParticlePhysicsObject::SetPosition(Vec3 const& position)
{
	g_theParticleSystem->m_livePhysicsObjects[m_physicsObjectGPUIndex].m_position = position;
}

void ParticlePhysicsObject::SetForceMagnitude(float forceMagnitude)
{
	g_theParticleSystem->m_livePhysicsObjects[m_physicsObjectGPUIndex].m_forceMagnitude = forceMagnitude;
}

void ParticlePhysicsObject::SetRadius(float radius)
{
	g_theParticleSystem->m_livePhysicsObjects[m_physicsObjectGPUIndex].m_radius = radius;
}

void ParticlePhysicsObject::SetFalloff(float falloff)
{
	g_theParticleSystem->m_livePhysicsObjects[m_physicsObjectGPUIndex].m_falloffExponent = falloff;
}

ParticlePhysicsObject::~ParticlePhysicsObject()
{
	g_theParticleSystem->m_livePhysicsObjects[m_physicsObjectGPUIndex].m_isActive = 0;
}

ParticlePhysicsAABB3::~ParticlePhysicsAABB3()
{
	m_physicsAABB3GPU.m_isActive = 0;
}

void ParticlePhysicsAABB3::SetCenterPosition(Vec3 const& position)
{
	
	Vec3 currentCenter = (m_physicsAABB3GPU.m_mins + m_physicsAABB3GPU.m_maxs) * .5f;
	Vec3 displacmentToNewCenter = position - currentCenter;
	m_physicsAABB3GPU.m_mins += displacmentToNewCenter;
	m_physicsAABB3GPU.m_maxs += displacmentToNewCenter;
}

void ParticlePhysicsAABB3::SetScale(Vec3 const& scale)
{
	Vec3 currentCenter = (m_physicsAABB3GPU.m_mins + m_physicsAABB3GPU.m_maxs) * .5f;
	m_physicsAABB3GPU.m_mins = currentCenter - (.5f*scale);
	m_physicsAABB3GPU.m_maxs = currentCenter + (.5f * scale);
}

void ParticlePhysicsAABB3::SetMins(Vec3 const& mins)
{
	m_physicsAABB3GPU.m_mins = mins;
}

void ParticlePhysicsAABB3::SetMaxs(Vec3 const& maxs)
{
	m_physicsAABB3GPU.m_maxs = maxs;
}

ParticlePhysicsAABB3::ParticlePhysicsAABB3(ParticlePhysicsAABB3GPU& phsicsAABB3GPU)
	: m_physicsAABB3GPU(phsicsAABB3GPU)
{
}
