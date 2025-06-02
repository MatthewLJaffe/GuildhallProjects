#include "BinkeyController.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Player.hpp"


BinkeyController::BinkeyController(Map* currentMap)
{
	m_map = currentMap;
}

void BinkeyController::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (m_currentState == BinkeyState::BINKEY_STATE_IDLE)
	{
		return;
	}

	else if (m_currentState == BinkeyState::BINKEY_STATE_ATTACK)
	{
		UpdateAttackState();
	}

	else if (m_currentState == BinkeyState::BINKEY_STATE_FOLLOW_PLAYER)
	{
		UpdateFollowState();
	}
}

void BinkeyController::UpdateAttackState()
{
	Actor* targetPtr = g_theGame->m_currentMap->GetActorByUID(m_enemyTarget);
	Actor* thisActorPtr = GetActor();
	if (targetPtr == nullptr)
	{
		FindNewTarget();
		targetPtr = g_theGame->m_currentMap->GetActorByUID(m_enemyTarget);
		if (targetPtr == nullptr)
		{
			return;
		}
	}

	Vec3 dispToTarget = targetPtr->m_position - thisActorPtr->m_position;
	thisActorPtr->TurnTorwardsDirection(dispToTarget.GetXY());

	float distanceFromTarget = dispToTarget.GetLength();
	float closeDistance = 2.f;
	float moveSpeed = thisActorPtr->m_actorDefinition->m_runSpeed;
	if (distanceFromTarget <= closeDistance)
	{
		moveSpeed = thisActorPtr->m_actorDefinition->m_walkSpeed;
	}

	if (distanceFromTarget < targetPtr->m_physicsCylinderRadius + thisActorPtr->m_physicsCylinderRadius + .1f)
	{
		moveSpeed = 0.f;
	}

	if (distanceFromTarget < .75f)
	{
		thisActorPtr->Attack();
	}

	thisActorPtr->MoveInDirection(thisActorPtr->GetForwardZUp(), moveSpeed);
}

void BinkeyController::UpdateFollowState()
{
	Actor* targetPtr = g_theGame->m_players[0]->GetActor();
	Actor* thisActorPtr = GetActor();

	Vec3 dispToTarget = targetPtr->m_position - thisActorPtr->m_position;
	thisActorPtr->TurnTorwardsDirection(dispToTarget.GetXY());

	float distanceFromTarget = dispToTarget.GetLength();
	float walkDistance = 5.f;
	float closeDistance = 3.f;
	float moveSpeed = thisActorPtr->m_actorDefinition->m_runSpeed;
	if (distanceFromTarget < walkDistance)
	{
		moveSpeed = thisActorPtr->m_actorDefinition->m_walkSpeed;
	}
	if (distanceFromTarget < closeDistance)
	{
		moveSpeed = 0.f;
	}
	thisActorPtr->MoveInDirection(thisActorPtr->GetForwardZUp(), moveSpeed);
}

void BinkeyController::OnDamagedBy(ActorUID damageDealer)
{
	UNUSED(damageDealer);
	Task5_CheckForBinkeyAlerted* alertTask = dynamic_cast<Task5_CheckForBinkeyAlerted*>( g_theGame->GetTaskByName("Task5_CheckForBinkeyAlerted") );
	if (alertTask != nullptr)
	{
		alertTask->m_binkeyHurt = true;
	}
	if (damageDealer.GetActor() == g_theGame->GetFirsPlayerActor())
	{
		Task9_BinkeyRap* rapTask = dynamic_cast<Task9_BinkeyRap*>(g_theGame->GetTaskByName("Task9_BinkeyRap"));
		if (rapTask)
		{
			rapTask->m_endRap = true;
		}
	}
}

bool BinkeyController::FindNewTarget()
{
	Actor* thisActorPtr = GetActor();
	ActorUID newTarget = thisActorPtr->m_map->FindTargetForActor(thisActorPtr);
	if (newTarget != ActorUID::INVALID)
	{
		m_enemyTarget = newTarget;
		return true;
	}
	return false;
}
