#include "Game/Model.hpp"
#include "Engine/Renderer/ObjLoader.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Time.hpp"

Model::Model(std::string const& xmlFilePath)
{
	Load(xmlFilePath);
}

Model::~Model()
{
	delete m_gpuMesh;
	delete m_cpuMesh;
	delete m_material;
}

bool Model::Load(std::string const& fileName)
{
	if (m_cpuMesh != nullptr)
	{
		delete m_cpuMesh;
		m_cpuMesh = nullptr;
	}
	if (m_gpuMesh != nullptr)
	{
		delete m_gpuMesh;
		m_gpuMesh = nullptr;
	}
	if (m_material != nullptr)
	{
		delete m_material;
		m_material = nullptr;
	}
	Strings fileNameSplit = SplitStringOnDelimiter(fileName, '.');
	DebuggerPrintf("-----------------------------------------------------\n");
	DebuggerPrintf("Loaded file %s\n", fileName.c_str());
	if (fileNameSplit[1] == "obj")
	{
		LoadObj(fileName);
	}
	else if (fileNameSplit[1] == "xml")
	{
		LoadXml(fileName);
	}

	DebuggerPrintf("-----------------------------------------------------\n");
	return true;
}

void Model::Render() const
{
	g_theRenderer->BindMaterial(m_material);
	g_theRenderer->BindShader(m_shader);
	m_gpuMesh->Render();
}

bool Model::LoadXml(std::string const& fileName)
{
	XmlDocument xmlDocument;
	GUARANTEE_OR_DIE(xmlDocument.LoadFile(fileName.c_str()) == 0, std::string("Failed to load ") + fileName);
	XmlElement* rootElement = xmlDocument.FirstChildElement("Model");
	GUARANTEE_OR_DIE(rootElement != nullptr, std::string("Could not get the Model from ") + fileName);
	m_name = ParseXmlAttribute(*rootElement, "name", "Missing");
	m_objFilename = ParseXmlAttribute(*rootElement, "path", "Missing");
	std::string materialFilePath = ParseXmlAttribute(*rootElement, "material", "Missing");
	if (materialFilePath == "Missing")
	{
		m_material = nullptr;
	}
	else
	{
		m_material = new Material(materialFilePath, g_theRenderer);
	}
	std::string shaderFilePath = ParseXmlAttribute(*rootElement, "shader", "Missing");
	if (shaderFilePath == "Missing")
	{
		m_shader = nullptr;
	}
	else
	{
		m_shader = g_theRenderer->CreateOrGetShaderFromFile(shaderFilePath.c_str(), VertexType::VERTEX_TYPE_PCUTBN);
	}

	XmlElement* transformElement = rootElement->FirstChildElement("Transform");
	Vec3 x = ParseXmlAttribute(*transformElement, "x", Vec3::ZERO);
	Vec3 y = ParseXmlAttribute(*transformElement, "y", Vec3::ZERO);
	Vec3 z = ParseXmlAttribute(*transformElement, "z", Vec3::ZERO);
	Vec3 t = ParseXmlAttribute(*transformElement, "t", Vec3::ZERO);
	float scale = ParseXmlAttribute(*transformElement, "scale", 1.f);

	m_modelTransform.SetIJKT3D(x, y, z, t);
	m_modelTransform.AppendScaleUniform3D(scale);
	LoadObj(m_objFilename, m_modelTransform);
	return true;
}

bool Model::LoadObj(std::string const& path, Mat44 const& transform)
{
	m_cpuMesh = new CPUMesh(g_theRenderer, path, transform);
	m_gpuMesh = new GPUMesh(m_cpuMesh);

	return true;
}
