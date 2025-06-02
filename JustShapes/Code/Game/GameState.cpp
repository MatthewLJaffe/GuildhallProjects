#include "Game/GameState.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Button.hpp"
#include "Game/Player.hpp"
#include "Game/Boss1.hpp"
#include "Game/Boss2.hpp"
#include "Game/Boss3.hpp"

GameState::GameState(GameStateType gameStateType)
	: m_gameStateType(gameStateType)
{
	m_allEntitiesByType.resize((int)EntityType::COUNT);
}

GameState::~GameState()
{
	for (int i = 0; i < (int)m_allEntities.size(); i++)
	{
		RemoveEntity(m_allEntities[i]);
	}
}

void GameState::OnEnable()
{
}

void GameState::OnDisable()
{
	g_theGame->ResetCurrentState();
}

void GameState::Update(float deltaSeconds)
{
	for (int i = 0; i < (int)m_allEntities.size(); i++)
	{
		if (m_allEntities[i] != nullptr && !m_allEntities[i]->m_isGarbage)
		{
			m_allEntities[i]->Update(deltaSeconds);
		}
	}
	DeleteGarbageEntities();
}

void GameState::DeleteGarbageEntities()
{
	for (int i = 0; i < (int)m_allEntities.size(); i++)
	{
		if (m_allEntities[i] != nullptr && m_allEntities[i]->m_isGarbage)
		{
			RemoveEntity(m_allEntities[i]);
		}
	}
}

void GameState::Render()
{
	g_theRenderer->BeginCamera(g_theGame->m_worldCamera);
	for (int i = 0; i < (int)m_allEntitiesByType.size(); i++)
	{
		std::vector<Entity*>& entityList = m_allEntitiesByType[i];
		for (int j = 0; j < entityList.size(); j++)
		{
			if (entityList[j] == nullptr)
			{
				continue;
			}
			if (entityList[j]->IsScreenSpace())
			{
				break;
			}
			if (!entityList[j]->m_isGarbage)
			{
				entityList[j]->Render();
			}
		}

	}
	g_theRenderer->EndCamera(g_theGame->m_worldCamera);

	g_theRenderer->BeginCamera(g_theGame->m_screenCamera);
	for (int i = 0; i < (int)m_allEntitiesByType.size(); i++)
	{
		std::vector<Entity*>& entityList = m_allEntitiesByType[i];
		for (int j = 0; j < entityList.size(); j++)
		{
			if (entityList[j] == nullptr)
			{
				continue;
			}
			if (!entityList[j]->IsScreenSpace())
			{
				break;
			}
			if (!entityList[j]->m_isGarbage)
			{
				entityList[j]->Render();
			}
		}

	}
	g_theRenderer->EndCamera(g_theGame->m_screenCamera);
}

void GameState::AddEntity(Entity* entityToAdd)
{
	AddEntityToList(entityToAdd, m_allEntities);
	if (entityToAdd->IsScreenSpace())
	{
		AddEntityToList(entityToAdd, m_screenEntities);
	}
	else
	{
		AddEntityToList(entityToAdd, m_worldEntities);
	}

	if (entityToAdd->m_isHazard || entityToAdd->m_entityType == EntityType::ENEMY_PROJECTILE || entityToAdd->m_entityType == EntityType::ARMS)
	{
		AddEntityToList(entityToAdd, m_hazardEntities);
	}
	AddEntityToList(entityToAdd, m_allEntitiesByType[(int)entityToAdd->m_entityType]);
}

void GameState::RemoveEntity(Entity* entityToRemove)
{
	if (entityToRemove == nullptr)
	{
		return;
	}
	RemoveEntityFromList(entityToRemove, m_allEntities);
	if (entityToRemove->IsScreenSpace())
	{
		RemoveEntityFromList(entityToRemove, m_screenEntities);
	}
	else
	{
		RemoveEntityFromList(entityToRemove, m_worldEntities);
	}
	if (entityToRemove->m_isHazard || entityToRemove->m_entityType == EntityType::ENEMY_PROJECTILE)
	{
		RemoveEntityFromList(entityToRemove, m_hazardEntities);
	}
	RemoveEntityFromList(entityToRemove, m_allEntitiesByType[(int)entityToRemove->m_entityType]);
	RecursivelyDeleteEntityAndChildren(entityToRemove);
}

void GameState::RecursivelyDeleteEntityAndChildren(Entity* entity)
{
	if (entity->m_children.size() > 0)
	{
		for (int i = 0; i < (int)entity->m_children.size(); i++)
		{
			RecursivelyDeleteEntityAndChildren(entity->m_children[i]);
		}
	}
	delete entity;
	entity = nullptr;
}

Player* GameState::GetPlayer()
{
	if (m_allEntitiesByType[(int)EntityType::PLAYER].size() > 0)
	{
		Player* player = dynamic_cast<Player*>(m_allEntitiesByType[(int)EntityType::PLAYER][0]);
		return player;
	}
	return nullptr;
}

Boss2* GameState::GetBoss2()
{
	if (m_allEntitiesByType[(int)EntityType::BOSS_2].size() == 0)
	{
		return nullptr;
	}
	return dynamic_cast<Boss2*>(m_allEntitiesByType[(int)EntityType::BOSS_2][0]);
}

Boss3* GameState::GetBoss3()
{
	if (m_allEntitiesByType[(int)EntityType::BOSS_3].size() == 0)
	{
		return nullptr;
	}
	return dynamic_cast<Boss3*>(m_allEntitiesByType[(int)EntityType::BOSS_3][0]);
}


void GameState::SetPause(bool isPaused)
{
	UNUSED(isPaused);
}

void GameState::AddEntityToList(Entity* entityToAdd, std::vector<Entity*>& entityList)
{
	for (int i = 0; i < (int)entityList.size(); i++)
	{
		if (entityList[i] == nullptr)
		{
			entityList[i] = entityToAdd;
			return;
		}
	}

	entityList.push_back(entityToAdd);
}

void GameState::RemoveEntityFromList(Entity* entityToRemove, std::vector<Entity*>& entityList)
{
	for (int i = 0; i < (int)entityList.size(); i++)
	{
		if (entityList[i] == entityToRemove)
		{
			entityList[i] = nullptr;
			return;
		}
	}
}


