#pragma once
#include "Game/Controller.hpp"

class AIController : public Controller
{
public:
	AIController(Map* currentMap);
	virtual void Update(float deltaSeconds) override;
	void OnDamagedBy(ActorUID damageDealer) override;
	bool FindNewTarget();
	ActorUID m_targetUID;
};