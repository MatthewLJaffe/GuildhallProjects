#include "Game/Model.hpp"
#include "Engine/Renderer/ObjLoader.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Time.hpp"

Model::Model(Game* game, Vec3 const& position)
	: Entity(game, position)
{
}

Model::~Model()
{
	delete m_gpuMesh;
	delete m_cpuMesh;
	delete m_debugVertexBuffer;
	delete m_material;
}

bool Model::Load(std::string const& fileName)
{
	if (m_cpuMesh != nullptr)
	{
		delete m_cpuMesh;
		m_cpuMesh = nullptr;
	}
	if (m_debugVertexBuffer != nullptr)
	{
		delete m_debugVertexBuffer;
		m_debugVertexBuffer = nullptr;
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

void Model::Update(float deltaSeconds)
{
	m_position += m_velocity * deltaSeconds;
	if (m_game->m_debugRotation)
	{
		m_orientationDegrees.m_yaw += m_angularVelocity.m_yaw * deltaSeconds;
		m_orientationDegrees.m_pitch += m_angularVelocity.m_pitch * deltaSeconds;
		m_orientationDegrees.m_roll += m_angularVelocity.m_roll * deltaSeconds;
	}


}

void Model::Render() const
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	if (m_material != nullptr)
	{
		g_theRenderer->SetModelConstants(GetModelMatrix(), m_material->m_color);
	}
	else
	{
		g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	}
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	if (m_material != nullptr)
	{
		g_theRenderer->BindMaterial(m_material);
	}
	else
	{
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BindShader(g_theRenderer->CreateOrGetShaderFromFile("Data/Shaders/Diffuse", VertexType::VERTEX_TYPE_PCUTBN));
	}
	m_gpuMesh->Render();
	if (m_renderDebug)
	{
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->DrawVertexBuffer(m_debugVertexBuffer, (int)m_debugVertexes.size());
	}
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
	XmlElement* transformElement = rootElement->FirstChildElement("Transform");
	Vec3 x = ParseXmlAttribute(*transformElement, "x", Vec3::ZERO);
	Vec3 y = ParseXmlAttribute(*transformElement, "y", Vec3::ZERO);
	Vec3 z = ParseXmlAttribute(*transformElement, "z", Vec3::ZERO);
	Vec3 t = ParseXmlAttribute(*transformElement, "t", Vec3::ZERO);
	EulerAngles rot = ParseXmlAttribute(*transformElement, "rot", EulerAngles());
	float scale = ParseXmlAttribute(*transformElement, "scale", 1.f);
	m_modelTransform.SetIJKT3D(x, y, z, t);
	m_modelTransform.Append(rot.GetAsMatrix_IFwd_JLeft_KUp());
	m_modelTransform.AppendScaleUniform3D(scale);
	LoadObj(m_objFilename, m_modelTransform);
	return true;
}

bool Model::LoadObj(std::string const& path, Mat44 const& transform)
{
	double timeBeforeCPUMesh = GetCurrentTimeSeconds();
	m_cpuMesh = new CPUMesh(g_theRenderer, path, transform);
	DebuggerPrintf("Created CPU mesh time: %f sec\n", GetCurrentTimeSeconds() - timeBeforeCPUMesh);
	m_indexes = m_cpuMesh->m_indexes;
	m_vertexes = m_cpuMesh->m_vertexes;

	double timeBeforeGPUMesh = GetCurrentTimeSeconds();
	m_gpuMesh = new GPUMesh(m_cpuMesh);
	DebuggerPrintf("Created GPU mesh time: %f sec\n", GetCurrentTimeSeconds() - timeBeforeGPUMesh);

	double timeBeforeDebug = GetCurrentTimeSeconds();
	CreateDebugTangentAndBasisVectors();
	DebuggerPrintf("Created debug normals time: %f sec\n", GetCurrentTimeSeconds() - timeBeforeDebug);

	return true;
}
