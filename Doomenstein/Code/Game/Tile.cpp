#include "Game/Tile.hpp"
#include "Game/TileDefinition.hpp"

Tile::Tile(AABB3 const& bounds, TileDefinition const* tileDef)
	: m_bounds(bounds)
	, m_def(tileDef)
{ }

bool Tile::IsTileSolid() const
{
	return m_def->m_isSolid;
}

AABB2 Tile::GetBounds2D() const
{
	return AABB2(Vec2(m_bounds.m_mins.x, m_bounds.m_mins.y), Vec2(m_bounds.m_maxs.x, m_bounds.m_maxs.y));
}
