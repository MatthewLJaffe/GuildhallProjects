#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Engine/Renderer/IndirectArgsBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

IndirectArgsBuffer::IndirectArgsBuffer(size_t size)
	: GPUBuffer(size)
{
}

IndirectArgsBuffer::~IndirectArgsBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}
