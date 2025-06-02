#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/XmlUtils.hpp"

Material::Material(std::string const& xmlFileName, Renderer* renderer)
	: m_name(xmlFileName)
{
	if (!Load(xmlFileName, renderer))
	{
		ERROR_RECOVERABLE(Stringf("Could not find material %s", xmlFileName.c_str()));
	}
}

Material::Material()
{
}

bool Material::Load(std::string const& xmlFileName, Renderer* renderer)
{
	XmlDocument xmlDocument;
	GUARANTEE_OR_DIE(xmlDocument.LoadFile(xmlFileName.c_str()) == 0, std::string("Failed to load ") + xmlFileName);
	XmlElement* rootElement = xmlDocument.FirstChildElement("Material");
	GUARANTEE_OR_DIE(rootElement != nullptr, std::string("Could not get the Material from ") + xmlFileName);
	m_name = ParseXmlAttribute(*rootElement, "name", "Missing");
	m_vertexType = ParseVertexType(ParseXmlAttribute(*rootElement, "vertexType", "Missing"));
	m_shader = renderer->CreateOrGetShaderFromFile( ParseXmlAttribute(*rootElement, "shader", "Missing").c_str(), m_vertexType);
	std::string diffuseTexturePath = ParseXmlAttribute(*rootElement, "diffuseTexture", "Missing");
	if (diffuseTexturePath != "Missing")
	{
		m_diffuseTexture = renderer->CreateOrGetTextureFromFile(diffuseTexturePath.c_str());
	}
	std::string normalTexturePath = ParseXmlAttribute(*rootElement, "normalTexture", "Missing");
	if (normalTexturePath != "Missing")
	{
		m_normalTexture = renderer->CreateOrGetTextureFromFile(normalTexturePath.c_str());
	}
	std::string specGlossEmitTexturePath = ParseXmlAttribute(*rootElement, "specGlossEmitTexture", "Missing");
	if (specGlossEmitTexturePath != "Missing")
	{
		m_specGlossEmitTexture = renderer->CreateOrGetTextureFromFile(specGlossEmitTexturePath.c_str());
	}
	m_color = ParseXmlAttribute(*rootElement, "color", Rgba8::WHITE);
	return true;
}

VertexType Material::ParseVertexType(std::string vertexTypeAsStr)
{
	if (vertexTypeAsStr == "Vertex_PCUTBN")
	{
		return VertexType::VERTEX_TYPE_PCUTBN;
	}
	return VertexType::VERTEX_TYPE_PCU;
}
