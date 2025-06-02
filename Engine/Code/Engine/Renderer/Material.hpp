#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"

class Shader;

class Material
{
public:
	Material(std::string const& xmlFileName, Renderer* renderer);
	Material();
	~Material() = default;

	bool Load(std::string const& xmlFileName, Renderer* renderer);
	VertexType ParseVertexType(std::string vertexTypeAsStr);
	std::string m_name;

	Shader* m_shader = nullptr;
	VertexType m_vertexType = VertexType::VERTEX_TYPE_PCUTBN;
	Texture* m_diffuseTexture = nullptr;
	Texture* m_normalTexture = nullptr;
	Texture* m_specGlossEmitTexture = nullptr;
	Rgba8 m_color;
};