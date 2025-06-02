#pragma once
#include "Engine/Renderer/GPUBuffer.hpp"
struct ID3D11Buffer;

class IndexBuffer : GPUBuffer
{
	friend class Renderer;

public:
	IndexBuffer(const IndexBuffer& copy) = delete;
	virtual ~IndexBuffer() override;

private:
	IndexBuffer(size_t size);
};