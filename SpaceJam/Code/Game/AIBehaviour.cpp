#include "Game/AIBehaviour.hpp"
#include "Game/Actor.hpp"
#include "Game/GameScene.hpp"
#include "Game/PlayerShip.hpp"	
#include "Engine/Renderer/DebugRenderSystem.hpp"

//AIBehaviour-------------------------------------------------------------------------------------------------------------------------------------------
std::vector<AIBehaviourDefinition> AIBehaviourDefinition::s_aiBehaviourDefs;

AIBehaviour::AIBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor)
	: m_definition(aiBehaviourDef)
	, m_actor(actor)
{
	for (int i = 0; i < (int)m_definition.m_steeringBehaviourInits.size(); i++)
	{
		m_steeringBehaviours.push_back(SteeringBehaviour::ConstructFromInitializer(m_definition.m_steeringBehaviourInits[i], m_actor));
	}
}

AIBehaviour* AIBehaviour::GetNextBehaviour()
{
	for (int i = 0; i < (int)m_definition.m_transitions.size(); i++)
	{
		AIBehaviourTransition const& currTransition = m_definition.m_transitions[i];
		if (IsTransitionValid(currTransition))
		{
			for (int b = 0; b < m_actor.m_aiBehaviours.size(); b++)
			{
				if (m_actor.m_aiBehaviours[b]->m_definition.m_name == currTransition.m_nextBehaviour)
				{
					return m_actor.m_aiBehaviours[b];
				}
			}
		}
	}
	return this;
}

bool AIBehaviour::IsTransitionValid(AIBehaviourTransition const& transition) const
{
	float distanceToPlayer = GetDistance3D(g_theGame->m_playerShip->GetWorldPosition(), m_actor.GetWorldPosition());
	if (distanceToPlayer > transition.m_maxDistanceToPlayer)
	{
		return true;
	}
	if (distanceToPlayer < transition.m_minDistanceToPlayer)
	{
		return true;
	}
	if (m_activeDuration > transition.m_duration)
	{
		return true;
	}
	return false;
}

void AIBehaviour::OnBegin()
{
	m_activeDuration = 0.f;
}

AIBehaviour* AIBehaviour::Tick(float deltaSeconds)
{
	m_activeDuration += deltaSeconds;
	Vec3 strafeAcceleration = Vec3::ZERO;
	Vec3 thrustAcceleration = Vec3::ZERO;
	float strafeWeight = 0.f;
	float thrustWeight = 0.f;

	for (int i = 0; i < m_steeringBehaviours.size(); i++)
	{
		if (m_steeringBehaviours[i] != nullptr)
		{
			Vec3 weightedSteerDir = m_steeringBehaviours[i]->GetWeightedSteerDireciton();
			//acceleration += weightedSteerDir;
			
			//steer direction should contribute to which way the actor is facing
			if (m_steeringBehaviours[i]->m_strafe)
			{
				strafeAcceleration += weightedSteerDir;
				strafeWeight += fabsf(m_steeringBehaviours[i]->m_weight);
			}
			else
			{
				thrustAcceleration += weightedSteerDir;
				thrustWeight += fabsf(m_steeringBehaviours[i]->m_weight);
			}
			
		}
	}
	//acceleration.SetLength(m_actor.m_definition->m_acceleration);
	//m_actor.m_velocity += acceleration * deltaSeconds;
	float acceleration = m_definition.m_acceleration;
	if (acceleration < 0.f)
	{
		acceleration = m_actor.m_definition->m_acceleration;
	}

	float newStrafeLength = acceleration * strafeWeight;
	float newThrustLength = acceleration * thrustWeight;

	strafeAcceleration.SetLength(newStrafeLength);
	thrustAcceleration.SetLength(newThrustLength);

	m_actor.m_strafeVelocity += strafeAcceleration * deltaSeconds;
	float maxSpeed = m_definition.m_maxSpeed;
	if (maxSpeed < 0.f)
	{
		maxSpeed = m_actor.m_definition->m_maxSpeed;
	}
	m_actor.m_strafeVelocity = m_actor.m_strafeVelocity.Clamp(maxSpeed * strafeWeight);

	m_actor.m_thrustVelocity += thrustAcceleration * deltaSeconds;
	m_actor.m_thrustVelocity = m_actor.m_thrustVelocity.Clamp(maxSpeed * thrustWeight);

	m_actor.m_velocity = (m_actor.m_strafeVelocity + m_actor.m_thrustVelocity);
	

	return GetNextBehaviour();
}

void AIBehaviour::OnEnd()
{

}

AIBehaviour::~AIBehaviour()
{
    for (int i = 0; i < (int)m_steeringBehaviours.size(); i++)
    {
        delete m_steeringBehaviours[i];
        m_steeringBehaviours[i] = nullptr;
    }
}

AIBehaviour* AIBehaviour::ConstructAIBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor)
{
	if (aiBehaviourDef.m_type == "ExplodeBehaviour")
	{
		return new ExplodeBehaviour(aiBehaviourDef, actor);
	}
	if (aiBehaviourDef.m_type == "GunnerAttackBehaviour")
	{
		return new GunnerAttackBehaviour(aiBehaviourDef, actor);
	}
	if (aiBehaviourDef.m_type == "TurnAroundBehaviour")
	{
		return new TurnAroundBehaviour(aiBehaviourDef, actor);
	}
	if (aiBehaviourDef.m_type == "ArtilleryAttackBehaviour")
	{
		return new ArtilleryAttackBehaviour(aiBehaviourDef, actor);
	}
	if (aiBehaviourDef.m_type == "ImperialDefaultBehaviour")
	{
		return new ImperialDefaultBehaviour(aiBehaviourDef, actor);

	}

	return new AIBehaviour(aiBehaviourDef, actor);
}

AIBehaviourDefinition const& AIBehaviourDefinition::LoadFromXMLElement(XmlElement const& element)
{
	std::string behaviourName = ParseXmlAttribute(element, "name", "");
	for (int i = 0; i < (int)s_aiBehaviourDefs.size(); i++)
	{
		if (s_aiBehaviourDefs[i].m_name == behaviourName)
		{
			return s_aiBehaviourDefs[i];
		}
	}
	s_aiBehaviourDefs.emplace_back();
	AIBehaviourDefinition& currentDef = s_aiBehaviourDefs[(int)s_aiBehaviourDefs.size() - 1];
	currentDef.m_name = behaviourName;
	currentDef.m_type = ParseXmlAttribute(element, "type", "AIBehaviour");
	currentDef.m_default = ParseXmlAttribute(element, "default", false);
	currentDef.m_maxSpeed = ParseXmlAttribute(element, "maxSpeed", -1.f);
	currentDef.m_acceleration = ParseXmlAttribute(element, "acceleration", -1.f);

	//parse steering behaviors
	XmlElement const* steeringBehaviours = element.FirstChildElement("SteeringBehaviours");
	if (steeringBehaviours != nullptr)
	{
		for (XmlElement const* currElement = steeringBehaviours->FirstChildElement("SteeringBehaviour");
			currElement != nullptr; currElement = currElement->NextSiblingElement("SteeringBehaviour"))
		{
			currentDef.m_steeringBehaviourInits.emplace_back();
			SteeringBehaviourInitializer& currentSBInit = currentDef.m_steeringBehaviourInits[(int)currentDef.m_steeringBehaviourInits.size() - 1];
			currentSBInit.m_type = ParseXmlAttribute(*currElement, "type", "");
			currentSBInit.m_strafe = ParseXmlAttribute(*currElement, "strafe", false);
			currentSBInit.m_weight = ParseXmlAttribute(*currElement, "weight", 0.f);
		}
	}

	//parse transitions
	XmlElement const* transitions = element.FirstChildElement("Transitions");
	if (transitions != nullptr)
	{
		for (XmlElement const* currElement = transitions->FirstChildElement("Transition");
			currElement != nullptr; currElement = currElement->NextSiblingElement("Transition"))
		{
			currentDef.m_transitions.emplace_back();
			AIBehaviourTransition& currentTransition = currentDef.m_transitions[(int)currentDef.m_transitions.size() - 1];
			currentTransition.m_maxDistanceToPlayer = ParseXmlAttribute(*currElement, "maxDistance", FLT_MAX);
			currentTransition.m_minDistanceToPlayer = ParseXmlAttribute(*currElement, "minDistance", FLT_MIN);
			currentTransition.m_duration = ParseXmlAttribute(*currElement, "duration", 9999999999.f);
			currentTransition.m_nextBehaviour = ParseXmlAttribute(*currElement, "nextBehaviour", "");
		}
	}

	return currentDef;
}

//ExplodeBehaviour-----------------------------------------------------------------------------------------------------------------------------------
ExplodeBehaviour::ExplodeBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor)
	: AIBehaviour(aiBehaviourDef, actor)
{
}

void ExplodeBehaviour::OnBegin()
{
	AIBehaviour::OnBegin();
	m_explodeTimer = Timer(BEAT_TIME, g_theApp->m_clock);
	float timeToNextBeat = BEAT_TIME - g_theGame->m_gameScene->GetCurrentTimeFromLastBeat();
	m_explodeTimer.m_period = timeToNextBeat + BEAT_TIME;
	m_explodeMidwayPoint = timeToNextBeat / m_explodeTimer.m_period;
	m_explodeTimer.Start();
}

AIBehaviour* ExplodeBehaviour::Tick(float deltaSeconds)
{
	//slow down
    if (m_actor.m_velocity.GetLengthSquared() > .01f)
    {
		Vec3 prevVel = m_actor.m_velocity;
		m_actor.m_velocity -= m_actor.m_velocity.GetNormalized() * m_actor.m_definition->m_acceleration * deltaSeconds;
		if (DotProduct3D(prevVel, m_actor.m_velocity) <= 0.f)
		{
			m_actor.m_velocity = Vec3::ZERO;
		}
    }

	float m_maxScale = 1.75f;
	//#ToDo explode animation
	if (m_explodeTimer.GetElapsedFraction() < m_explodeMidwayPoint)
	{
		float t = m_explodeTimer.GetElapsedFraction() / m_explodeMidwayPoint;
		t = SmoothStart5(t);
		m_actor.SetLocalScale(Vec3::Lerp(Vec3(1.f, 1.f, 1.f), Vec3(m_maxScale, m_maxScale, m_maxScale), t));
		//m_color = LerpColor(Rgba8::WHITE, Rgba8::YELLOW, t);
	}
	else
	{
		float t = GetFractionWithinRange(m_explodeTimer.GetElapsedFraction(), m_explodeMidwayPoint, 1.f);
		m_actor.SetLocalScale(Vec3::Lerp(Vec3(m_maxScale, m_maxScale, m_maxScale), Vec3(1.f, 1.f, 1.f), t));
		t = SmoothStop3(t);
		m_actor.m_color = LerpColor(Rgba8::WHITE, Rgba8::RED, t);
	}

	if (m_explodeTimer.HasPeriodElapsed() || m_explodeTimer.GetElapsedFraction() > .9f && g_theGame->m_gameScene->m_isFrameBeat)
	{
		m_actor.Die();
	}
    return this;
}

//GunnerAttackBehaviour-------------------------------------------------------------------------------------------------------------------------------
GunnerAttackBehaviour::GunnerAttackBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor)
: AIBehaviour(aiBehaviourDef, actor)
{
	m_lockOnTimer = Timer(BEAT_TIME * 4.f, g_theApp->m_clock);
}

AIBehaviour* GunnerAttackBehaviour::Tick(float deltaSeconds)
{
	/*
	Vec3 dirToPlayer = (g_theGame->m_playerShip->GetWorldPosition() - m_actor.GetWorldPosition()).GetNormalized();
	Mat44 rotation;
	Vec3 iBasis = -dirToPlayer;
	Vec3 kBasis = g_theGame->m_playerShip->GetWorldOrientaiton().GetKBasis3D().GetNormalized();
	Vec3 jBasis = CrossProduct3D(kBasis, iBasis).GetNormalized();
	rotation.SetIJK3D(iBasis, jBasis, kBasis);
	m_actor.SetLocalOrientation(rotation);
	*/
	if (g_theGame->m_playerShip->m_isDashing && g_theGame->m_playerShip->m_isDashGood)
	{
		m_lockOnTimer.Start();
		m_lockOnTimer.m_period = (float)g_randGen->RollRandomIntInRange(1, 5) * BEAT_TIME;
	}
	if (g_theGame->m_gameScene->m_isFrameBeat && (m_lockOnTimer.HasPeriodElapsed() || m_lockOnTimer.IsStopped()))
	{
		FireProjectileAtPlayer();
	}

	return AIBehaviour::Tick(deltaSeconds);
}

void GunnerAttackBehaviour::FireProjectileAtPlayer()
{
	Vec3 spawnPos = m_actor.GetWorldPosition() + m_actor.GetWorldOrientaiton().GetIBasis3D() * 2.f;
	SpawnInfo laserSpawnInfo(spawnPos, m_actor.GetWorldOrientaiton(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName("LaserEnemy"));

	Vec3 playerPos = g_theGame->m_playerShip->GetWorldPosition();

	float desiredVelocity = 50.f;
	float distanceBetweenEntitiesNow = GetDistance3D(spawnPos, playerPos);
	float desiredTravelTime = distanceBetweenEntitiesNow / desiredVelocity;
	int halfTravelBeatsRoundedDown = RoundDownToInt(desiredTravelTime / (BEAT_TIME * .5f));

	//in the future we can just treat this as a hitscan and spawn the particle at the enemy ship
	if (halfTravelBeatsRoundedDown % 2 == 0)
	{
		halfTravelBeatsRoundedDown++;
	}
	float travelTime = (float)halfTravelBeatsRoundedDown * (BEAT_TIME * .5f);
	Vec3 actorFuturePos = playerPos + travelTime * g_theGame->m_playerShip->GetWorldSpaceVelocity();
	Vec3 projectileVelocityDirection = actorFuturePos - spawnPos;
	projectileVelocityDirection.SetLength(projectileVelocityDirection.GetLength() / travelTime);
	laserSpawnInfo.m_velocity = projectileVelocityDirection;
	laserSpawnInfo.m_orientation = GetRotationMatrixFromIBasisKUp(laserSpawnInfo.m_velocity.GetNormalized());

	//g_theAudio->StartSound(m_laserSound, false, .125f);

	
	BeatSound beatSound;
	beatSound.m_beatTimeInSeconds = BEAT_TIME * .5f;
	beatSound.m_volume = .125f;
	beatSound.m_sound = g_theAudio->CreateOrGetSound("Data/Audio/Enemy_Shoot.mp3");
	g_theGame->m_gameScene->QueueBeatSound(beatSound);
	

	g_theGame->m_gameScene->SpawnActor(laserSpawnInfo);
}

void GunnerAttackBehaviour::OnBegin()
{
	AIBehaviour::OnBegin();
	//m_actor.m_overrideOrientation = true;
}

void GunnerAttackBehaviour::OnEnd()
{
	//m_actor.m_overrideOrientation = false;
}

//Turn Around Behaviour ------------------------------------------------------------------------------------------------------------------------------------------------
TurnAroundBehaviour::TurnAroundBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor)
	: AIBehaviour(aiBehaviourDef, actor)
{
	m_turnTimer = Timer(1.f, g_theApp->m_clock);
}

void TurnAroundBehaviour::OnBegin()
{
	AIBehaviour::OnBegin();
	m_turnTimer.Start();
	m_originalFwd = m_actor.GetWorldOrientaiton().GetIBasis3D();
}

AIBehaviour* TurnAroundBehaviour::Tick(float deltaSeconds)
{
	AIBehaviour::Tick(deltaSeconds);
	Mat44 currentFrameRotation = Mat44::CreateZRotationDegrees(m_turnRate * deltaSeconds);
	Mat44 currentActorOrientation = m_actor.GetWorldOrientaiton();

	currentActorOrientation.Append(currentFrameRotation);
	m_actor.SetLocalOrientation(currentActorOrientation);

	/*
	Vec3 dirToPlayer = g_theGame->m_playerShip->GetWorldPosition() - m_actor.GetWorldPosition();
	dirToPlayer = dirToPlayer.GetNormalized();
	Vec3 up = g_theGame->m_playerShip->GetWorldOrientaiton().GetKBasis3D();
	Mat44 midpointRotation;
	Mat44 endRotation;
	float rotationTheta = 180.f * m_turnTimer.GetElapsedFraction();

	Vec3 newFwd = Mat44::CreateAxisAngleRotation(up, rotationTheta).TransformVectorQuantity3D(dirToPlayer);
	

	newFwd = newFwd.GetNormalized();
	m_actor.SetRotationFromFwdPlayerUp(newFwd);
	*/
	return GetNextBehaviour();
}

//Artillery Attack Behaviour ------------------------------------------------------------------------------------------------------------------------------------------------
ArtilleryAttackBehaviour::ArtilleryAttackBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor)
	: AIBehaviour(aiBehaviourDef, actor)
{
	m_lockOnTimer = Timer((float)g_randGen->RollRandomIntInRange(1, 3) * BEAT_TIME, g_theApp->m_clock);
}

void ArtilleryAttackBehaviour::OnBegin()
{
	AIBehaviour::OnBegin();
	m_beatOffset = g_randGen->RollRandomIntInRange(0, 7);
}

void ArtilleryAttackBehaviour::OnEnd()
{
	AIBehaviour::OnEnd();
}

AIBehaviour* ArtilleryAttackBehaviour::Tick(float deltaSeconds)
{
	if (g_theGame->m_playerShip->m_isDashing && g_theGame->m_playerShip->m_isDashGood)
	{
		m_lockOnTimer.m_period = (float)g_randGen->RollRandomIntInRange(1, 4) * BEAT_TIME;
		m_lockOnTimer.Start();
	}
	if (m_lockOnTimer.HasPeriodElapsed() || m_lockOnTimer.IsStopped())
	{
		if (g_theGame->m_gameScene->m_isFrameBeat && g_theGame->m_gameScene->m_currentBeats % 8 == m_beatOffset)
		{
			ArtilleryStrikeAtPlayer();
		}
	}
	return AIBehaviour::Tick(deltaSeconds);
}

void ArtilleryAttackBehaviour::ArtilleryStrikeAtPlayer()
{
	Vec3 shootPoint = m_actor.GetWorldOrientaiton().GetIBasis3D() * 2.f + m_actor.GetWorldPosition();
	Vec3 playerPos = g_theGame->m_playerShip->GetWorldPosition();

	float desiredVelocity = 30.f;
	float distanceBetweenEntitiesNow = GetDistance3D(shootPoint, playerPos);
	float desiredTravelTime = distanceBetweenEntitiesNow / desiredVelocity;
	int halfTravelBeatsRoundedDown = RoundDownToInt(desiredTravelTime / (BEAT_TIME * .5f));

	//in the future we can just treat this as a hitscan and spawn the particle at the enemy ship
	if (halfTravelBeatsRoundedDown % 2 == 0)
	{
		halfTravelBeatsRoundedDown++;
	}
	float travelTime = (float)halfTravelBeatsRoundedDown * (BEAT_TIME * .5f);
	Vec3 actorFuturePos = playerPos + travelTime * g_theGame->m_playerShip->GetWorldSpaceVelocity();
	Vec3 projectileVelocityDirection = actorFuturePos - shootPoint;
	projectileVelocityDirection.SetLength(projectileVelocityDirection.GetLength() / travelTime);

	SpawnInfo warnZoneSpawn(actorFuturePos, Mat44(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName("ArtilleryWarnZone"));
	Actor& warnZone = g_theGame->m_gameScene->SpawnActor(warnZoneSpawn);
	warnZone.m_lifetimeOverride = travelTime;

	Mat44 shellRotation;
	Vec3 iBasis = projectileVelocityDirection.GetNormalized();
	Vec3 kBasis = g_theGame->m_playerShip->GetWorldOrientaiton().GetKBasis3D().GetNormalized();
	Vec3 jBasis = CrossProduct3D(kBasis, iBasis).GetNormalized();
	shellRotation.SetIJK3D(iBasis, jBasis, kBasis);
	SpawnInfo artilleryShellSpawn(shootPoint, shellRotation, projectileVelocityDirection, ActorDefinition::GetActorDefinitionFromName("ArtilleryShell"));
	Actor& artilleryShell = g_theGame->m_gameScene->SpawnActor(artilleryShellSpawn);
	artilleryShell.m_lifetimeOverride = travelTime;
	
}

ImperialDefaultBehaviour::ImperialDefaultBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor)
	: AIBehaviour(aiBehaviourDef, actor)
{
	m_unitSpawnProbs.emplace_back("Thrall", .7f);
	m_unitSpawnProbs.emplace_back("Gunner", .3f);
}

void ImperialDefaultBehaviour::OnBegin()
{
	AIBehaviour::OnBegin();
	m_attackPlayerTimer = Timer(BEAT_TIME * (float)g_randGen->RollRandomIntInRange(m_minAttackCooldown, m_minAttackCooldown) ,g_theApp->m_clock);
	m_spawnTimer = Timer(BEAT_TIME * (float)g_randGen->RollRandomIntInRange(m_minSpawnCooldown, m_maxSpawnCooldown), g_theApp->m_clock);

	m_attackPlayerTimer.Start();
	m_spawnTimer.Start();

}

void ImperialDefaultBehaviour::OnEnd()
{
	AIBehaviour::OnEnd();
}

AIBehaviour* ImperialDefaultBehaviour::Tick(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	Vec3 playerPos = g_theGame->m_playerShip->GetWorldPosition();
	float attackRange = 60.f;

	if (GetDistanceSquared3D(playerPos, m_actor.GetWorldPosition()) < attackRange * attackRange)
	{
		if (m_attackPlayerTimer.HasPeriodElapsed())
		{
			m_attackPlayerTimer.m_period = BEAT_TIME * (float)g_randGen->RollRandomIntInRange(m_minAttackCooldown, m_minAttackCooldown);
			m_attackPlayerTimer.Start();
		}
	}

	if (m_spawnTimer.HasPeriodElapsed())
	{
		SpawnUnit();
		m_spawnTimer.Start();
	}

	return this;
}

void ImperialDefaultBehaviour::SpawnUnit()
{
	float random = g_randGen->RollRandomFloatZeroToOne();
	for (int i = 0; i < (int)m_unitSpawnProbs.size(); i++)
	{
		if (m_unitSpawnProbs[i].m_probability > random)
		{
			Vec3 spawnPos = m_actor.GetWorldPosition() - m_actor.GetWorldOrientaiton().GetKBasis3D() * 5.f;
			Mat44 spawnOrientation = m_actor.GetWorldOrientaiton();
			SpawnInfo spawnInfo(spawnPos, spawnOrientation, Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName( m_unitSpawnProbs[i].m_unitName));
			g_theGame->m_gameScene->SpawnActor(spawnInfo);
			break;
		}
		random -= m_unitSpawnProbs[i].m_probability;
	}
}
