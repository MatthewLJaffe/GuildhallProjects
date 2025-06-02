#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Definition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

class TileDefinition : public Definition
{
public:
	//Methods all definitions should have
	static void InitializeDefinitions(const char* path);
	static void ClearDefinitions();
	static TileDefinition const* GetByName(const std::string& name);
	static std::vector<TileDefinition const*> s_tileDefinitions;
	bool LoadFromXmlElement(XmlElement const& element) override;

	//Getters
	static TileDefinition const* GetTileDefFromColor(Rgba8 const& color);
	static TileDefinition const* GetTileDefFromName(std::string const& name);
	AABB2 GetFloorUVs(SpriteSheet& spriteSheet) const;
	AABB2 GetCeilingUVs(SpriteSheet& spriteSheet) const;
	AABB2 GetWallUVs(SpriteSheet& spriteSheet) const;

public:
	//from xml
	bool m_isSolid;
	Rgba8 m_mapPixelColor;
	IntVec2 m_floorSpriteCoords;
	IntVec2 m_ceilingSpriteCoords;
	IntVec2 m_wallSpriteCoords;
	bool m_isLit = false;
};