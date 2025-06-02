#include "TileDefinition.hpp"

#include "TileDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"

std::vector<TileDefinition const*> TileDefinition::s_tileDefinitions;

void TileDefinition::InitializeDefinitions(const char* path)
{
	XmlDocument gameConfigDocument;
	GUARANTEE_OR_DIE(gameConfigDocument.LoadFile(path) == 0, std::string("Failed to load ") + std::string("Data/Definitions/TileDefinitions.xml"));
	XmlElement* rootElement = gameConfigDocument.FirstChildElement("Definitions");
	GUARANTEE_OR_DIE(rootElement != nullptr, std::string("Could not get the root GameConfig element from ") + std::string("Data/Definitions/TileDefinitions.xml"));
	for (XmlElement const* currElement = rootElement->FirstChildElement(); currElement != nullptr; currElement = currElement->NextSiblingElement())
	{
		TileDefinition* tileDef = new TileDefinition();
		tileDef->LoadFromXmlElement(*currElement);
		s_tileDefinitions.push_back(tileDef);
	}
}

void TileDefinition::ClearDefinitions()
{
	for (size_t i = 0; i < s_tileDefinitions.size(); i++)
	{
		delete s_tileDefinitions[i];
		s_tileDefinitions[i] = nullptr;
	}
	s_tileDefinitions.clear();
}

TileDefinition const* TileDefinition::GetByName(const std::string& name)
{
	for (size_t i = 0; i < s_tileDefinitions.size(); i++)
	{
		if (s_tileDefinitions[i]->m_name == name)
		{
			return s_tileDefinitions[i];
		}
	}
	GUARANTEE_OR_DIE(false, "Failed to get tile definition from name " + name);
	return nullptr;
}

bool TileDefinition::LoadFromXmlElement(XmlElement const& element)
{
	m_isSolid = ParseXmlAttribute(element, "isSolid", false);
	m_ceilingSpriteCoords = ParseXmlAttribute(element, "ceilingSpriteCoords", IntVec2(-1, -1));
	m_floorSpriteCoords = ParseXmlAttribute(element, "floorSpriteCoords", IntVec2(-1, -1));
	m_wallSpriteCoords = ParseXmlAttribute(element, "wallSpriteCoords", IntVec2(-1, -1));
	m_mapPixelColor = ParseXmlAttribute(element, "mapImagePixelColor", Rgba8(255, 0, 255));
	m_name = ParseXmlAttribute(element, "name", "missing name");
	m_isLit = ParseXmlAttribute(element, "isLit", false);
	return true;
}

TileDefinition const* TileDefinition::GetTileDefFromColor(Rgba8 const& color)
{
	for (size_t i = 0; i < s_tileDefinitions.size(); i++)
	{
		if (s_tileDefinitions[i]->m_mapPixelColor.r == color.r &&
			s_tileDefinitions[i]->m_mapPixelColor.g == color.g && s_tileDefinitions[i]->m_mapPixelColor.b == color.b)
		{
			return s_tileDefinitions[i];
		}
	}
	GUARANTEE_OR_DIE(false, "Failed to get tile definition from color");
	return nullptr;
}

TileDefinition const* TileDefinition::GetTileDefFromName(std::string const& name)
{
	for (size_t i = 0; i < s_tileDefinitions.size(); i++)
	{
		if (s_tileDefinitions[i]->m_name == name)
		{
			return s_tileDefinitions[i];
		}
	}
	GUARANTEE_OR_DIE(false, "Failed to get tile definition from color");
	return nullptr;
}

AABB2 TileDefinition::GetFloorUVs(SpriteSheet& spriteSheet) const
{
	int spriteIdx = spriteSheet.GetIndexFromCoords(m_floorSpriteCoords);
	return spriteSheet.GetSpriteUVs(spriteIdx);
}

AABB2 TileDefinition::GetCeilingUVs(SpriteSheet& spriteSheet) const
{
	int spriteIdx = spriteSheet.GetIndexFromCoords(m_ceilingSpriteCoords);
	return spriteSheet.GetSpriteUVs(spriteIdx);
}

AABB2 TileDefinition::GetWallUVs(SpriteSheet& spriteSheet) const
{
	int spriteIdx = spriteSheet.GetIndexFromCoords(m_wallSpriteCoords);
	return spriteSheet.GetSpriteUVs(spriteIdx);
}




