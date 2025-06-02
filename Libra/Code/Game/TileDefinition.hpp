#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"

class TileDefinition
{
public:
	TileDefinition() = default;
	Rgba8 m_tintColor = Rgba8::MAGENTA;
	Rgba8 m_mapColor;
	bool m_isSolid = false;
	bool m_isWater = false;
	bool m_isDestructible  = false;
	float m_maxHealth = 0.f;
	std::string m_destroyedTileType = "";
	Vec2 m_uvAtMins = Vec2::ZERO;
	Vec2 m_uvAtMaxs = Vec2::ONE;
	std::string m_name;
	static void InitializeTileDefs(SpriteSheet const& tileSpriteSheet);
	static std::vector<TileDefinition> s_tileDefinitions;
	static TileDefinition const* GetTileDefFromName(std::string tileName);
	static TileDefinition const* GetTileDefFromColor(Rgba8 const& color);
};