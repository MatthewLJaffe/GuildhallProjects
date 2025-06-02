#include "Game/Block.hpp"
#include "Game/BlockDefinition.hpp"
#include "Game/GameCommon.hpp"

Block::Block(BlockType blockType)
    : m_blockType(blockType)
{
}

BlockDefinition* Block::GetBlockDef() const
{
    return BlockDefinition::GetBlockDefinitionByType(m_blockType);
}

int Block::GetIndoorLightInfluence() const
{
    return m_lightInfluence & INDOOR_LIGHTING_MASK;
}

int Block::GetOutdoorLightInfluence() const
{
	return (m_lightInfluence & OUTDOOR_LIGHTING_MASK) >> NUM_INDOOR_BITS;
}

bool Block::IsBlockSky() const
{
    return (bool)(m_bitFlags & IS_SKY_BITMASK);
}

bool Block::IsBlockSolid() const
{
    return m_blockType != 0;
}

bool Block::IsBlockOpaque() const
{
    return m_blockType != 0;
}

void Block::SetIsBlockSky(bool isSky)
{
    m_bitFlags &= ~IS_SKY_BITMASK;
    m_bitFlags |= (isSky << IS_SKY_BIT_POS);
}

void Block::SetIndoorLightInfluence(unsigned char indoorInfluence)
{
    //get rid of low bits
    m_lightInfluence &= OUTDOOR_LIGHTING_MASK;
    //copy low bits from indoor influence
    m_lightInfluence |= indoorInfluence;
}

void Block::SetOutdoorLightInfluence(unsigned char outdoorInfluence)
{
	//get rid of high bits
	m_lightInfluence &= INDOOR_LIGHTING_MASK;
    m_lightInfluence |= outdoorInfluence << NUM_INDOOR_BITS;
}

bool Block::IsLightDirty() const
{
	return (bool)(m_bitFlags & IS_LIGHT_DIRTY_BITMASK);
}

void Block::SetIsLightDirty(bool isSky)
{
	m_bitFlags &= ~IS_LIGHT_DIRTY_BITMASK;
	m_bitFlags |= (isSky << IS_LIGHT_DIRTY_BIT_POS);
}
