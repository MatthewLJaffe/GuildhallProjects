#include "Game/GameCommon.hpp"
class SpriteDefinition;

class SpriteSheet
{
public:
	explicit SpriteSheet ( Texture* texture, IntVec2 const& simpleGridLayout );

	Texture* GetTexture() const;
	int GetNumSprites() const;
	SpriteDefinition const& GetSpriteDef( int spriteIndex );
	void GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex ) const;
	AABB2 GetSpriteUVs( int spriteIndex ) const;
protected:
	Texture* m_texture;
	std::vector<SpriteDefinition> m_spriteDefs;
};