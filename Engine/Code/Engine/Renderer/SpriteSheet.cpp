#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"

SpriteSheet::SpriteSheet(Texture* texture, IntVec2 const& simpleGridLayout)
	: m_texture(texture)
	, m_simpleGridLayout(simpleGridLayout)
{
	IntVec2 sheetDimensions = m_texture->GetDimensions();
	Vec2 uvCorrectionOffset(1.f / (static_cast<float>(sheetDimensions.x) * 128.f), 1.f / (static_cast<float>(sheetDimensions.y) * 128.f));
	for (int y = 0; y < simpleGridLayout.y; y++)
	{
		for (int x = 0; x < simpleGridLayout.x; x++)
		{
			int spriteIndex = x + y*simpleGridLayout.x;
			Vec2 uvAtMins;
			uvAtMins.x = (float)x / (float)simpleGridLayout.x;
			uvAtMins.y = (float)((simpleGridLayout.y) - ( y + 1)) / (float)(simpleGridLayout.y);
			Vec2 uvAtMaxs;
			uvAtMaxs.x = (float)(x + 1) / (float)simpleGridLayout.x;
			uvAtMaxs.y = (float)(simpleGridLayout.y - y) / (float)simpleGridLayout.y;
			uvAtMins += uvCorrectionOffset;
			uvAtMaxs -= uvCorrectionOffset;
			m_spriteDefs.push_back(SpriteDefinition(*this, spriteIndex, uvAtMins, uvAtMaxs));
		}
	}
}

Texture* SpriteSheet::GetTexture() const
{
	return m_texture;
}

int SpriteSheet::GetNumSprites() const
{
	return static_cast<int>(m_spriteDefs.size());
}

SpriteDefinition const& SpriteSheet::GetSpriteDef(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex];
}

void SpriteSheet::GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const
{
	m_spriteDefs[spriteIndex].GetUVs(out_uvAtMins, out_uvAtMaxs);
}

AABB2 SpriteSheet::GetSpriteUVs(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex].GetUVs();
}

int SpriteSheet::GetIndexFromCoords(IntVec2 const& spriteCoords) const
{
	return spriteCoords.x + spriteCoords.y * m_simpleGridLayout.x;
}

