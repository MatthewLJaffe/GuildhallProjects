#pragma once
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"

struct ModelDefinition
{
	~ModelDefinition();
	std::string m_fileName = "";
	std::string m_name = "";
	std::string m_objFilename = "";
	CPUMesh* m_cpuMesh = nullptr;
	GPUMesh* m_gpuMesh = nullptr;
	Material* m_material = nullptr;
	Mat44 m_modelTransform;
	bool LoadObj(std::string const&, Mat44 const& transform = Mat44());
	static std::vector<ModelDefinition*> s_modelDefinitions;
	static void RemoveModelDefinitions();
	static int GetModelDefIndexFromFileName(std::string const& fileName);
	static ModelDefinition const* CreateModelDefinitionFromFile(std::string const& fileName);
	static ModelDefinition const* CreateOrGetModelDefinitionFromFile(std::string const& fileName);
};