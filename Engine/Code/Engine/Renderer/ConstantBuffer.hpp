#pragma once
#include "Engine/Renderer/GPUBuffer.hpp"

struct ID3D11Buffer;

class ConstantBuffer : public GPUBuffer
{
	friend class Renderer;

private:
	ConstantBuffer(size_t size);

public:
	ConstantBuffer(const ConstantBuffer& copy) = delete;
	virtual ~ConstantBuffer() override;
};