#pragma once
#include "Engine/Core/EngineCommon.hpp"

struct ID3D11ComputeShader;

struct ComputeShaderConfig
{
	std::string m_name;
	std::string m_computeEntryPoint = "CSMain";
};

class ComputeShader
{
	friend class Renderer;

public:
	ComputeShader(const ComputeShaderConfig& config);
	ComputeShader(const ComputeShader& copy) = delete;
	~ComputeShader();

	const std::string GetName() const;

	ComputeShaderConfig m_config;
	ID3D11ComputeShader* m_computeShader = nullptr;

};