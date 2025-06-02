#include "Game/BlockIterator.hpp"
#include "Game/Chunk.hpp"
#include "Game/Block.hpp"

BlockIterator::BlockIterator(Chunk* chunk, int blockIndex)
	: m_chunk(chunk)
	, m_blockIndex(blockIndex)
{
}

Block* BlockIterator::GetBlock()
{
	if (m_chunk == nullptr)
	{
		return nullptr;
	}

	return &m_chunk->m_blocks[m_blockIndex];
}

Vec3 BlockIterator::GetWorldCenter()
{
	if (m_chunk == nullptr)
	{
		return Vec3::ZERO;
	}

	IntVec3 localChunkCoords = GetCoordsFromChunkIndex(m_blockIndex);
	Vec3 localBlockPos = Vec3((float)localChunkCoords.x + .5f, (float)localChunkCoords.y + .5f, (float)localChunkCoords.z + .5f);
	Vec3 worldBlockPos = localBlockPos +
		Vec3((float)(m_chunk->m_chunkCoords.x * XSIZE), (float)(m_chunk->m_chunkCoords.y * YSIZE), 0.f);
	return worldBlockPos;
}

bool BlockIterator::IsValid()
{
	return m_chunk != nullptr && m_blockIndex >= 0 && m_blockIndex < CHUNK_SIZE;
}

BlockIterator BlockIterator::GetEastNeighbor()
{
	if (m_chunk == nullptr)
	{
		return BlockIterator(nullptr, 0);
	}
	int eastIndex = m_blockIndex;
	int xPos = (m_blockIndex & XMASK);
	if (xPos == XSIZE - 1)
	{
		eastIndex &= ~XMASK;
		return BlockIterator(m_chunk->m_eastNeighbor, eastIndex);
	}

	eastIndex++;
	return BlockIterator(m_chunk, eastIndex);
}

BlockIterator BlockIterator::GetNorthNeighbor()
{
	if (m_chunk == nullptr)
	{
		return BlockIterator(nullptr, 0);
	}
	int northIndex = m_blockIndex;
	int yPos = (m_blockIndex & YMASK) >> XBITS;
	if (yPos == YSIZE - 1)
	{
		northIndex &= ~YMASK;
		return BlockIterator(m_chunk->m_northNeighbor, northIndex);
	}

	northIndex += XSIZE;
	return BlockIterator(m_chunk, northIndex);
}

BlockIterator BlockIterator::GetSouthNeighbor()
{
	if (m_chunk == nullptr)
	{
		return BlockIterator(nullptr, 0);
	}
	int southIndex = m_blockIndex;
	int yPos = (m_blockIndex & YMASK) >> XBITS;
	if (yPos == 0)
	{
		southIndex |= YMASK;
		return BlockIterator(m_chunk->m_southNeighbor, southIndex);
	}

	southIndex -= XSIZE;
	return BlockIterator(m_chunk, southIndex);
}

BlockIterator BlockIterator::GetWestNeighbor()
{
	if (m_chunk == nullptr)
	{
		return BlockIterator(nullptr, 0);
	}
	int westIndex = m_blockIndex;
	int xPos = (m_blockIndex & XMASK);
	if (xPos == 0)
	{
		westIndex |= XMASK;
		return BlockIterator(m_chunk->m_westNeighbor, westIndex);
	}

	westIndex--;
	return BlockIterator(m_chunk, westIndex);
}

BlockIterator BlockIterator::GetTopNeighbor()
{
	if (m_chunk == nullptr)
	{
		return BlockIterator(nullptr, 0);
	}
	int topIndex = m_blockIndex;
	int zPos = (m_blockIndex & ZMASK) >> (XBITS + YBITS);
	if (zPos == ZSIZE - 1)
	{
		return BlockIterator(nullptr, -1);
	}

	topIndex += (XSIZE * YSIZE);
	return BlockIterator(m_chunk, topIndex);
}

BlockIterator BlockIterator::GetBottomNeighbor()
{
	if (m_chunk == nullptr)
	{
		return BlockIterator(nullptr, 0);
	}
	int bottomIndex = m_blockIndex;
	int zPos = (m_blockIndex & ZMASK) >> (XBITS + YBITS);
	if (zPos == 0)
	{
		return BlockIterator(nullptr, -1);
	}

	bottomIndex -= (XSIZE * YSIZE);
	return BlockIterator(m_chunk, bottomIndex);
}

BlockIterator BlockIterator::GetNeighbor(Direction dir)
{
	switch (dir)
	{
	case Direction::EAST: return GetEastNeighbor();
	case Direction::WEST: return GetWestNeighbor();
	case Direction::NORTH: return GetNorthNeighbor();
	case Direction::SOUTH: return GetSouthNeighbor();
	case Direction::TOP: return GetTopNeighbor();
	case Direction::BOTTOM: return GetBottomNeighbor();
	case Direction::COUNT: return BlockIterator();
	default: return BlockIterator();
	}
}
