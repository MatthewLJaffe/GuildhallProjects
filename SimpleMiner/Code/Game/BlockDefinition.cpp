#include "Game/BlockDefinition.hpp"
#include "Game/Block.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

BlockDefinition* BlockDefinition::s_blockDefinitions[256] = { nullptr };

void BlockDefinition::CreateNewBlockDef(std::string name, bool visible, bool solid, bool opaque, IntVec2 const& topSprite, IntVec2 const& sideSprite, IntVec2 const& bottomSprite, int indoorLightEmitted)
{
	BlockDefinition* blockDef = new BlockDefinition();
	blockDef->m_blockName = name;
	blockDef->m_isVisible = visible;
	blockDef->m_isSolid = solid;
	blockDef->m_isOpaque = opaque;
	SpriteSheet const& sheet = *SPRITE_SHEET_BASIC_SPRITES;

	int topSpriteIdx = sheet.GetIndexFromCoords(topSprite);
	AABB2 topSpriteUVs = sheet.GetSpriteUVs(topSpriteIdx);
	blockDef->m_topSpriteDef = new SpriteDefinition(sheet, topSpriteIdx, topSpriteUVs.m_mins, topSpriteUVs.m_maxs);

	int sideSpriteIdx = sheet.GetIndexFromCoords(sideSprite);
	AABB2 sideSpriteUVs = sheet.GetSpriteUVs(sideSpriteIdx);
	blockDef->m_sideSpriteDef = new SpriteDefinition(sheet, sideSpriteIdx, sideSpriteUVs.m_mins, sideSpriteUVs.m_maxs);

	int bottomSpriteIdx = sheet.GetIndexFromCoords(bottomSprite);
	AABB2 bottomSpriteUVs = sheet.GetSpriteUVs(bottomSpriteIdx);
	blockDef->m_bottomSpriteDef = new SpriteDefinition(sheet, bottomSpriteIdx, bottomSpriteUVs.m_mins, bottomSpriteUVs.m_maxs);

	blockDef->m_indoorLightEmitted = indoorLightEmitted;

	for (int i = 0; i < 256; i++)
	{
		if (s_blockDefinitions[i] == nullptr)
		{
			blockDef->m_blockType = (BlockType)i;
			s_blockDefinitions[i] = blockDef;
			return;
		}
	}
}

BlockDefinition* BlockDefinition::GetBlockDefinitionByName(std::string blockName)
{
	for (size_t i = 0; i < 256; i++)
	{
		if (s_blockDefinitions[i]->m_blockName == blockName)
		{
			return s_blockDefinitions[i];
		}
	}

	return nullptr;
}

BlockDefinition* BlockDefinition::GetBlockDefinitionByType(BlockType blockType)
{
	for (size_t i = 0; i < 256; i++)
	{
		if (s_blockDefinitions[i]->m_blockType == blockType)
		{
			return s_blockDefinitions[i];
		}
	}

	return nullptr;
}

void BlockDefinition::DeleteBlockDefinitions()
{
	for (int i = 0; i < 256; i++)
	{
		if (s_blockDefinitions[i] == nullptr)
		{
			continue;
		}

		delete s_blockDefinitions[i];
		s_blockDefinitions[i] = nullptr;
	}

}
