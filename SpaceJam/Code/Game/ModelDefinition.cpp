#include "Game/ModelDefinition.hpp"
#include "Engine/Renderer/CPUMesh.hpp"

std::vector<ModelDefinition*> ModelDefinition::s_modelDefinitions;

ModelDefinition const* ModelDefinition::CreateOrGetModelDefinitionFromFile(std::string const& fileName)
{
	for (int i = 0; i < (int)s_modelDefinitions.size(); i++)
	{
		if (s_modelDefinitions[i]->m_fileName == fileName)
		{
			return s_modelDefinitions[i];
		}
	}
	return CreateModelDefinitionFromFile(fileName);
}

void ModelDefinition::RemoveModelDefinitions()
{
	for (int i = 0; i < (int)s_modelDefinitions.size(); i++)
	{
		delete s_modelDefinitions[i];
		s_modelDefinitions[i] = nullptr;
	}
}

int ModelDefinition::GetModelDefIndexFromFileName(std::string const& fileName)
{
	for (int i = 0; i < (int)s_modelDefinitions.size(); i++)
	{
		if (s_modelDefinitions[i]->m_fileName == fileName)
		{
			return i;
		}
	}

	CreateModelDefinitionFromFile(fileName);
	return (int)s_modelDefinitions.size() - 1;
}



ModelDefinition const* ModelDefinition::CreateModelDefinitionFromFile(std::string const& fileName)
{
	ModelDefinition* defToLoad = new ModelDefinition();
	XmlDocument xmlDocument;
	GUARANTEE_OR_DIE(xmlDocument.LoadFile(fileName.c_str()) == 0, std::string("Failed to load ") + fileName);
	XmlElement* rootElement = xmlDocument.FirstChildElement("Model");
	GUARANTEE_OR_DIE(rootElement != nullptr, std::string("Could not get the Model from ") + fileName);
	defToLoad->m_name = ParseXmlAttribute(*rootElement, "name", "Missing");
	defToLoad->m_objFilename = ParseXmlAttribute(*rootElement, "path", "Missing");
	defToLoad->m_fileName = fileName;
	std::string materialFilePath = ParseXmlAttribute(*rootElement, "material", "Missing");
	if (materialFilePath == "Missing")
	{
		defToLoad->m_material = nullptr;
	}
	else
	{
		defToLoad->m_material = new Material(materialFilePath, g_theRenderer);
	}
	XmlElement* transformElement = rootElement->FirstChildElement("Transform");
	Vec3 x = ParseXmlAttribute(*transformElement, "x", Vec3::ZERO);
	Vec3 y = ParseXmlAttribute(*transformElement, "y", Vec3::ZERO);
	Vec3 z = ParseXmlAttribute(*transformElement, "z", Vec3::ZERO);
	Vec3 t = ParseXmlAttribute(*transformElement, "t", Vec3::ZERO);
	EulerAngles rot = ParseXmlAttribute(*transformElement, "rot", EulerAngles());
	float scale = ParseXmlAttribute(*transformElement, "scale", 1.f);
	defToLoad->m_modelTransform.SetIJKT3D(x, y, z, t);
	defToLoad->m_modelTransform.Append(rot.GetAsMatrix_IFwd_JLeft_KUp());
	defToLoad->m_modelTransform.AppendScaleUniform3D(scale);
	defToLoad->LoadObj(defToLoad->m_objFilename, defToLoad->m_modelTransform);
	s_modelDefinitions.push_back(defToLoad);
	return defToLoad;
}



ModelDefinition::~ModelDefinition()
{
	delete m_material;
}

bool ModelDefinition::LoadObj(std::string const& path, Mat44 const& transform)
{
	m_cpuMesh = g_theRenderer->CreateOrGetCPUMeshFromFile(path, transform);
	m_gpuMesh = g_theRenderer->CreateOrGetGPUMeshFromFile(path, transform);

	return true;
}
