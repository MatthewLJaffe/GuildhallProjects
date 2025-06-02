#include "Game/ChunkJobWriteToDisk.hpp"
#include "Game/World.hpp"

ChunkJobWriteToDisk::ChunkJobWriteToDisk(Chunk* myChunk)
	: ChunkJob(myChunk)
{
	m_jobTypeBit = JOB_WRITE_CHUNK_TO_DISK;
}

void ChunkJobWriteToDisk::Execute()
{
	m_chunk->SaveChunkToDisk();
}
