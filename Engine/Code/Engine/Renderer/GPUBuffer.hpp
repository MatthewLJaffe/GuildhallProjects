#pragma once

struct ID3D11Buffer;

class GPUBuffer
{
	friend class Renderer;
	friend class UAV;
	friend class SRV;

protected:
	GPUBuffer(size_t size);
	ID3D11Buffer* m_buffer = nullptr;
	size_t m_size = 0;

public:
	GPUBuffer(const GPUBuffer& copy) = delete;
	virtual ~GPUBuffer();
};