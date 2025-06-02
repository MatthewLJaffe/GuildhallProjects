#include "Game/Controller.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"




Controller::Controller()
{
}

Controller::~Controller()
{
}

void Controller::BeginFrame()
{
}

void Controller::Possess(ActorUID actorToPosses)
{
	Actor* actorToUnpossessRaw = GetActor();
	if (actorToUnpossessRaw != nullptr)
	{
		actorToUnpossessRaw->OnUnpossessed();
	}
	m_currentlyPossesedActor = actorToPosses;
	Actor* possesedActorRaw = GetActor();
	if (possesedActorRaw != nullptr)
	{
		possesedActorRaw->OnPossessed(this);
	}
}

Actor* Controller::GetActor()
{
	return m_map->GetActorByUID(m_currentlyPossesedActor);
}

void Controller::OnDamagedBy(ActorUID damageDealer)
{
	UNUSED(damageDealer);
}

void Controller::OnKilledBy(ActorUID damageDealer)
{
	UNUSED(damageDealer);
}

bool Controller::IsPlayer() const
{
	return m_isPlayer;
}
