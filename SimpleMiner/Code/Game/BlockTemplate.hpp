#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Game/Block.hpp"

struct BlockTemplateEntry
{
	BlockTemplateEntry(BlockType blockType, IntVec3 templateOffset);
	BlockType m_blocktype;
	IntVec3 m_templateOffset;
};

class BlockTemplate
{
public:
	std::vector<BlockTemplateEntry> m_templateEntries;
	IntVec3 m_templateOrigin;
};