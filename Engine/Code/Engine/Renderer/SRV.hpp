#pragma once

struct ID3D11ShaderResourceView;
class GPUBuffer;

class SRV
{
	friend class Renderer;

public:
	virtual ~SRV();
	SRV(const SRV& copy) = delete;

private:
	SRV(GPUBuffer* gpuBuffer);
	ID3D11ShaderResourceView* m_srv = nullptr;
	GPUBuffer* m_gpuBuffer = nullptr;
};