#pragma once
#include "Game/GameCommon.hpp"

class Map;
class Actor;

class Controller
{
public:
	Controller();
	virtual ~Controller();
	ActorUID m_currentlyPossesedActor = ActorUID::INVALID;
public:
	virtual void Update(float deltaSeconds) = 0;
	virtual void BeginFrame();
	void Possess(ActorUID actorToPosses);
	Actor* GetActor();
	Map* m_map;
	virtual void OnDamagedBy(ActorUID damageDealer);
	virtual void OnKilledBy(ActorUID damageDealer);
	bool IsPlayer() const;
protected:
	bool m_isPlayer = false;
};