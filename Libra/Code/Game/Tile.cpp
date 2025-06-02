#include "Game/Tile.hpp"
#include "Game/TileDefinition.hpp"

Rgba8 Tile::GetTileColor() const
{
	return m_tileDef->m_tintColor;
}

bool Tile::IsTileSolid() const
{
	return m_tileDef->m_isSolid;
}

bool Tile::IsTileOpaque() const
{
	return !m_tileDef->m_isWater && m_tileDef->m_isSolid;
}

bool Tile::IsTileWater() const
{
	return m_tileDef->m_isWater;
}


void Tile::SetTileDefinition(std::string const& name)
{
	m_tileDef = TileDefinition::GetTileDefFromName(name);
}

AABB2 Tile::GetTileBounds() const
{
	Vec2 tileMins((float)m_tileCoords.x, (float)m_tileCoords.y);
	Vec2 tileMaxs(tileMins.x + 1, tileMins.y + 1);
	return AABB2(tileMins, tileMaxs);
}
