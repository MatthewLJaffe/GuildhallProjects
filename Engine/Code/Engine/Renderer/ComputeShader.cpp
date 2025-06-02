#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")


#include "Engine/Renderer/ComputeShader.hpp"
#include "Engine/Renderer/Renderer.hpp"

ComputeShader::ComputeShader(const ComputeShaderConfig& config)
	: m_config(config)
{
}

ComputeShader::~ComputeShader()
{
	DX_SAFE_RELEASE(m_computeShader);
}

const std::string ComputeShader::GetName() const
{
	return m_config.m_name;
}


