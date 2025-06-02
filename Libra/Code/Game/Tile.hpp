#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"

class TileDefinition;

class Tile
{

public:
	Rgba8 GetTileColor() const;
	AABB2 GetTileBounds() const;
	bool IsTileSolid() const;
	bool IsTileOpaque() const;
	bool IsTileWater() const;
	void SetTileDefinition(std::string const& name);
	TileDefinition const* m_tileDef = nullptr;
	float m_health = 0;
	IntVec2 m_tileCoords = IntVec2(-1, -1);
};