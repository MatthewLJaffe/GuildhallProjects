#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Engine/Renderer/GPUBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

GPUBuffer::GPUBuffer(size_t size)
	: m_size(size)
{
}

GPUBuffer::~GPUBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}
