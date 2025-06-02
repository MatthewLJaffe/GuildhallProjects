#include "Game/AIController.hpp"
#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/ActorDefinition.hpp"

AIController::AIController(Map* currentMap)
{
	m_map = currentMap;
}

void AIController::Update(float deltaSeconds)
{
	//return;
	UNUSED(deltaSeconds);
	Actor* targetPtr = g_theGame->m_currentMap->GetActorByUID(m_targetUID);
	Actor* thisActorPtr = GetActor();
	if (targetPtr == nullptr)
	{
		FindNewTarget();
		targetPtr = g_theGame->m_currentMap->GetActorByUID(m_targetUID);
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

	if (distanceFromTarget < 1.f) 
	{
		thisActorPtr->Attack();
	}

	thisActorPtr->MoveInDirection(thisActorPtr->GetForwardZUp(), moveSpeed);
}

void AIController::OnDamagedBy(ActorUID damageDealer)
{
	Actor* damageDealerPtr = g_theGame->m_currentMap->GetActorByUID(damageDealer);
	Actor* thisActorPtr = GetActor();
	if (damageDealerPtr == nullptr || thisActorPtr == nullptr)
	{
		return;
	}
	if (thisActorPtr->IsOpposingFaction(damageDealerPtr))
	{
		if (m_map->GetActorByUID(m_targetUID) != nullptr && m_map->GetActorByUID(m_targetUID)->m_actorDefinition->m_binkeyAI)
		{
			return;
		}

		m_targetUID = damageDealer;
	}
}

bool AIController::FindNewTarget()
{
	Actor* thisActorPtr = GetActor();
	ActorUID newTarget = thisActorPtr->m_map->FindTargetForActor(thisActorPtr);
	if (newTarget != ActorUID::INVALID)
	{
		m_targetUID = newTarget;
		return true;
	}
	return false;
}
