#pragma once
#include "Engine/Core/EngineCommon.hpp"

class Entity;
class Player;
class Boss1;
class Boss2;
class Boss3;

enum class GameStateType
{
	MAIN_MENU = 0,
	SETTINGS,
	HOW_TO_PLAY,
	LEVEL_SELECT,
	LEVEL_1,
	LEVEL_2,
	LEVEL_3,
	COUNT
};

class GameState
{
public:
	GameState(GameStateType gameStateType);
	virtual ~GameState();
	virtual void StartUp() = 0;
	virtual void OnEnable();
	virtual void OnDisable();
	virtual void Update(float deltaSeconds);
	void DeleteGarbageEntities();
	virtual void Render();
	void AddEntity(Entity* entityToAdd);
	void RemoveEntity(Entity* entityToRemove);
	void RecursivelyDeleteEntityAndChildren(Entity* entity);
	Player* GetPlayer();
	Boss2* GetBoss2();
	Boss3* GetBoss3();
	GameStateType m_gameStateType;
	bool m_needsReset = false;
	virtual void SetPause(bool isPaused);
protected:
	void AddEntityToList(Entity* entityToAdd, std::vector<Entity*>& entityList);
	void RemoveEntityFromList(Entity* entityToRemove, std::vector<Entity*>& entityList);
	std::vector<Entity*> m_allEntities;
	std::vector<Entity*> m_worldEntities;
	std::vector<Entity*> m_screenEntities;
	std::vector<Entity*> m_hazardEntities;
	std::vector<std::vector<Entity*>> m_allEntitiesByType;
};