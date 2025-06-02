#pragma once
#include "Game/ChunkJob.hpp"
#include "Engine/Core/JobSystem.hpp"

class ChunkJobGenerate : public ChunkJob
{
public:
	ChunkJobGenerate(Chunk* myChunk);
	void Execute() override;
};