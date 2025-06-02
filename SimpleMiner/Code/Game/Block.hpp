#pragma once
typedef unsigned char BlockType;

constexpr unsigned char NUM_INDOOR_BITS = 4;
constexpr unsigned char NUM_OUTDOOR_BITS = 4;
constexpr unsigned char INDOOR_LIGHTING_MASK =0b00001111;
constexpr unsigned char OUTDOOR_LIGHTING_MASK = 0b11110000;
constexpr unsigned char IS_SKY_BITMASK = 0b00000001;
constexpr unsigned char IS_SKY_BIT_POS = 0;
constexpr unsigned char IS_LIGHT_DIRTY_BITMASK = 0b00000010;
constexpr unsigned char IS_LIGHT_DIRTY_BIT_POS = 1;

enum class BlockFace
{
	TOP = 0,
	BOTTOM = 1,
	NORTH = 2,
	EAST = 3,
	SOUTH = 4,
	WEST = 5,
	COUNT = 6
};

struct BlockDefinition;

struct Block
{
	Block() = default;
	Block(BlockType blockType);
	BlockType m_blockType = 0;
	//outdoor light influence 0-15 stored in high bits indoor light influence 0-15 stored in low bits
	unsigned char m_lightInfluence = 0;
	unsigned char m_bitFlags = 0;
	BlockDefinition* GetBlockDef() const;

	int GetIndoorLightInfluence() const;
	void SetIndoorLightInfluence(unsigned char indoorInfluence);
	int GetOutdoorLightInfluence() const;
	void SetOutdoorLightInfluence(unsigned char outdoorInfluence);
	bool IsBlockSky() const;
	bool IsBlockSolid() const;
	bool IsBlockOpaque() const;
	void SetIsBlockSky(bool isSky);
	bool IsLightDirty() const;
	void SetIsLightDirty(bool isSky);
};

