#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

Entity::Entity(Game* game, const Vec3& startPos)
	: m_game(game)
	, m_position(startPos)
{}

Entity::~Entity()
{
	
}

Mat44 Entity::GetModelMatrix() const
{
	Mat44 m_modelMatrix;
	m_modelMatrix.AppendTranslation3D(m_position);
	m_modelMatrix.Append(m_orientationDegrees.GetAsMatrix_IFwd_JLeft_KUp());
	m_modelMatrix.AppendScaleNonUniform3D(m_scale);
	return m_modelMatrix;
}

Vec3 Entity::GetForwardNormal()
{
	return m_orientationDegrees.GetIFwd();
}

void Entity::CreateDebugTangentAndBasisVectors()
{
	for (int i = 0; i < (int)m_vertexes.size(); i++)
	{
		AddVertsForBoxLine3D(m_debugVertexes, m_vertexes[i].m_position, m_vertexes[i].m_position + m_vertexes[i].m_tangent * .1f, .0005f, Rgba8::RED);
		AddVertsForBoxLine3D(m_debugVertexes, m_vertexes[i].m_position, m_vertexes[i].m_position + m_vertexes[i].m_bitangent * .1f, .0005f, Rgba8::GREEN);
		AddVertsForBoxLine3D(m_debugVertexes, m_vertexes[i].m_position, m_vertexes[i].m_position + m_vertexes[i].m_normal * .1f, .0005f, Rgba8::BLUE);
	}

	if (m_debugVertexBuffer != nullptr)
	{
		delete m_debugVertexBuffer;
		m_debugVertexBuffer = nullptr;
	}
	m_debugVertexBuffer = g_theRenderer->CreateVertexBuffer(m_debugVertexes.size() * sizeof(Vertex_PCU));
	g_theRenderer->CopyCPUToGPU((void*)m_debugVertexes.data(), m_debugVertexes.size() * sizeof(Vertex_PCU), m_debugVertexBuffer);
}

