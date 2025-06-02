#include "Game/ActorDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/WeaponDefinition.hpp"

std::vector<ActorDefinition const*> ActorDefinition::s_actorDefinitions;

ActorDefinition::~ActorDefinition()
{
	if (m_spriteSheet != nullptr)
	{
		delete m_spriteSheet;
	}
}

void ActorDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument gameConfigDocument;
	GUARANTEE_OR_DIE(gameConfigDocument.LoadFile(path) == 0, std::string("Failed to load ") + std::string("Data/Definitions/ActorDefinitions.xml"));
	XmlElement* rootElement = gameConfigDocument.FirstChildElement("Definitions");
	GUARANTEE_OR_DIE(rootElement != nullptr, std::string("Could not get the root GameConfig element from ") + std::string("Data/Definitions/ActorDefinitions.xml"));
	for (XmlElement const* currElement = rootElement->FirstChildElement(); currElement != nullptr; currElement = currElement->NextSiblingElement())
	{
		ActorDefinition* actorDef = new ActorDefinition();
		actorDef->LoadFromXmlElement(*currElement);
		s_actorDefinitions.push_back(actorDef);
	}
}

void ActorDefinition::ClearDefinitions()
{
	s_actorDefinitions.clear();
}

ActorDefinition const* ActorDefinition::GetByName(const std::string& name)
{
	for (size_t i = 0; i < s_actorDefinitions.size(); i++)
	{
		if (s_actorDefinitions[i]->m_name == name)
		{
			return s_actorDefinitions[i];
		}
	}
	return nullptr;
}

ActorDefinition const* ActorDefinition::GetByColor(Rgba8 const& color)
{
	for (size_t i = 0; i < s_actorDefinitions.size(); i++)
	{
		if (s_actorDefinitions[i]->m_mapColor.r == color.r && s_actorDefinitions[i]->m_mapColor.g == color.g && s_actorDefinitions[i]->m_mapColor.b == color.b)
		{
			return s_actorDefinitions[i];
		}
	}
	return nullptr;
}

bool ActorDefinition::LoadFromXmlElement(XmlElement const& element)
{
	//general
	m_name = ParseXmlAttribute(element, "name", "missing name");
	m_faction = GetFactionFromAttributeName(ParseXmlAttribute(element, "faction", "none"));
	m_health = ParseXmlAttribute(element, "health", 1.f);
	m_canBePossesed = ParseXmlAttribute(element, "canBePossessed", false);
	m_corpseLifetime = ParseXmlAttribute(element, "corpseLifetime", 0.f);
	m_lifetime = ParseXmlAttribute(element, "lifetime", -1.f);
	m_visible = ParseXmlAttribute(element, "visible", false);
	m_dieOnSpawn = ParseXmlAttribute(element, "dieOnSpawn", false);
	m_mapColor = ParseXmlAttribute(element, "mapColor", Rgba8::MAGENTA);

	//collision
	XmlElement const* collisionElement = element.FirstChildElement("Collision");
	if (collisionElement != nullptr)
	{
		m_collisionRadius = ParseXmlAttribute(*collisionElement, "radius", 0.f);
		m_collisionHeight = ParseXmlAttribute(*collisionElement, "height", 0.f);
		m_collidesWithActors = ParseXmlAttribute(*collisionElement, "collidesWithActors", false);
		m_collidesWithWorld = ParseXmlAttribute(*collisionElement, "collidesWithWorld", false);
		m_damageOnCollide =  ParseXmlAttribute(*collisionElement, "damageOnCollide", FloatRange(0.f, 0.f));
		m_impulseOnCollide = ParseXmlAttribute(*collisionElement, "impulseOnCollide", 0.f);
		m_dieOnCollide = ParseXmlAttribute(*collisionElement, "dieOnCollide", false);
		m_collidesWithSelf = ParseXmlAttribute(*collisionElement, "collidesWithSelf", true);
		m_spawnActorOnFloor = ParseXmlAttribute(*collisionElement, "spawnActorOnFloor", "");
		m_damageable = ParseXmlAttribute(*collisionElement, "damageable", false);
		m_pickupWeapon = ParseXmlAttribute(*collisionElement, "pickupWeapon", "");
	}

	//physics
	XmlElement const* physicsElement = element.FirstChildElement("Physics");
	if (physicsElement != nullptr)
	{
		m_physicsSimulated = ParseXmlAttribute(*physicsElement, "simulated", false);
		m_walkSpeed = ParseXmlAttribute(*physicsElement, "walkSpeed", 1.f);
		m_runSpeed = ParseXmlAttribute(*physicsElement, "runSpeed", 2.f);
		m_turnSpeed = ParseXmlAttribute(*physicsElement, "turnSpeed", 180.f);
		m_drag = ParseXmlAttribute(*physicsElement, "drag", 1.f);
		m_isFlying = ParseXmlAttribute(*physicsElement, "flying", false);
		m_isStatic = ParseXmlAttribute(*physicsElement, "isStatic", false);
	}

	//camera
	XmlElement const* cameraElement = element.FirstChildElement("Camera");
	if (cameraElement != nullptr)
	{
		m_eyeHeight = ParseXmlAttribute(*cameraElement, "eyeHeight", .5f);
		m_cameraFOV = ParseXmlAttribute(*cameraElement, "cameraFOV", 60.f);
	}

	//Visuals
	XmlElement const* visualsElement = element.FirstChildElement("Visuals");
	if (visualsElement != nullptr)
	{
		m_billboardSize = ParseXmlAttribute(*visualsElement, "size", Vec2::ONE);
		m_billboardPivot = ParseXmlAttribute(*visualsElement, "pivot", Vec2::ONE);
		m_billboardType = GetBillboardTypeFromAttributeName(ParseXmlAttribute(*visualsElement, "billboardType", "WorldUpFacing"));
		m_renderLit = ParseXmlAttribute(*visualsElement, "renderLit", false);
		m_renderRounded = ParseXmlAttribute(*visualsElement, "renderRounded", false);
		VertexType vertTypeToUse = m_renderLit ? VertexType::VERTEX_TYPE_PCUTBN : VertexType::VERTEX_TYPE_PCU;
		m_shader = g_theRenderer->CreateOrGetShaderFromFile(ParseXmlAttribute(*visualsElement, "shader", "not found").c_str(), vertTypeToUse);
		IntVec2 spriteSheetDimensions = ParseXmlAttribute(*visualsElement, "cellCount", IntVec2(1, 1));
		std::string spriteSheetPath = ParseXmlAttribute(*visualsElement, "spriteSheet", "not found");
		m_spriteSheet = g_theRenderer->CreateOrGetSpriteSheetFromFile(spriteSheetPath.c_str(), spriteSheetDimensions);
		ParseAnimationGroups(*visualsElement);
		m_depthDisabled = ParseXmlAttribute(*visualsElement, "depthDisabled", false);
		m_blendMode = GetBlendModeFromAttributeName(ParseXmlAttribute(*visualsElement, "blendMode", "Alpha"));
		m_tint = ParseXmlAttribute(*visualsElement, "tint", Rgba8::WHITE);
		m_spawnPointLight = ParseXmlAttribute(*visualsElement, "spawnPointLight", false);
	}

	//AI
	XmlElement const* aiElement = element.FirstChildElement("AI");
	if (aiElement != nullptr)
	{
		m_aiEnabled = ParseXmlAttribute(*aiElement, "aiEnabled", false);
		m_sightAngle = ParseXmlAttribute(*aiElement, "sightAngle", 0.f);
		m_sightRadius = ParseXmlAttribute(*aiElement, "sightRadius", 0.f);
		m_binkeyAI = ParseXmlAttribute(*aiElement, "binkeyAI", false);
	}
	
	XmlElement const* soundsElement = element.FirstChildElement("Sounds");
	if (soundsElement != nullptr) 
	{
		ParseSounds(*soundsElement);
	}

	//inventory
	XmlElement const* inventoryElement = element.FirstChildElement("Inventory");
	if (inventoryElement != nullptr)
	{
		for (XmlElement const* currElement = inventoryElement->FirstChildElement("Weapon"); currElement != nullptr; currElement = currElement->NextSiblingElement("Weapon"))
		{
			std::string weaponName = ParseXmlAttribute(*currElement, "name", "name not found");
			m_inventory.push_back(WeaponDefinition::GetByName(weaponName));
		}
	}

	return true;
}


void ActorDefinition::ParseSounds(XmlElement const& soundsElement)
{
	for (XmlElement const* currSound = soundsElement.FirstChildElement("Sound"); currSound != nullptr; currSound = currSound->NextSiblingElement("Sound"))
	{
		std::string soundType = ParseXmlAttribute(*currSound, "sound", "Missing this sound");
		if (soundType == std::string("Hurt"))
		{
			m_hurtSound = g_theAudio->CreateOrGetSound(ParseXmlAttribute(*currSound, "name", "Missing file path"), true);
		}
		if (soundType == std::string("Death"))
		{
			m_deathSound = g_theAudio->CreateOrGetSound(ParseXmlAttribute(*currSound, "name", "Missing file path"), true);
		}
	}
}

void ActorDefinition::ParseAnimationGroups(XmlElement const& visualsElement)
{
	for (XmlElement const* currGroupElement = visualsElement.FirstChildElement("AnimationGroup"); currGroupElement != nullptr; currGroupElement = currGroupElement->NextSiblingElement("AnimationGroup"))
	{
		AnimationGroup currAnimationGroup;
		currAnimationGroup.m_name = ParseXmlAttribute(*currGroupElement, "name", "no name");
		currAnimationGroup.m_scaleBySpeed = ParseXmlAttribute(*currGroupElement, "scaleBySpeed", false);
		currAnimationGroup.m_secondsPerFrame = ParseXmlAttribute(*currGroupElement, "secondsPerFrame", 0.f);
		currAnimationGroup.m_playbackMode = GetPlaybackTypeFromAttributeName(ParseXmlAttribute(*currGroupElement, "playbackMode", "missing"));
		for (XmlElement const* directionElement = currGroupElement->FirstChildElement("Direction"); directionElement != nullptr; directionElement = directionElement->NextSiblingElement("Direction"))
		{
			XmlElement const* animationElement = directionElement->FirstChildElement("Animation");
			if (animationElement == nullptr)
			{
				continue;
			}

			Vec3 dirVector = ParseXmlAttribute(*directionElement, "vector", Vec3::ZERO).GetNormalized();
			int startFrame = ParseXmlAttribute(*animationElement, "startFrame", 0);
			int endFrame = ParseXmlAttribute(*animationElement, "endFrame", 0);
			float animationDuration = ((float) (endFrame - startFrame + 1)) * currAnimationGroup.m_secondsPerFrame;
			SpriteAnimDefinition animDef(*m_spriteSheet, startFrame, endFrame, animationDuration, currAnimationGroup.m_playbackMode, m_shader);
			
			AnimationDirection currAnimDirection(dirVector, animDef);
			currAnimationGroup.m_animationDirections.push_back(currAnimDirection);
		}
		currAnimationGroup.m_numFrames = currAnimationGroup.m_animationDirections[0].m_animationDef.GetNumFrames();
		m_animationGroups.push_back(currAnimationGroup);
	}
}

Faction ActorDefinition::GetFactionFromAttributeName(std::string factionName)
{
	if (factionName._Equal("Marine"))
	{
		return Faction::MARINE;
	}
	else if (factionName._Equal("Demon"))
	{
		return Faction::DEMON;
	}
	else
	{
		return Faction::NEUTRAL;
	}
}

BlendMode ActorDefinition::GetBlendModeFromAttributeName(std::string blendModeName)
{
	if (blendModeName == "Alpha")
	{
		return BlendMode::ALPHA;
	}
	if (blendModeName == "Additive")
	{
		return BlendMode::ADDITIVE;
	}
	if (blendModeName == "Opaque")
	{
		return BlendMode::OPAQUE;
	}
	return BlendMode::ALPHA;
}

BillboardType ActorDefinition::GetBillboardTypeFromAttributeName(std::string billboardName)
{
	if (billboardName == "FullFacing")
	{
		return BillboardType::FULL_FACING;
	}
	if (billboardName == "FullOpposing")
	{
		return BillboardType::FULL_OPPOSING;
	}
	if (billboardName == "WorldUpFacing")
	{
		return BillboardType::WORLD_UP_FACING;
	}
	if (billboardName == "WorldUpOpposing")
	{
		return BillboardType::WORLD_UP_OPPOSING;
	}
	return BillboardType::NONE;
}

SpriteAnimPlaybackType ActorDefinition::GetPlaybackTypeFromAttributeName(std::string attributeName)
{
	if (attributeName == "Once")
	{
		return SpriteAnimPlaybackType::ONCE;
	}
	if (attributeName == "Loop")
	{
		return SpriteAnimPlaybackType::LOOP;
	}
	if (attributeName == "PingPong" || attributeName == "Pingpong")
	{
		return SpriteAnimPlaybackType::PINGPONG;
	}
	return SpriteAnimPlaybackType::ONCE;
}

AnimationDirection::AnimationDirection(Vec3 const& direction, SpriteAnimDefinition const& animationDef)
	: m_directionVector(direction)
	, m_animationDef(animationDef)
{
}
