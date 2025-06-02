#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Engine/Renderer/UAV.hpp"
#include "Engine/Renderer/Renderer.hpp"

UAV::~UAV()
{
	DX_SAFE_RELEASE(m_uav);
}

UAV::UAV(GPUBuffer* gpuBuffer)
	: m_gpuBuffer(gpuBuffer)
{

}
