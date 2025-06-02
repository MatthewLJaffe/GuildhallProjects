#pragma once
#include "Game/Controller.hpp"

enum class BinkeyState
{
	BINKEY_STATE_IDLE,
	BINKEY_STATE_ATTACK,
	BINKEY_STATE_FOLLOW_PLAYER,
};

class BinkeyController : public Controller
{
public:
	BinkeyController(Map* currentMap);
	virtual void Update(float deltaSeconds) override;
	void UpdateAttackState();
	void UpdateFollowState();
	void OnDamagedBy(ActorUID damageDealer) override;
	bool FindNewTarget();
	ActorUID m_enemyTarget = ActorUID::INVALID;
	ActorUID m_playerTarget = ActorUID::INVALID;
	BinkeyState m_currentState = BinkeyState::BINKEY_STATE_IDLE;
};
