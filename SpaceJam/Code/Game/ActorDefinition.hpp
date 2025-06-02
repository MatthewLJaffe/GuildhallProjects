#pragma once
#include "Game/GameCommon.hpp"
#include "Game/ModelDefinition.hpp"	
#include "Game/AIBehaviour.hpp"

enum class CollisionType
{
	NONE,
	AABB3,
	SPHERE
};

enum class Faction
{
	NEUTRAL,
	PLAYER,
	ENEMY
};

struct ActorSound
{
	SoundID m_soundID = MISSING_SOUND_ID;
	float m_volume = 1.f;
};

enum class ActorType
{
	ACTOR,
	PLAYER,
	THRALL,
};

struct ActorDefinition
{
	ModelDefinition const* GetModelDef() const;
	static std::vector<ActorDefinition> s_actorDefinitions;
	static void LoadActorDefinitionsFromXML(std::string actorDefsFilepath);
	static ActorDefinition const* GetActorDefinitionFromName(std::string actorDefName);
	void LoadFromXMLElement(XmlElement const& element);
	Faction GetFactionFromString(std::string const& factionAsStr);
	ActorType GetActorTypeFromString(std::string const& actorTypeAsStr);
	CollisionType GetCollisionTypeFromString(std::string const& collisionAsStr);
	std::string m_name;
	ActorType m_actorType = ActorType::ACTOR;
	Faction m_faction = Faction::NEUTRAL;
	bool m_bezierPath = false;
	float m_maxTargetAngle = 0.f;
	float m_lifetime = -1.f;
	EulerAngles m_rotationalVelocity;

	//life
	float m_health = 0.f;
	bool m_damageable = false;
	float m_overlapDamage = 0.f;

	//Death
	bool m_dieOnOverlap = false;
	std::string m_dieParticleEffect = "";
	float m_dieParticleEffectDuration = 0.f;
	bool m_orientDieParticleEffect = false;
	float m_deathTime = 0.f;
	ActorSound m_hurtSound;
	ActorSound m_dieSound;
	std::string m_dieActorSpawn = "";
	float m_dieActorSpawnChance = 1.f;

	//physics
	CollisionType m_collisionType = CollisionType::NONE;
	float m_colliderSphereRadius = 0.f;
	AABB3 m_colliderAABB3;
	bool m_collidesWithFaction = true;
	bool m_debugDrawCollider = false;
	bool m_isTrigger = false;
	bool m_static = false;
	float m_acceleration = 20.f;
	float m_maxSpeed = 1000.f;
	int m_missleAmmoToDrop = 0;
	float m_megaLaserAmmoToDrop = 0.f;
	float m_healthToRestore = 0.f;

	//appearance
	int m_modelDefIndex = -1;
	std::vector<std::string> m_owendParticleEffectFilePaths;
	bool m_faceVelocity = false;
	bool m_facePlayer = false;
	std::string m_meshType = "model";

	//AI
	std::vector<AIBehaviourDefinition> m_behaviourDefs;

};