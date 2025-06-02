#include "Engine/ParticleSystem/ParticleEffect.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/VertexUtils.hpp"

ParticleEffect::ParticleEffect(ParticleEffectDefinition const& effectDef, int definitionIndex)
	: m_definitionIndex(definitionIndex)
{
	m_name = effectDef.m_name;
	m_uniformScale = effectDef.m_scale;
	for (int i = 0; i < (int)effectDef.m_particleEmitterDefs.size(); i++)
	{
		ParticleEmitter* currEmitter = new ParticleEmitter(effectDef.m_particleEmitterDefs[i], this);
		m_emitters.push_back(currEmitter);
	}

	std::vector<Vertex_PCU> debugVerts;
	AddVertsForBasis3D(debugVerts, Mat44());
	m_debugVerts = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU) * debugVerts.size());
	m_numDebugVerts = (int)debugVerts.size();
	g_theRenderer->CopyCPUToGPU(debugVerts.data(), debugVerts.size() * sizeof(Vertex_PCU), m_debugVerts);
}

Mat44 ParticleEffect::GetTransformMatrix() const
{
	if (m_useWorldTransform)
	{
		return m_worldTransform;
	}
	Mat44 effectBasis;
	effectBasis.AppendTranslation3D(m_position);
	effectBasis.Append(m_orientationDegrees.GetAsMatrix_IFwd_JLeft_KUp());
	return effectBasis;
}

ParticleEffectDefinition* ParticleEffect::GetEffectDefinition()
{
	ParticleEffectDefinition* effectDef = &g_theParticleSystem->m_loadedEffectDefinitions[m_definitionIndex];
	return effectDef;
}

void ParticleEffect::AddEmitter(ParticleEmitterInitializer& initializer)
{
	ParticleEmitter* currEmitter = new ParticleEmitter(initializer, this);
	GetEffectDefinition()->m_particleEmitterDefs.push_back(initializer);
	m_emitters.push_back(currEmitter);
}

Vec3 ParticleEffect::GetPosition()
{
	if (m_useWorldTransform)
	{
		return m_worldTransform.GetTranslation3D();
	}
	return m_position;
}

void ParticleEffect::SetWorldTransform(Mat44 const& transform)
{
	if (m_useWorldTransform)
	{
		m_worldTransform = transform;
	}
	m_position = m_worldTransform.GetTranslation3D();
	m_orientationDegrees = m_worldTransform.GetEulerAngles();
	for (int i = 0; i < m_emitters.size(); i++)
	{
		m_emitters[i]->UpdateTransform();
	}
}

void ParticleEffect::SetScale(float uniformScale)
{
	if (m_useWorldTransform)
	{
		m_worldTransform.SetScale(Vec3(uniformScale, uniformScale, uniformScale));
	}
	m_uniformScale = uniformScale;
	for (int i = 0; i < m_emitters.size(); i++)
	{
		m_emitters[i]->UpdateTransform();
	}
}

float ParticleEffect::GetScale() const
{
	return m_uniformScale;
}

EulerAngles ParticleEffect::GetOrientationDegrees()
{
	if (m_useWorldTransform)
	{
		ERROR_RECOVERABLE("Asking to convert from matrix to euler");
		return m_worldTransform.GetEulerAngles();
	}
	return m_orientationDegrees;
}

void ParticleEffect::DebugDrawParticleEffect()
{
	
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants(GetTransformMatrix());
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexBuffer(m_debugVerts, m_numDebugVerts);
	for (int i = 0; i < (int)m_emitters.size(); i++)
	{
		m_emitters[i]->DebugDrawEmitter();
	}
}

void ParticleEffect::SetPosition(Vec3 const& position)
{
	if (m_useWorldTransform)
	{
		m_worldTransform.SetTranslation3D(position);
	}
	m_position = position;
	for (int i = 0; i < m_emitters.size(); i++)
	{
		m_emitters[i]->UpdateTransform();
	}
}

void ParticleEffect::SetOrientationDegrees(EulerAngles const& orientationDegrees)
{
	if (m_useWorldTransform)
	{
		Vec3 iBasis;
		Vec3 jBasis;
		Vec3 kBasis;
		orientationDegrees.GetAsVectors_IFwd_JLeft_KUp(iBasis, jBasis, kBasis);
		m_worldTransform.SetIJK3D(iBasis, jBasis, kBasis);
	}
	m_orientationDegrees = orientationDegrees;
	for (int i = 0; i < m_emitters.size(); i++)
	{
		m_emitters[i]->UpdateTransform();
	}
}

void ParticleEffect::Stop()
{
	for (int i = 0; i < m_emitters.size(); i++)
	{
		m_emitters[i]->Stop();
	}
}

void ParticleEffect::KillParticles()
{
	for (int i = 0; i < m_emitters.size(); i++)
	{
		m_emitters[i]->KillParticles();
	}
}

void ParticleEffect::Play(float duration)
{
	for (int i = 0; i < m_emitters.size(); i++)
	{
		m_emitters[i]->Play(duration);
	}
}

bool ParticleEffect::IsActive()
{
	for (int i = 0; i < (int)m_emitters.size(); i++)
	{
		EmitterInstanceCPU* emitterInstance = m_emitters[i]->GetEmitterInstanceCPU();
		if (emitterInstance->m_isActive == 1)
		{
			return true;
		}
	}
	return false;
}


ParticleEffect::~ParticleEffect()
{
	while (m_emitters.size() > 0)
	{
		delete m_emitters[0];
	}
	delete m_debugVerts;
}

EmitterUpdateDefinitionGPU* ParticleEffectDefinition::GetUpdateDefByName(std::string name)
{
	for (int i = 0; i < (int)m_particleEmitterDefs.size(); i++)
	{
		if (m_particleEmitterDefs[i].GetParticleEmitterDefinition().m_name == name)
		{
			ParticleEmitterDefinition& definition = m_particleEmitterDefs[i].GetParticleEmitterDefinition();
			return &g_theParticleSystem->m_updateDefinitions[definition.m_loadedDefinitionIndex];
		}
	}
	return nullptr;
}

EmitterUpdateDefinitionGPU* ParticleEffectDefinition::GetUpdateDefByIndex(int index)
{
	if (index >= m_particleEmitterDefs.size())
	{
		return nullptr;
	}
	ParticleEmitterDefinition& definition = m_particleEmitterDefs[index].GetParticleEmitterDefinition();
	return &g_theParticleSystem->m_updateDefinitions[definition.m_loadedDefinitionIndex];
}

EmitterRenderDefinitionGPU* ParticleEffectDefinition::GetRenderDefByName(std::string name)
{
	for (int i = 0; i < (int)m_particleEmitterDefs.size(); i++)
	{
		if (m_particleEmitterDefs[i].GetParticleEmitterDefinition().m_name == name)
		{
			ParticleEmitterDefinition& definition = m_particleEmitterDefs[i].GetParticleEmitterDefinition();
			return &g_theParticleSystem->m_renderDefinitions[definition.m_loadedDefinitionIndex];
		}
	}
	return nullptr;
}

EmitterRenderDefinitionGPU* ParticleEffectDefinition::GetRenderDefByIndex(int index)
{
	if (index >= m_particleEmitterDefs.size())
	{
		return nullptr;
	}
	ParticleEmitterDefinition& definition = m_particleEmitterDefs[index].GetParticleEmitterDefinition();
	return &g_theParticleSystem->m_renderDefinitions[definition.m_loadedDefinitionIndex];
}

ParticleEmitterDefinition& ParticleEmitterInitializer::GetParticleEmitterDefinition()
{
	return g_theParticleSystem->m_loadedEmitterDefinitions[m_emitterIndex];
}

ParticleEmitterDefinition const& ParticleEmitterInitializer::GetConstantParticleEmitterDefinition() const
{
	return g_theParticleSystem->m_loadedEmitterDefinitions[m_emitterIndex];
}
