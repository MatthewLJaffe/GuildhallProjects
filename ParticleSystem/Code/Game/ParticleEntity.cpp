#include "Game/ParticleEntity.hpp"
#include "Game/Scene.hpp"

ParticleEntity::ParticleEntity(Game* game, const Vec3& startPos, EulerAngles const& orientation, ParticleEffect* effect, ParticlePhysicsObject* physObject, float lifeTime)
	: Entity(game, startPos, orientation)
	, m_effect(effect)
	, m_currentLifeTime(lifeTime)
	, m_physicsObject(physObject)
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
	m_position += m_velocity * deltaSeconds;
	m_effect->SetPosition(m_position);
	m_effect->SetOrientationDegrees(m_orientationDegrees);
	

	if (m_physicsObject != nullptr)
	{
		m_physicsObject->SetPosition(m_position);
	}

	std::vector<Entity*>& entities = m_game->GetCurrentScene()->m_sceneEntities;
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i] == nullptr)
		{
			continue;
		}
		if (entities[i]->m_physicsBounds.GetDimensions().GetLength() > 0.f)
		{
			if (DoAABBsOverlap3D(m_physicsBounds.GetTranslated(m_position), entities[i]->m_physicsBounds.GetTranslated(entities[i]->m_position)))
			{
				m_isAlive = false;
			}

		}

	}
}

void ParticleEntity::Render() const
{
}

ParticleEntity::~ParticleEntity()
{
	g_theParticleSystem->PlayParticleEffectByFileName("Data/Saves/ParticleEffects/IceLanceBurst.xml", m_position, EulerAngles(), 3.f);
	delete m_effect;
	if (m_physicsObject != nullptr)
	{
		delete m_physicsObject;
	}
}
