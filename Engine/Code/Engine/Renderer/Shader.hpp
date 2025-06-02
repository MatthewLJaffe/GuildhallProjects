#pragma once
#include "Engine/Core/EngineCommon.hpp"

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;

struct ShaderConfig
{
	std::string m_name;
	std::string m_vertexEntrypoint = "VertexMain";
	std::string m_pixelEntryPoint = "PixelMain";
};

class Shader
{
	friend class Renderer;

public:
	Shader(const Shader& copy) = delete;
	~Shader();
	const std::string GetName() const;

private:
	Shader(const ShaderConfig& config);
	ShaderConfig m_config;
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_inputLayout = nullptr;
};