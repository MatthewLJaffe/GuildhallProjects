#pragma once

struct ID3D11UnorderedAccessView;
class GPUBuffer;

class UAV
{
	friend class Renderer;

public:
	virtual ~UAV();
	UAV(const UAV& copy) = delete;
	GPUBuffer* m_gpuBuffer = nullptr;
private:
	UAV(GPUBuffer* gpuBuffer);
	ID3D11UnorderedAccessView* m_uav = nullptr;	

};