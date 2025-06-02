#pragma once
#include "Game/GameCommon.hpp"
class Chunk;
struct Block;


struct BlockIterator
{
	BlockIterator() = default;
	BlockIterator(Chunk* chunk, int blockIndex);
	Chunk* m_chunk = nullptr;
	int m_blockIndex = -1;
	Block* GetBlock();
	Vec3 GetWorldCenter();
	bool IsValid();
	BlockIterator GetEastNeighbor();
	BlockIterator GetNorthNeighbor();
	BlockIterator GetSouthNeighbor();
	BlockIterator GetWestNeighbor();
	BlockIterator GetTopNeighbor();
	BlockIterator GetBottomNeighbor();
	BlockIterator GetNeighbor(Direction dir);
};