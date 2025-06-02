#include "Game/Particle.hpp"
#include "Game/ParticleEmitter.hpp"
#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"

Particle::Particle(ParticleEmitter const* emitter)
{
	ParticleEmitterConfig const& emitterConfig = emitter->m_config;
	m_position = emitterConfig.m_position + g_randGen->RollRandomNormalizedVec3() * emitterConfig.m_emissionRadius;
	m_velocity.x = g_randGen->RollRandomFloatInRange(emitterConfig.m_velocityXRange);
	m_velocity.y = g_randGen->RollRandomFloatInRange(emitterConfig.m_velocityYRange);
	m_velocity.z = g_randGen->RollRandomFloatInRange(emitterConfig.m_velocityZRange);
	m_liveTime = emitterConfig.m_lifetime;
}

void Particle::Update(float deltaSeconds)
{
	m_velocity += m_velocity.GetNormalized() * deltaSeconds * g_theGame->m_emitter->m_config.m_acceleration;
	if (m_velocity.GetLengthSquared() > g_theGame->m_emitter->m_config.m_maxSpeed * g_theGame->m_emitter->m_config.m_maxSpeed)
	{
		m_velocity.SetLength(g_theGame->m_emitter->m_config.m_maxSpeed);
	}
	m_position += m_velocity * deltaSeconds;
	m_liveTime -= deltaSeconds;
}

void Particle::AddVertsForParticle(std::vector<Vertex_PCU>& verts)
{
	Mat44 targetMatrix = g_theGame->m_player->GetModelMatrix();
	Mat44 billboardMatrix = GetBillboardMatrix(BillboardType::FULL_FACING, targetMatrix, m_position, Vec2::ONE);

	Vec3 bottomLeft = billboardMatrix.GetTranslation3D() - .5f * billboardMatrix.GetJBasis3D() - .5f * billboardMatrix.GetKBasis3D();
	Vec3 bottomRight = billboardMatrix.GetTranslation3D() + .5f * billboardMatrix.GetJBasis3D() - .5f * billboardMatrix.GetKBasis3D();
	Vec3 topRight = billboardMatrix.GetTranslation3D() + .5f * billboardMatrix.GetJBasis3D() + .5f * billboardMatrix.GetKBasis3D();
	Vec3 topLeft = billboardMatrix.GetTranslation3D() - .5f * billboardMatrix.GetJBasis3D() + .5f * billboardMatrix.GetKBasis3D();
	AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, g_theGame->m_emitter->m_config.m_color, g_theGame->m_emitter->m_config.m_uvs);
}

void Particle::operator=(Particle const& particle)
{
	m_liveTime = particle.m_liveTime;
	m_position = particle.m_position;
	m_velocity = particle.m_velocity;
}
