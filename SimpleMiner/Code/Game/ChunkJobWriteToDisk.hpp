#pragma once
#include "Game/ChunkJob.hpp"

class ChunkJobWriteToDisk : public ChunkJob
{
public:
	ChunkJobWriteToDisk(Chunk* myChunk);
	void Execute() override;
};