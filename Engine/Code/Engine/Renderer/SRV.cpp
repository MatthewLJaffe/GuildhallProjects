#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Engine/Renderer/SRV.hpp"
#include "Engine/Renderer/Renderer.hpp"

SRV::~SRV()
{
	DX_SAFE_RELEASE(m_srv);
}

SRV::SRV(GPUBuffer* gpuBuffer)
	: m_gpuBuffer(gpuBuffer)
{
}
