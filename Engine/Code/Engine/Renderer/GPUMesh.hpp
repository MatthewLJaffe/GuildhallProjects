#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"

class CPUMesh;
class IndexBuffer;
class VertexBuffer;

class GPUMesh
{
public:
	GPUMesh();
	GPUMesh(std::vector<Vertex_PCU> const& unlitVertexes);
	GPUMesh(const CPUMesh* cpuMesh);
	virtual ~GPUMesh();
	void Create(const CPUMesh* cpuMesh);
	void Render() const;
	std::string m_fileName;
	Mat44 m_transform;
protected:
	IndexBuffer* m_indexBuffer = nullptr;
	VertexBuffer* m_vertexBuffer = nullptr;
	int m_numIndexes = 0;
	VertexType m_vertType = VertexType::VERTEX_TYPE_PCUTBN;
};