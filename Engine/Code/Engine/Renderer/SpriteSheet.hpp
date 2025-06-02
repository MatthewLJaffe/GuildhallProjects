#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"

class Texture;
class AABB2;
class SpriteDefinition;

class SpriteSheet
{
public:
	explicit SpriteSheet ( Texture* texture, IntVec2 const& simpleGridLayout );
	IntVec2 m_simpleGridLayout;
	Texture* GetTexture() const;
	int GetNumSprites() const;
	SpriteDefinition const& GetSpriteDef( int spriteIndex ) const;
	void GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex ) const;
	AABB2 GetSpriteUVs( int spriteIndex ) const;
	int GetIndexFromCoords(IntVec2 const& spriteCoords) const;
protected:
	Texture* m_texture;
	std::vector<SpriteDefinition> m_spriteDefs;
};