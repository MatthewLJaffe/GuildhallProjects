#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

GPUMesh::GPUMesh()
{
}

GPUMesh::GPUMesh(std::vector<Vertex_PCU> const& unlitVertexes)
{
	m_numIndexes = (int)unlitVertexes.size();

	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU) * unlitVertexes.size());
	g_theRenderer->CopyCPUToGPU(unlitVertexes.data(), sizeof(Vertex_PCU) * unlitVertexes.size(), m_vertexBuffer);
	m_vertType = VertexType::VERTEX_TYPE_PCU;
}

GPUMesh::GPUMesh(const CPUMesh* cpuMesh)
	: m_fileName(cpuMesh->m_meshFileName)
	, m_transform(cpuMesh->m_transform)
{
	Create(cpuMesh);
}

GPUMesh::~GPUMesh()
{
	delete m_indexBuffer;
	delete m_vertexBuffer;
}

void GPUMesh::Create(const CPUMesh* cpuMesh)
{
	m_numIndexes = (int)cpuMesh->m_indexes.size();
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(sizeof(unsigned int) * cpuMesh->m_indexes.size());
	g_theRenderer->CopyCPUToGPU(cpuMesh->m_indexes.data(), sizeof(unsigned int) * cpuMesh->m_indexes.size(), m_indexBuffer);

	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCUTBN) * cpuMesh->m_vertexes.size());
	g_theRenderer->CopyCPUToGPU(cpuMesh->m_vertexes.data(), sizeof(Vertex_PCUTBN) * cpuMesh->m_vertexes.size(), m_vertexBuffer);
}

void GPUMesh::Render() const
{
	if (m_indexBuffer != nullptr)
	{
		g_theRenderer->DrawVertexBufferIndexed(m_vertexBuffer, m_indexBuffer, m_numIndexes, m_vertType);
	}
	else
	{
		g_theRenderer->DrawVertexBuffer(m_vertexBuffer, m_numIndexes, 0, m_vertType);
	}
}

