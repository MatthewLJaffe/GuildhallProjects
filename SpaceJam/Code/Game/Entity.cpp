#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

Entity::Entity(const Vec3& startPos, EulerAngles const& startOrientation)
{
	Vec3 iBasis;
	Vec3 jBasis;
	Vec3 kBasis;
	startOrientation.GetAsVectors_IFwd_JLeft_KUp(iBasis, jBasis, kBasis);
	m_transform.SetIJKT3D(iBasis, jBasis, kBasis, startPos);
}

Entity::Entity(Vec3 const& startPos, Mat44 const& startOrientation)
{
	m_transform = startOrientation;
	m_transform.SetTranslation3D(startPos);
}

Entity::~Entity()
{
	for (int i = 0; i < m_children.size(); i++)
	{
		delete m_children[i];
		m_children[i] = nullptr;
	}
}

Mat44 Entity::GetWorldTransform() const
{
	Mat44 worldTransform;
	if (m_parent != nullptr)
	{
		worldTransform.Append(m_parent->GetWorldTransform());
	}
	worldTransform.Append(m_transform);
	return worldTransform;
}

Vec3 Entity::GetWorldPosition() const
{
	if (m_parent != nullptr)
	{
		return m_parent->GetWorldTransform().TransformPosition3D(m_transform.GetTranslation3D());
	}
	return m_transform.GetTranslation3D();
}

Vec3 Entity::GetWorldScale() const
{
	return m_transform.GetScale();
}

Vec3 Entity::GetLocalPosition() const
{
	return m_transform.GetTranslation3D();
}

void Entity::SetLocalPosition(Vec3 const& newLocalPos)
{
	m_transform.SetTranslation3D(newLocalPos);
}

Mat44 Entity::GetWorldOrientaiton() const
{
	if (m_parent != nullptr)
	{
		Mat44 parentWorldOrientation = m_parent->GetWorldOrientaiton();
		parentWorldOrientation.Append(GetLocalOrientation());
		return parentWorldOrientation;
	}
	return GetLocalOrientation();
}

Mat44 Entity::GetLocalOrientation() const
{
	return m_transform.GetNormalizedIJKMatrix();
}

void Entity::SetLocalOrientation(EulerAngles const& newLocalOrientation)
{
	Vec3 iBasis;
	Vec3 jBasis;
	Vec3 kBasis;
	newLocalOrientation.GetAsVectors_IFwd_JLeft_KUp(iBasis, jBasis, kBasis);
	m_transform.SetIJK3D(iBasis, jBasis, kBasis);
}

void Entity::SetLocalOrientation(Mat44 const& newLocalOrientation)
{
	m_transform.SetIJK3D(newLocalOrientation.GetIBasis3D(), newLocalOrientation.GetJBasis3D(), newLocalOrientation.GetKBasis3D());
}

void Entity::SetLocalScale(Vec3 const& scale)
{
	m_transform.SetScale(scale);
}

Vec3 Entity::GetForwardNormal()
{
	if (m_parent != nullptr)
	{
		return m_parent->GetWorldTransform().TransformVectorQuantity3D(m_transform.GetIBasis3D());
	}
	return m_transform.GetIBasis3D();
}

void Entity::Translate(Vec3 const& disp)
{
	m_transform.SetTranslation3D(m_transform.GetTranslation3D() + disp);
}

void Entity::AddChild(Entity* child)
{
	m_children.push_back(child);
	child->m_parent = this;
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

