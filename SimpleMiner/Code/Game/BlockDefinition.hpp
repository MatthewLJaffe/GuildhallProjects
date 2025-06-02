#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Game/Block.hpp"

struct BlockDefinition
{
	bool m_isVisible = false;
	bool m_isSolid = false;
	bool m_isOpaque = false;
	BlockType m_blockType = 0;
	std::string m_blockName = "";
	SpriteDefinition* m_topSpriteDef = nullptr;
	SpriteDefinition* m_sideSpriteDef = nullptr;
	SpriteDefinition* m_bottomSpriteDef = nullptr;
	int m_indoorLightEmitted = 0;
public:
	static void CreateNewBlockDef(std::string name, bool visible, bool solid, bool opaque, IntVec2 const& topSprite, IntVec2 const& sideSprite, IntVec2 const& bottomSprite, int indoorLightEmitted = 0);
	static BlockDefinition* GetBlockDefinitionByName(std::string blockName);
	static BlockDefinition* GetBlockDefinitionByType(BlockType blockType);
	static void DeleteBlockDefinitions();

private:
	static BlockDefinition* s_blockDefinitions[256];
};

