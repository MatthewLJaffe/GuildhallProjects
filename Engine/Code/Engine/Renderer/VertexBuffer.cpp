#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

VertexBuffer::VertexBuffer(size_t size)
	: GPUBuffer(size)
{

}

VertexBuffer::~VertexBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}
