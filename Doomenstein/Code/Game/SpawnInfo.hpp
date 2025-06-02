#pragma once
#include "Game/GameCommon.hpp"

class ActorDefinition;

struct SpawnInfo
{
	//name of actor definition to spawn
	ActorDefinition const* m_actorDefinition;
	Vec3 m_position;
	EulerAngles m_orientation;
	Vec3 m_velocity;
};