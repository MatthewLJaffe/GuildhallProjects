#include "Engine/Renderer/Texture.hpp"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include "Engine/Renderer/Renderer.hpp"


Texture::~Texture()
{
	DX_SAFE_RELEASE(m_texture);
	DX_SAFE_RELEASE(m_shaderResourceView);
	DX_SAFE_RELEASE(m_renderTargetView);
}
