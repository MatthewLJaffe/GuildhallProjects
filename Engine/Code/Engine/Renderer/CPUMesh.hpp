#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"

class CPUMesh
{
public:
	CPUMesh();
	CPUMesh(Renderer* renderer, const std::string& objFilename, const Mat44& transform);
	virtual ~CPUMesh();

	void Load(Renderer* renderer, const std::string& objFilename, const Mat44& transform);
	std::string m_meshFileName;
	Mat44 m_transform;
	std::vector<unsigned int> m_indexes;
	std::vector<Vertex_PCUTBN> m_vertexes;
};