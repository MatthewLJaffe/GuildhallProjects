#pragma once
#include "Game/GameCommon.hpp"

class MapDefinition
{
public:
	MapDefinition() = default;
	static void InitializeMapDefs();
	static MapDefinition const* GetMapDefFromName(std::string mapName);
	static std::vector<MapDefinition> s_mapDefinitions;
	std::string m_name;
	std::string m_mapImageName;
	IntVec2 m_mapImageOffset;
	IntVec2 m_dimensions;
	std::string m_fillTileType;
	std::string m_edgeTileType;
	std::string m_worm1TileType;
	std::string m_worm2TileType;
	std::string m_worm3TileType;
	int m_worm1Count = 0;
	int m_worm1MaxLength = 0;
	int m_worm2Count = 0;
	int m_worm2MaxLength = 0;
	int m_worm3Count = 0;
	int m_worm3MaxLength = 0;
	std::string m_startFloorTileType;
	std::string m_startBunkerTileType;
	std::string m_endFloorTileType;
	std::string m_endBunkerTileType;
	std::string m_entranceTileType = "MapEntry";
	std::string m_exitTileType = "MapExit";
	int m_leoCount = 0;
	int m_ariesCount = 0;
	int m_scorpioCount = 0;
	int m_taurusCount = 0;
	int m_capricornCount = 0;
	int m_sagittariusCount = 0;
	float m_aspectRatio = 1.f;
	int m_startBunkerSize = 5;
	int m_endBunkerSize = 6;
};