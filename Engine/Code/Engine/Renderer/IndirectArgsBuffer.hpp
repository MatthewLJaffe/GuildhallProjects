#pragma once
#include "Engine/Renderer/GPUBuffer.hpp"
struct ID3D11Buffer;

class IndirectArgsBuffer : public GPUBuffer
{
	friend class Renderer;

public:
	IndirectArgsBuffer(const IndirectArgsBuffer& copy) = delete;
	virtual ~IndirectArgsBuffer() override;

private:
	IndirectArgsBuffer(size_t size);
};