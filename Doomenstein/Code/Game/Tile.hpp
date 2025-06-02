#pragma once
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/AABB2.hpp"

class TileDefinition;

struct Tile
{
	Tile(AABB3 const&  bounds, TileDefinition const* tileDef);
	AABB3 m_bounds;
	TileDefinition const* m_def;

	bool IsTileSolid() const;
	AABB2 GetBounds2D() const;
};