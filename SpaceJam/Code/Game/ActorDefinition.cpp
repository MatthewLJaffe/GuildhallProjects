#include "ActorDefinition.hpp"

std::vector<ActorDefinition> ActorDefinition::s_actorDefinitions;

ModelDefinition const* ActorDefinition::GetModelDef() const
{
    if (m_modelDefIndex < 0)
    {
        return nullptr;
    }
    return ModelDefinition::s_modelDefinitions[m_modelDefIndex];
}

void ActorDefinition::LoadActorDefinitionsFromXML(std::string actorDefsFilepath)
{
	XmlDocument actorDefDocument;
	GUARANTEE_OR_DIE(actorDefDocument.LoadFile(actorDefsFilepath.c_str()) == 0, std::string("Failed to load ") + std::string("Data/Definitions/ActorDefinitions.xml"));
	XmlElement* rootElement = actorDefDocument.FirstChildElement("ActorDefinitions");
	GUARANTEE_OR_DIE(rootElement != nullptr, std::string("Could not get the root GameConfig element from ") + std::string("Data/Definitions/ActorDefinitions.xml"));
	for (XmlElement const* currElement = rootElement->FirstChildElement("ActorDefinition"); 
		currElement != nullptr; currElement = currElement->NextSiblingElement("ActorDefinition"))
	{
		int actorDefIndex = (int)s_actorDefinitions.size();
		s_actorDefinitions.emplace_back();
		s_actorDefinitions[actorDefIndex].LoadFromXMLElement(*currElement);
	}
}

ActorDefinition const* ActorDefinition::GetActorDefinitionFromName(std::string actorDefName)
{
	for (int i = 0; i < (int)s_actorDefinitions.size(); i++)
	{
		if (s_actorDefinitions[i].m_name == actorDefName)
		{
			return &s_actorDefinitions[i];
		}
	}
	return nullptr;
}

void ActorDefinition::LoadFromXMLElement(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", "Missing Name");
	std::string actorTypeAsStr = ParseXmlAttribute(element, "type", "Actor");
	m_actorType = GetActorTypeFromString(actorTypeAsStr);
	std::string factionAsStr = ParseXmlAttribute(element, "faction", "Neutral");
	m_faction = GetFactionFromString(factionAsStr);
	m_bezierPath = ParseXmlAttribute(element, "bezierPath", false);
	m_maxTargetAngle = ParseXmlAttribute(element, "maxTargetAngle", 0.f);

	//life
	XmlElement const* lifeElement = element.FirstChildElement("Life");
	if (lifeElement != nullptr)
	{
		m_health = ParseXmlAttribute(*lifeElement, "health", 0.f);
		m_damageable = ParseXmlAttribute(*lifeElement, "damageable", false);
		m_overlapDamage = ParseXmlAttribute(*lifeElement, "overlapDamage", 0.f);
		m_lifetime = ParseXmlAttribute(*lifeElement, "lifetime", -1.f);
	}

	//death
	XmlElement const* deathElement = element.FirstChildElement("Death");
	if (deathElement != nullptr)
	{
		m_deathTime = ParseXmlAttribute(*deathElement, "deathTime", 0.f);
		m_dieOnOverlap = ParseXmlAttribute(*deathElement, "dieOnOverlap", false);
		XmlElement const* dieParticleEffectElement = deathElement->FirstChildElement("DieParticleEffect");
		if (dieParticleEffectElement != nullptr)
		{
			m_dieParticleEffect = ParseXmlAttribute(*dieParticleEffectElement, "dieParticleEffectPath", "");
			m_dieParticleEffectDuration = ParseXmlAttribute(*dieParticleEffectElement, "dieParticleEffectDuration", 1.f);
			m_orientDieParticleEffect = ParseXmlAttribute(*dieParticleEffectElement, "orientDieParticleEffect", false);
		}

		XmlElement const* dieActorSpawnElement = deathElement->FirstChildElement("DieActorSpawn");
		if (dieActorSpawnElement != nullptr)
		{
			m_dieActorSpawn = ParseXmlAttribute(*dieActorSpawnElement, "name", "");
			m_dieActorSpawnChance = ParseXmlAttribute(*dieActorSpawnElement, "chance", 1.f);
		}
	}

	//physics
	XmlElement const* physics = element.FirstChildElement("Physics");
	if (physics != nullptr)
	{
		std::string collisionTypeAsStr = ParseXmlAttribute(*physics, "collisionType", "None");
		m_collisionType = GetCollisionTypeFromString(collisionTypeAsStr);
		m_maxSpeed = ParseXmlAttribute(*physics, "maxSpeed", 1000.f);
		m_acceleration = ParseXmlAttribute(*physics, "acceleration", 20.f);

		m_colliderSphereRadius = ParseXmlAttribute(*physics, "colliderSphereRadius", 0.f);
		m_colliderAABB3 = AABB3(ParseXmlAttribute(*physics, "colliderAABB3Mins", Vec3::ZERO), ParseXmlAttribute(*physics, "colliderAABB3Maxs", Vec3::ZERO));
		m_collidesWithFaction = ParseXmlAttribute(*physics, "collidesWithFaction", true);
		m_debugDrawCollider = ParseXmlAttribute(*physics, "debugDrawCollider", false);
		m_isTrigger = ParseXmlAttribute(*physics, "isTrigger", false);
		m_static = ParseXmlAttribute(*physics, "static", false);
		m_missleAmmoToDrop = ParseXmlAttribute(*physics, "missileAmmoToDrop", 0);
		m_megaLaserAmmoToDrop = ParseXmlAttribute(*physics, "megaLaserAmmoToDrop", 0.f);
		m_healthToRestore = ParseXmlAttribute(*physics, "healthToRestore", 0.f);

	}

	//appearance
	XmlElement const* appearance = element.FirstChildElement("Appearance");
	if (appearance != nullptr)
	{
		m_meshType = ParseXmlAttribute(*appearance, "meshType", "model");
		m_rotationalVelocity = ParseXmlAttribute(*appearance, "rotationalVelocity", EulerAngles());

		if (m_meshType == "model")
		{
			std::string modelFilePath = ParseXmlAttribute(*appearance, "modelFilePath", "");
			if (modelFilePath != "")
			{
				m_modelDefIndex = ModelDefinition::GetModelDefIndexFromFileName(modelFilePath);
			}
		}

		m_faceVelocity = ParseXmlAttribute(*appearance, "faceVelocity", false);
		m_facePlayer = ParseXmlAttribute(*appearance, "facePlayer", false);
		XmlElement const* ownedParticleEffects = appearance->FirstChildElement("OwnedParticleEffects");
		if (ownedParticleEffects != nullptr)
		{
			for (XmlElement const* currElement = ownedParticleEffects->FirstChildElement("OwnedParticleEffect");
				currElement != nullptr; currElement = currElement->NextSiblingElement("OwnedParticleEffect"))
			{
				std::string ownedParticleEffectPath = ParseXmlAttribute(*currElement, "path", "");
				if (ownedParticleEffectPath != "")
				{
					m_owendParticleEffectFilePaths.push_back(ownedParticleEffectPath);
				}
			}
		}
	}

	//sounds
	XmlElement const* sounds = element.FirstChildElement("Sounds");
	if (sounds != nullptr)
	{
		for (XmlElement const* currElement = sounds->FirstChildElement("Sound");
			currElement != nullptr; currElement = currElement->NextSiblingElement("Sound"))
		{
			std::string soundType= ParseXmlAttribute(*currElement, "sound", "");
			if (soundType == "Hurt")
			{
				std::string soundPath = ParseXmlAttribute(*currElement, "name", "");
				if (soundPath != "")
				{
					m_hurtSound.m_soundID = g_theAudio->CreateOrGetSound(soundPath);
				}
				m_hurtSound.m_volume = ParseXmlAttribute(*currElement, "volume", 1.f);
			}
			if (soundType == "Die")
			{
				std::string soundPath = ParseXmlAttribute(*currElement, "name", "");
				if (soundPath != "")
				{
					m_dieSound.m_soundID = g_theAudio->CreateOrGetSound(soundPath);
				}
				m_dieSound.m_volume = ParseXmlAttribute(*currElement, "volume", 1.f);
			}
		}
	}

	//AI
	XmlElement const* behaviours = element.FirstChildElement("Behaviours");
	if (behaviours != nullptr)
	{
		for (XmlElement const* currElement = behaviours->FirstChildElement("Behaviour");
			currElement != nullptr; currElement = currElement->NextSiblingElement("Behaviour"))
		{
			m_behaviourDefs.push_back(AIBehaviourDefinition::LoadFromXMLElement(*currElement));
		}
	}
}

Faction ActorDefinition::GetFactionFromString(std::string const& factionAsStr)
{
	if (ToLower(factionAsStr) == "neutral")
	{
		return Faction::NEUTRAL;
	}
	if (ToLower(factionAsStr) == "player")
	{
		return Faction::PLAYER;
	}
	if (ToLower(factionAsStr) == "enemy")
	{
		return Faction::ENEMY;
	}
	return Faction::NEUTRAL;
}

ActorType ActorDefinition::GetActorTypeFromString(std::string const& actorTypeAsStr)
{
	std::string actorTypeLower = ToLower(actorTypeAsStr);
	if (actorTypeLower == "thrall")
	{
		return ActorType::THRALL;
	}
	if (actorTypeLower == "player")
	{
		return ActorType::PLAYER;
	}
	return ActorType::ACTOR;
}

CollisionType ActorDefinition::GetCollisionTypeFromString(std::string const& collisionAsStr)
{
	std::string collisionLower = ToLower(collisionAsStr);
	if (collisionLower == "none")
	{
		return CollisionType::NONE;
	}
	if (collisionLower == "aabb3")
	{
		return CollisionType::AABB3;
	}
	if (collisionLower == "sphere")
	{
		return CollisionType::SPHERE;
	}
	return CollisionType::NONE;
}
