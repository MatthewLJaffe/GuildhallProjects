#include "Game/Model.hpp"
#include "Engine/Renderer/ObjLoader.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Time.hpp"



Model::Model(Vec3 const& position, std::string const& filePath)
	: Entity(position)
	, m_defIndex(ModelDefinition::GetModelDefIndexFromFileName(filePath))
{
}

Model::~Model()
{
	delete m_debugVertexBuffer;
}

void Model::Update(float deltaSeconds)
{
	Translate(m_velocity * deltaSeconds);
	for (int i = 0; i < (int)m_children.size(); i++)
	{
		m_children[i]->Update(deltaSeconds);
	}
}

void Model::Render() const
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	if (GetDef()->m_material != nullptr)
	{
		g_theRenderer->SetModelConstants(GetWorldTransform(), GetDef()->m_material->m_color);
	}
	else
	{
		g_theRenderer->SetModelConstants(GetWorldTransform(), m_color);
	}
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	if (GetDef()->m_material != nullptr)
	{
		g_theRenderer->BindMaterial(GetDef()->m_material);
	}
	else
	{
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BindShader(g_theRenderer->CreateOrGetShaderFromFile("Data/Shaders/Diffuse", VertexType::VERTEX_TYPE_PCUTBN));
	}
	GetDef()->m_gpuMesh->Render();
	for (int i = 0; i < (int)m_children.size(); i++)
	{
		m_children[i]->Render();
	}
}

ModelDefinition const* Model::GetDef() const
{
	if (m_defIndex < 0)
	{
		return nullptr;
	}
	return ModelDefinition::s_modelDefinitions[m_defIndex];
}
