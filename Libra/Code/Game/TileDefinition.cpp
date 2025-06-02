#include "TileDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"

std::vector<TileDefinition> TileDefinition::s_tileDefinitions;

void TileDefinition::InitializeTileDefs(SpriteSheet const& tileSpriteSheet)
{
	XmlDocument gameConfigDocument;
	GUARANTEE_OR_DIE(gameConfigDocument.LoadFile("Data/Definitions/TileDefinitions.xml") == 0, "Failed to load Data/Definitions/TileDefinitions.xml");
	XmlElement* rootElement = gameConfigDocument.FirstChildElement("TileDefinitions");
	GUARANTEE_OR_DIE(rootElement != nullptr, "Could not get the root GameConfig element from Data/Definitions/TileDefinitions.xml");
	for (XmlElement const* currElement = rootElement->FirstChildElement(); currElement != nullptr; currElement = currElement->NextSiblingElement())
	{
		TileDefinition currTileDef;
		currTileDef.m_name = ParseXmlAttribute(*currElement, "name", "");
		currTileDef.m_isSolid = ParseXmlAttribute(*currElement, "isSolid", false);
		currTileDef.m_isWater = ParseXmlAttribute(*currElement, "isWater", false);
		IntVec2 spriteCoords = ParseXmlAttribute(*currElement, "spriteCoords", IntVec2(0, 0));
		currTileDef.m_mapColor = ParseXmlAttribute(*currElement, "mapColor", Rgba8(0,0,0,0));
		int spriteSheetIdx = spriteCoords.x + spriteCoords.y * tileSpriteSheet.m_simpleGridLayout.x;
		tileSpriteSheet.GetSpriteDef(spriteSheetIdx).GetUVs(currTileDef.m_uvAtMins, currTileDef.m_uvAtMaxs);
		currTileDef.m_tintColor = ParseXmlAttribute(*currElement, "tint", Rgba8::WHITE);
		currTileDef.m_isDestructible = ParseXmlAttribute(*currElement, "isDestructible", false);
		currTileDef.m_maxHealth = ParseXmlAttribute(*currElement, "maxHealth", 0.f);
		currTileDef.m_destroyedTileType = ParseXmlAttribute(*currElement, "destroyedTileType", "");

		s_tileDefinitions.push_back(currTileDef);
	}
}

TileDefinition const* TileDefinition::GetTileDefFromName(std::string tileName)
{
	for (size_t i = 0; i < s_tileDefinitions.size(); i++)
	{
		if (s_tileDefinitions[i].m_name == tileName)
		{
			return &s_tileDefinitions[i];
		}
	}
	GUARANTEE_OR_DIE(false, "Failed to get tile definition from name " + tileName);
	return nullptr;
}

TileDefinition const* TileDefinition::GetTileDefFromColor(Rgba8 const& color)
{
	for (size_t i = 0; i < s_tileDefinitions.size(); i++)
	{
		if (s_tileDefinitions[i].m_mapColor.r == color.r && 
			s_tileDefinitions[i].m_mapColor.g == color.g && s_tileDefinitions[i].m_mapColor.b == color.b)
		{
			return &s_tileDefinitions[i];
		}
	}
	GUARANTEE_OR_DIE(false, "Failed to get tile definition from color");
	return nullptr;
}

