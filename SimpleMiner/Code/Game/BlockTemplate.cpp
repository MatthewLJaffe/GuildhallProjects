#include "Game/BlockTemplate.hpp"

BlockTemplateEntry::BlockTemplateEntry(BlockType blockType, IntVec3 templateOffset)
	: m_blocktype(blockType)
	, m_templateOffset(templateOffset)
{
}
