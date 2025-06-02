#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/HCIS.hpp"
#include "Game/SteeringBehaviour.hpp"

class Actor;

struct AIBehaviourTransition
{
	HCIS m_nextBehaviour = "";
	float m_minDistanceToPlayer = FLT_MIN;
	float m_maxDistanceToPlayer = FLT_MAX;
	float m_duration = 999999999.f;
};

struct AIBehaviourDefinition
{
	static AIBehaviourDefinition const& LoadFromXMLElement(XmlElement const& element);

	static std::vector<AIBehaviourDefinition> s_aiBehaviourDefs;
	HCIS m_name = "";
	HCIS m_type = "";
	std::vector<AIBehaviourTransition> m_transitions;
	std::vector<SteeringBehaviourInitializer> m_steeringBehaviourInits;
	bool m_default = false;
	float m_maxSpeed = -1.f;
	float m_acceleration = -1.f;
};

class AIBehaviour
{
public:
	static AIBehaviour* ConstructAIBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor);
	virtual AIBehaviour* Tick(float deltaSeconds);
	virtual void OnBegin();
	virtual void OnEnd();
	Actor& m_actor;
	AIBehaviourDefinition const& m_definition;
	std::vector<SteeringBehaviour*> m_steeringBehaviours;
	virtual ~AIBehaviour();
	virtual bool IsTransitionValid(AIBehaviourTransition const& transition) const;
	AIBehaviour* GetNextBehaviour();
	float m_activeDuration = 0.f;
protected:
	AIBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor);
};

class ExplodeBehaviour : public AIBehaviour
{
	friend class AIBehaviour;
	ExplodeBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor);
	virtual void OnBegin() override;
	virtual AIBehaviour* Tick(float deltaSeconds) override;
	Timer m_explodeTimer;
	float m_explodeMidwayPoint = 0.f;
};

class GunnerAttackBehaviour : public AIBehaviour
{
	friend class AIBehaviour;
	GunnerAttackBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor);
	virtual void OnBegin() override;
	virtual void OnEnd() override;
	virtual AIBehaviour* Tick(float deltaSeconds) override;
	void FireProjectileAtPlayer();
	Timer m_lockOnTimer;
};

class TurnAroundBehaviour : public AIBehaviour
{
	friend class AIBehaviour;
	TurnAroundBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor);
	virtual AIBehaviour* Tick(float deltaSeconds) override;
	virtual void OnBegin() override;
	Vec3 m_originalFwd;
	Timer m_turnTimer;
	float m_turnRate = 180.f;
};

class ArtilleryAttackBehaviour : public AIBehaviour
{
	friend class AIBehaviour;
	ArtilleryAttackBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor);
	virtual void OnBegin() override;
	virtual void OnEnd() override;
	virtual AIBehaviour* Tick(float deltaSeconds) override;
	void ArtilleryStrikeAtPlayer();
	int m_beatOffset = 0;
	Timer m_lockOnTimer;
};

struct SpawnUnitProb
{
	SpawnUnitProb(std::string name, float prob)
		: m_unitName(name)
		, m_probability(prob)
	{ }
	std::string m_unitName;
	float m_probability = 0.f;
};

class ImperialDefaultBehaviour : public AIBehaviour
{
	friend class AIBehaviour;
public:
	ImperialDefaultBehaviour(AIBehaviourDefinition const& aiBehaviourDef, Actor& actor);
	virtual void OnBegin() override;
	virtual void OnEnd() override;
	virtual AIBehaviour* Tick(float deltaSeconds) override;
private:
	void SpawnUnit();
	std::vector<SpawnUnitProb> m_unitSpawnProbs;
	int m_minAttackCooldown = 5;
	int m_maxAttackCooldown = 10;

	int m_minSpawnCooldown = 4;
	int m_maxSpawnCooldown = 8;

	Timer m_attackPlayerTimer;
	Timer m_spawnTimer;
};
