#include "Game/ChunkJobReadFromDisk.hpp"
#include "Game/World.hpp"
#include "Game/Chunk.hpp"



ChunkJobReadFromDisk::ChunkJobReadFromDisk(Chunk* chunk)
	: ChunkJob(chunk)
{
	m_jobTypeBit = JOB_READ_CHUNK_FROM_DISK;
}

void ChunkJobReadFromDisk::Execute()
{
	m_chunk->m_chunkState = ChunkState::ACTIVATING_LOADING;
	if (!m_chunk->RetrieveBlockDataFromDisk())
	{
		ERROR_AND_DIE("IO JOB CANNOT FIND CHUNK DATA ON DISK");
	}
}

