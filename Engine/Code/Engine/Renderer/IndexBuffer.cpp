#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

IndexBuffer::IndexBuffer(size_t size)
	: GPUBuffer(size)
{
}

IndexBuffer::~IndexBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}
