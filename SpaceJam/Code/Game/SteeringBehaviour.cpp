#include "Game/SteeringBehaviour.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/GameScene.hpp"

SteeringBehaviour::SteeringBehaviour(SteeringBehaviourInitializer const& initializer, Actor const& actor)
	: m_actor(actor)
	, m_weight(initializer.m_weight)
	, m_strafe(initializer.m_strafe)
{
}

SteeringBehaviour* SteeringBehaviour::ConstructFromInitializer(SteeringBehaviourInitializer const& initializer, Actor const& owningActor)
{
	if (initializer.m_type == "ChasePlayerSB")
	{
		return new ChasePlayerSB(initializer, owningActor);
	}
	if (initializer.m_type == "AvoidObstaclesSB")
	{
		return new AvoidObstaclesSB(initializer, owningActor);
	}
	if (initializer.m_type == "CirclePlayerSB")
	{
		return new CirclePlayerSB(initializer, owningActor);
	}
	if (initializer.m_type == "LeftSB")
	{
		return new LeftSB(initializer, owningActor);
	}
	return nullptr;
}

//Chase Player-------------------------------------------------------------------------------------------------------------------------------------------------
ChasePlayerSB::ChasePlayerSB(SteeringBehaviourInitializer const& initializer, Actor const& actor)
	: SteeringBehaviour(initializer, actor)
{

}

Vec3 ChasePlayerSB::GetWeightedSteerDireciton() const
{
	Vec3 frontOfPlayer = g_theGame->m_playerShip->GetWorldPosition() + g_theGame->m_playerShip->GetForwardNormal();
	Vec3 steerDir = (frontOfPlayer - m_actor.GetWorldPosition()).GetNormalized() * m_weight;
	return steerDir;
}

//Avoid Obstacles-------------------------------------------------------------------------------------------------------------------------------------------------
AvoidObstaclesSB::AvoidObstaclesSB(SteeringBehaviourInitializer const& initializer, Actor const& actor)
	: SteeringBehaviour(initializer, actor)
{

}

Vec3 AvoidObstaclesSB::GetWeightedSteerDireciton() const
{
	Vec3 seperateDir = Vec3::ZERO;
	std::vector<Actor*> const& obstacles = g_theGame->m_gameScene->m_obstacles;
	float totalWeight = 0.f;
	for (int i = 0; i < (int)obstacles.size(); i++)
	{
		if (!IsValidActor(obstacles[i]) || obstacles[i] == &m_actor)
		{
			continue;
		}
		Vec3 otherThrallPos = obstacles[i]->GetWorldPosition();
		float maxAvoidDistance = 10.f;
		float minAvoidWeight = .0f;
		float maxAvoidWeight = 1.f;

		if (GetDistanceSquared3D(otherThrallPos, m_actor.GetWorldPosition()) > maxAvoidDistance * maxAvoidDistance)
		{
			continue;
		}
		float normalizedDistance = GetDistance3D(otherThrallPos, m_actor.GetWorldPosition()) / maxAvoidDistance;
		float distanceWeight = 1.f - normalizedDistance;

		//prioritize close
		distanceWeight = SmoothStart2(distanceWeight);
		distanceWeight = Lerp(minAvoidWeight, maxAvoidWeight, distanceWeight);

		if (distanceWeight < minAvoidWeight)
		{
			distanceWeight = minAvoidWeight;
		}
		seperateDir += distanceWeight * (m_actor.GetWorldPosition() - otherThrallPos).GetNormalized();
		totalWeight += distanceWeight;
	}
	if (totalWeight > 0.f)
	{
		return seperateDir * (m_weight/ (float)totalWeight);
	}
	return Vec3::ZERO;
}


//Circle Player----------------------------------------------------------------------------------------------------------------------------------------------------
CirclePlayerSB::CirclePlayerSB(SteeringBehaviourInitializer const& initializer, Actor const& actor)
	: SteeringBehaviour(initializer, actor)
{
}

Vec3 CirclePlayerSB::GetWeightedSteerDireciton() const
{
	Vec3 dirToPlayer = (g_theGame->m_playerShip->GetWorldPosition() - m_actor.GetWorldPosition()).GetNormalized();
	Vec3 iBasis = dirToPlayer;
	Vec3 kBasis = g_theGame->m_playerShip->GetWorldOrientaiton().GetKBasis3D().GetNormalized();
	Vec3 jBasis = CrossProduct3D(kBasis, iBasis).GetNormalized();
	return jBasis * m_weight;
}

LeftSB::LeftSB(SteeringBehaviourInitializer const& initializer, Actor const& actor)
	: SteeringBehaviour(initializer, actor)
{
}

Vec3 LeftSB::GetWeightedSteerDireciton() const
{
	return Vec3(0.f, 1.f, 0.f) * m_weight;
}

