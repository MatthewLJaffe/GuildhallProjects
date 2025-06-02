#include "Game/MapDefinition.hpp"

std::vector<MapDefinition> MapDefinition::s_mapDefinitions;

void MapDefinition::InitializeMapDefs()
{
	XmlDocument gameConfigDocument;
	GUARANTEE_OR_DIE(gameConfigDocument.LoadFile("Data/Definitions/MapDefinitions.xml") == 0, "Failed to load Data/Definitions/MapDefinitions.xml");
	XmlElement* rootElement = gameConfigDocument.FirstChildElement("MapDefinitions");
	GUARANTEE_OR_DIE(rootElement != nullptr, "Could not get the root GameConfig element from Data/Definitions/MapDefinitions.xml");
	for (XmlElement const* currElement = rootElement->FirstChildElement(); currElement != nullptr; currElement = currElement->NextSiblingElement())
	{
		MapDefinition currTileDef;
		currTileDef.m_name = ParseXmlAttribute(*currElement, "name", "");
		currTileDef.m_dimensions = ParseXmlAttribute(*currElement, "dimensions", IntVec2(10, 10));
		currTileDef.m_fillTileType = ParseXmlAttribute(*currElement, "fillTileType", "LongGrass");
		currTileDef.m_edgeTileType = ParseXmlAttribute(*currElement, "edgeTileType", "RockWall");

		currTileDef.m_worm1TileType = ParseXmlAttribute(*currElement, "worm1TileType", "DarkGrass");
		currTileDef.m_worm2TileType = ParseXmlAttribute(*currElement, "worm2TileType", "RockWall");
		currTileDef.m_worm3TileType = ParseXmlAttribute(*currElement, "worm3TileType", "RockWall");

		currTileDef.m_worm1Count = ParseXmlAttribute(*currElement, "worm1Count", 0);
		currTileDef.m_worm2Count = ParseXmlAttribute(*currElement, "worm2Count", 0);
		currTileDef.m_worm3Count = ParseXmlAttribute(*currElement, "worm3Count", 0);

		currTileDef.m_worm1MaxLength = ParseXmlAttribute(*currElement, "worm1MaxLength", 0);
		currTileDef.m_worm2MaxLength = ParseXmlAttribute(*currElement, "worm2MaxLength", 0);
		currTileDef.m_worm3MaxLength = ParseXmlAttribute(*currElement, "worm3MaxLength", 0);

		currTileDef.m_startFloorTileType = ParseXmlAttribute(*currElement, "startFloorTileType", "Concrete");
		currTileDef.m_startBunkerTileType = ParseXmlAttribute(*currElement, "startBunkerTileType", "RockWall");
		currTileDef.m_endFloorTileType = ParseXmlAttribute(*currElement, "endFloorTileType", "Concrete");
		currTileDef.m_endBunkerTileType = ParseXmlAttribute(*currElement, "endBunkerTileType", "StoneWall");
		currTileDef.m_leoCount = ParseXmlAttribute(*currElement, "leoCount", 0);
		currTileDef.m_ariesCount = ParseXmlAttribute(*currElement, "ariesCount", 0);
		currTileDef.m_scorpioCount = ParseXmlAttribute(*currElement, "scorpioCount", 0);
		currTileDef.m_taurusCount = ParseXmlAttribute(*currElement, "taurusCount", 0);
		currTileDef.m_capricornCount = ParseXmlAttribute(*currElement, "capricornCount", 0);
		currTileDef.m_sagittariusCount = ParseXmlAttribute(*currElement, "sagittariusCount", 0);
		currTileDef.m_mapImageName = ParseXmlAttribute(*currElement, "mapImageName", "");
		currTileDef.m_mapImageOffset = ParseXmlAttribute(*currElement, "mapImageOffset", IntVec2::ZERO);
		s_mapDefinitions.push_back(currTileDef);
	}
}

MapDefinition const* MapDefinition::GetMapDefFromName(std::string mapName)
{
	for (size_t i = 0; i < s_mapDefinitions.size(); i++)
	{
		if (s_mapDefinitions[i].m_name == mapName)
		{
			return &s_mapDefinitions[i];
		}
	}
	GUARANTEE_OR_DIE(false, "Failed to get tile definition from name " + mapName);
	return nullptr;
}
