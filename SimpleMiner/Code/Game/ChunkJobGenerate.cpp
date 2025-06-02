#include "Game/ChunkJobGenerate.hpp"
#include "Game/World.hpp"

ChunkJobGenerate::ChunkJobGenerate(Chunk* myChunk)
	: ChunkJob(myChunk)
{
	m_jobTypeBit = JOB_GENERATE_CHUNK;
}

void ChunkJobGenerate::Execute()
{
	m_chunk->m_chunkState = ChunkState::ACTIVATING_GENERATING;
	m_chunk->GenerateBlockData();
	m_chunk->GenerateCaves();
	m_chunk->SpawnBlockTemplates();
}
