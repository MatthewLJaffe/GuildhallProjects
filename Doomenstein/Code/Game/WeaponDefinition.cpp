#include "Game/WeaponDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

std::vector<WeaponDefinition const*> WeaponDefinition::s_weaponDefinitions;

WeaponDefinition::~WeaponDefinition()
{
	delete m_attackAnimation;
	delete m_idleAnimation;
}

void WeaponDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument gameConfigDocument;
	GUARANTEE_OR_DIE(gameConfigDocument.LoadFile(path) == 0, std::string("Failed to load ") + std::string("Data/Definitions/WeaponDefinitions.xml"));
	XmlElement* rootElement = gameConfigDocument.FirstChildElement("Definitions");
	GUARANTEE_OR_DIE(rootElement != nullptr, std::string("Could not get the root GameConfig element from ") + std::string("Data/Definitions/WeaponDefinitions.xml"));
	for (XmlElement const* currElement = rootElement->FirstChildElement(); currElement != nullptr; currElement = currElement->NextSiblingElement())
	{
		WeaponDefinition* weaponDef = new WeaponDefinition();
		weaponDef->LoadFromXmlElement(*currElement);
		s_weaponDefinitions.push_back(weaponDef);
	}
}

void WeaponDefinition::ClearDefinitions()
{
	s_weaponDefinitions.clear();
}

WeaponDefinition const* WeaponDefinition::GetByName(const std::string& name)
{
	for (size_t i = 0; i < s_weaponDefinitions.size(); i++)
	{
		if (s_weaponDefinitions[i]->m_name == name)
		{
			return s_weaponDefinitions[i];
		}
	}
	return nullptr;
}

bool WeaponDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_meleeArc = ParseXmlAttribute(element, "meleeArc", 0.f);
	m_meleeCount = ParseXmlAttribute(element, "meleeCount", 0);
	m_meleeDamage = ParseXmlAttribute(element, "meleeDamage", FloatRange(0.f, 0.f));
	m_meleeImpulse = ParseXmlAttribute(element, "meleeImpulse", 0.f);
	m_meleeRange = ParseXmlAttribute(element, "meleeRange", 0.f);
	m_name = ParseXmlAttribute(element, "name", "Name not found");
	std::string projectileActorName = ParseXmlAttribute(element, "projectileActor", "Projectile Actor not found");
	m_projectileActor = ActorDefinition::GetByName(projectileActorName);
	m_projectileCone = ParseXmlAttribute(element, "projectileCone", 0.f);
	m_projectileCount = ParseXmlAttribute(element, "projectileCount", 0);
	m_projectileSpeed = ParseXmlAttribute(element, "projectileSpeed", 0.f);
	m_rayCone = ParseXmlAttribute(element, "rayCone", 0.f);
	m_rayCount = ParseXmlAttribute(element, "rayCount", 0);
	m_rayDamage = ParseXmlAttribute(element, "rayDamage", FloatRange(0.f, 0.f));
	m_rayImpulse = ParseXmlAttribute(element, "rayImpulse", 0.f);
	m_rayRange = ParseXmlAttribute(element, "rayRange", 0.f);
	m_refireTime = ParseXmlAttribute(element, "refireTime", 0.f);

	XmlElement const* soundsElement = element.FirstChildElement("Sounds");
	if (soundsElement != nullptr)
	{
		ParseSounds(*soundsElement);
	}

	XmlElement const* hudElement = element.FirstChildElement("HUD");
	if (hudElement != nullptr)
	{
		ParseHud(*hudElement);
	}
	return true;
}

void WeaponDefinition::ParseSounds(XmlElement const& soundsElement)
{
	m_soundPlayCooldown = ParseXmlAttribute(soundsElement, "soundPlayCooldown", 0.f);
	for (XmlElement const* currSound = soundsElement.FirstChildElement("Sound"); currSound != nullptr; currSound = currSound->NextSiblingElement("Sound"))
	{
		std::string soundType = ParseXmlAttribute(*currSound, "sound", "Missing this sound");
		if (soundType == std::string("Fire"))
		{
			m_fireSound = g_theAudio->CreateOrGetSound(ParseXmlAttribute(*currSound, "name", "Missing file path"), true);
		}
	}
}

void WeaponDefinition::ParseHud(XmlElement const& hudElement)
{
	m_hudShader = g_theRenderer->GetShader(ParseXmlAttribute(hudElement, "shader", "no shader"));
	m_reticleTexture = g_theRenderer->CreateOrGetTextureFromFile(ParseXmlAttribute(hudElement, "reticleTexture", "no texture").c_str());
	m_baseTexture = g_theRenderer->CreateOrGetTextureFromFile(ParseXmlAttribute(hudElement, "baseTexture", "no texture").c_str());
	m_reticleSize = ParseXmlAttribute(hudElement, "reticleSize", Vec2::ZERO);
	m_spriteSize = ParseXmlAttribute(hudElement, "spriteSize", Vec2::ZERO);
	m_spritePivot = ParseXmlAttribute(hudElement, "spritePivot", Vec2::ZERO);

	for (XmlElement const* currElement = hudElement.FirstChildElement("Animation"); currElement != nullptr; currElement = currElement->NextSiblingElement("Animation"))
	{
		std::string animationName = ParseXmlAttribute(*currElement, "name", "no name");
		Shader* animationShader = g_theRenderer->GetShader(ParseXmlAttribute(*currElement, "hudShader", "no shader"));
		IntVec2 cellCount = ParseXmlAttribute(*currElement, "cellCount", IntVec2(0, 0));
		SpriteSheet* spriteSheet = g_theRenderer->CreateOrGetSpriteSheetFromFile(ParseXmlAttribute(*currElement, "spriteSheet", "no texture").c_str(), cellCount);
		float secondsPerFrame = ParseXmlAttribute(*currElement, "secondsPerFrame", 0.f);
		int startFrame = ParseXmlAttribute(*currElement, "startFrame", 0);
		int endFrame = ParseXmlAttribute(*currElement, "endFrame", 0);


		if (animationName == "Attack")
		{
			float animDuration = float((endFrame - startFrame) + 1) * secondsPerFrame;
			m_attackAnimation = new SpriteAnimDefinition(*spriteSheet, startFrame, endFrame, animDuration, SpriteAnimPlaybackType::ONCE, animationShader);
		}
		if (animationName == "Idle")
		{
			float animDuration = float(endFrame - startFrame) * secondsPerFrame;
			m_idleAnimation = new SpriteAnimDefinition(*spriteSheet, startFrame, endFrame, animDuration, SpriteAnimPlaybackType::LOOP, animationShader);
		}
	}
}
