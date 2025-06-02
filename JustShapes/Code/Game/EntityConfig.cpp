#include "Game/EntityConfig.hpp"
#include "Game/GameCommon.hpp"

std::vector<EntityConfig> EntityConfig::s_entityConfigs;

EntityConfig EntityConfig::GetEntityConfigByName(std::string configName)
{
	for (int i = 0; i < (int)s_entityConfigs.size(); i++)
	{
		if (s_entityConfigs[i].m_name == configName)
		{
			return s_entityConfigs[i];
		}
	}
	ERROR_RECOVERABLE(Stringf("Could not find EntityConfig by name %s", configName.c_str()));
	return EntityConfig();
}

void EntityConfig::InitializeEntityConfigs()
{
	XmlDocument gameConfigDocument;
	GUARANTEE_OR_DIE(gameConfigDocument.LoadFile("Data/Definitions/EntityConfigs.xml") == 0, "Failed to load Data/Definitions/EntityConfigs.xml");
	XmlElement* rootElement = gameConfigDocument.FirstChildElement("EntityConfigs");
	GUARANTEE_OR_DIE(rootElement != nullptr, "Could not get the root GameConfig element from Data/Definitions/EntityConfigs.xml");
	for (XmlElement const* currElement = rootElement->FirstChildElement("EntityConfig"); currElement != nullptr; currElement = currElement->NextSiblingElement("EntityConfig"))
	{
		EntityConfig currEntityConfig;
		currEntityConfig.m_useConfig = true;
		currEntityConfig.LoadFromXmlElement(*currElement);
		s_entityConfigs.push_back(currEntityConfig);
	}
}

void EntityConfig::LoadFromXmlElement(XmlElement const& entityConfigElement)
{
	m_name = ParseXmlAttribute(entityConfigElement, "name", "");
	XmlElement const* visualsElement = entityConfigElement.FirstChildElement("Visual");
	if (visualsElement != nullptr)
	{
		m_startOrientaiton = ParseXmlAttribute(*visualsElement, "startOrientation", 0.f);
		Vec2 spriteBoundsMins = ParseXmlAttribute(*visualsElement, "spriteBoundsMins", Vec2(0, 0));
		Vec2 spriteBoundsMaxs = ParseXmlAttribute(*visualsElement, "spriteBoundsMaxs", Vec2(16.f, 16.f));
		m_spriteBounds = AABB2(spriteBoundsMins, spriteBoundsMaxs);
		m_normalizedPivot = ParseXmlAttribute(*visualsElement, "normalizedPivot", Vec2(.5f, .5f));
		std::string texturePath = ParseXmlAttribute(*visualsElement, "texture", "");
		if (texturePath != "")
		{
			m_texture = g_theRenderer->CreateOrGetTextureFromFile(texturePath.c_str());
		}
		m_startColor = ParseXmlAttribute(*visualsElement, "startColor", Rgba8::WHITE);
		m_sortOrder = ParseXmlAttribute(*visualsElement, "sortOrder", 0);
	}

	XmlElement const* spriteSheetElement = entityConfigElement.FirstChildElement("SpriteSheet");
	if (spriteSheetElement != nullptr)
	{
		IntVec2 spriteSheetDimensions = ParseXmlAttribute(*spriteSheetElement, "spriteSheetDimensions", IntVec2(1, 1));
		std::string spriteSheetPath = ParseXmlAttribute(*spriteSheetElement, "spriteSheet","Missing path");
		SpriteSheet const* spriteSheet = g_theRenderer->CreateOrGetSpriteSheetFromFile(spriteSheetPath.c_str(), spriteSheetDimensions);
		IntVec2 spriteCoords = ParseXmlAttribute(*spriteSheetElement, "spriteCoords", IntVec2(0, 0));
		if (spriteSheet)
		{
			SpriteDefinition const& spriteDef = spriteSheet->GetSpriteDef(spriteSheet->GetIndexFromCoords(spriteCoords));
			m_texture = spriteDef.GetTexture();
			m_uvs = spriteDef.GetUVs();
		}
	}

	XmlElement const* lifetimeElement = entityConfigElement.FirstChildElement("Lifetime");
	if (lifetimeElement != nullptr)
	{
		m_liveTime = ParseXmlAttribute(*lifetimeElement, "liveTime", -1.f);
		m_startAnimation = ParseXmlAttribute(*lifetimeElement, "startAnimation", "");
		m_hideTime = ParseXmlAttribute(*lifetimeElement, "hideTime", -1.f);
	}

	XmlElement const* physicsElement = entityConfigElement.FirstChildElement("Physics");
	if (physicsElement != nullptr)
	{
		m_simulatePhysics = ParseXmlAttribute(*physicsElement, "simulatePhysics", false);
		m_startHazard = ParseXmlAttribute(*physicsElement, "startHazard", false);
		m_startVelocity = ParseXmlAttribute(*physicsElement, "startVelocity", Vec2::ZERO);
		m_startAcceleration = ParseXmlAttribute(*physicsElement, "startAcceleration", Vec2::ZERO);
		m_rotationSpeed = ParseXmlAttribute(*physicsElement, "rotationSpeed", 0.f);
		m_becomeHazardTime = ParseXmlAttribute(*physicsElement, "becomeHazardTime", -1.f);
		m_collisionRadius = ParseXmlAttribute(*physicsElement, "collisionRadius", 0.f);
	}

}
