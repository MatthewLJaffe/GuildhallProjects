#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

StructuredBuffer::StructuredBuffer(size_t size)
	: GPUBuffer(size)
{
}

StructuredBuffer::~StructuredBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}
