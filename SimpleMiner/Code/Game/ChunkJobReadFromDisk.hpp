#pragma once
#include "Game/ChunkJob.hpp"

class ChunkJobReadFromDisk : public ChunkJob
{
public:
	ChunkJobReadFromDisk(Chunk* chunk);
	void Execute() override;
};