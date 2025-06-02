#pragma once
#include "Game/Chunk.hpp"
#include "Engine/Core/JobSystem.hpp"

class ChunkJob : public Job
{
public:
	ChunkJob(Chunk* chunk);
	Chunk* m_chunk = nullptr;
};