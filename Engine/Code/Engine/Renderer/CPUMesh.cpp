#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/ObjLoader.hpp"

CPUMesh::CPUMesh()
{
}

CPUMesh::~CPUMesh()
{
}

CPUMesh::CPUMesh(Renderer* renderer, const std::string& objFilename, const Mat44& transform)
	: m_meshFileName(objFilename)
	, m_transform(transform)
{
	Load(renderer, objFilename, transform);
}

void CPUMesh::Load(Renderer* renderer, const std::string& objFilename, const Mat44& transform)
{
	bool hasNormals = false;
	bool hasUVs = false;
	m_vertexes.clear();
	m_indexes.clear();
	ObjLoader::Load(renderer, objFilename, m_vertexes, m_indexes, hasNormals, hasUVs, transform);
}
