#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

ConstantBuffer::ConstantBuffer(size_t size)
	: GPUBuffer(size)
{
}

ConstantBuffer::~ConstantBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}
