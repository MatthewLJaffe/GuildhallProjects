#include "ParticleEntity.hpp"

ParticleEntity::ParticleEntity(const Vec3& startPos, EulerAngles const& orientation, ParticleEffect* effect, float lifeTime)
	: Entity(startPos, orientation)
	, m_effect(effect)
	, m_currentLifeTime(lifeTime)
{
}

ParticleEntity::ParticleEntity(const Vec3& startPos, Mat44 const& orientation, ParticleEffect* effect, float lifeTime)
	: Entity(startPos, orientation)
	, m_effect(effect)
	, m_currentLifeTime(lifeTime)
{
}

void ParticleEntity::Update(float deltaSeconds)
{
	if (m_currentLifeTime != -1.f)
	{
		m_currentLifeTime -= deltaSeconds;
		if (m_currentLifeTime < 0.f)
		{
			m_isAlive = false;
		}

	}

	Translate(m_velocity * deltaSeconds);
	SetLocalPosition(GetLocalPosition() + m_velocity * deltaSeconds);
	Mat44 transform = GetWorldTransform();
	m_effect->SetWorldTransform(transform);
}

void ParticleEntity::Render() const
{
}

ParticleEntity::~ParticleEntity()
{
	delete m_effect;
}
