#pragma once
#include "Engine/Renderer/GPUBuffer.hpp"

struct ID3D11Buffer;

class StructuredBuffer : public GPUBuffer
{
	friend class Renderer;

private:
	StructuredBuffer(size_t size);

public:
	StructuredBuffer(const StructuredBuffer& copy) = delete;
	virtual ~StructuredBuffer() override;
};