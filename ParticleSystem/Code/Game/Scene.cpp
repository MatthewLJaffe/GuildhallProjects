#include "Game/Scene.hpp"
#include "Game/Entity.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"

void Scene::StartUp()
{
}

void Scene::SwitchOn()
{
}

void Scene::SwitchOff()
{
	for (int i = 0; i < (int)m_sceneParticleEffects.size(); i++)
	{
		delete m_sceneParticleEffects[i];
		m_sceneParticleEffects[i] = nullptr;
	}
	m_sceneParticleEffects.clear();
}

void Scene::Update(float deltaSeconds)
{
	for (int i = 0; i < (int)m_sceneEntities.size(); i++)
	{
		if (m_sceneEntities[i] != nullptr && !m_sceneEntities[i]->m_isAlive)
		{
			delete m_sceneEntities[i];
			m_sceneEntities[i] = nullptr;
		}
	}


	for (int i = 0; i < (int)m_sceneEntities.size(); i++)
	{
		if (m_sceneEntities[i] != nullptr && m_sceneEntities[i]->m_isAlive)
		{
			m_sceneEntities[i]->Update(deltaSeconds);
		}
	}
}

void Scene::Render() const
{
	for (int i = 0; i < m_sceneEntities.size(); i++)
	{
		if (m_sceneEntities[i] != nullptr && m_sceneEntities[i]->m_isAlive)
		{
			m_sceneEntities[i]->Render();
		}
	}
}

Scene::~Scene()
{
	for (int i = 0; i < (int)m_sceneEntities.size(); i++)
	{
		if (m_sceneEntities[i] != nullptr)
		{
			delete m_sceneEntities[i];
			m_sceneEntities[i] = nullptr;
		}
	}
	m_sceneEntities.clear();

	for (int i = 0; i < (int)m_sceneParticleEffects.size(); i++)
	{
		if (m_sceneParticleEffects[i] != nullptr)
		{
			delete m_sceneParticleEffects[i];
			m_sceneParticleEffects[i] = nullptr;
		}
	}
	m_sceneParticleEffects.clear();
}
