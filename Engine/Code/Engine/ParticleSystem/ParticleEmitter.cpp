#include "Engine/ParticleSystem/ParticleEmitter.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/IndirectArgsBuffer.hpp"
#include "Engine/ParticleSystem/ParticleEffect.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/VertexUtils.hpp"


void ParticleEmitter::Play(float duration)
{
	EmitterInstanceCPU* emitterInstanceCPU = GetEmitterInstanceCPU();
	emitterInstanceCPU->m_emitTimeLeft = duration;
	float emitTime = GetEmitterUpdateDef()->m_emitTime;
	if (emitTime < duration && emitTime != -1.f)
	{
		emitterInstanceCPU->m_emitTimeLeft = emitTime;
	}
	else
	{
		emitterInstanceCPU->m_emitTimeLeft = duration;
	}
	emitterInstanceCPU->m_currentRepeatTime = 0.f;
	emitterInstanceCPU->m_delayTimeLeft = GetEmitterDefininition()->m_emitterStartDelay;
	SetLocalPosition(emitterInstanceCPU->m_startPosition);
	float activeTimeLeft = emitterInstanceCPU->m_emitTimeLeft;
	if (emitterInstanceCPU->m_emitTimeLeft != -1.f)
	{
		activeTimeLeft = GetEmitterUpdateDef()->m_lifetime[1] + emitterInstanceCPU->m_emitTimeLeft;
	}
	emitterInstanceCPU->m_activeTimeLeft = activeTimeLeft;
	emitterInstanceCPU->m_isActive = 1;
	emitterInstanceCPU->m_currentBurst = 0;
	emitterInstanceCPU->m_currentBurstInterval = 0.f;
	for (int i = 0; i < (int)m_childern.size(); i++)
	{
		m_childern[i]->Play(duration);
	}
}

void ParticleEmitter::Stop()
{
	EmitterInstanceCPU* emitterInstanceCPU = GetEmitterInstanceCPU();
	emitterInstanceCPU->m_emitTimeLeft = 0.f;
	float activeTimeLeft = GetEmitterUpdateDef()->m_lifetime[1];
	emitterInstanceCPU->m_activeTimeLeft = activeTimeLeft;
	for (int i = 0; i < (int)m_childern.size(); i++)
	{
		m_childern[i]->Stop();
	}
}

void ParticleEmitter::KillParticles()
{
	GetEmitterInstanceGPU()->m_killParticles = 1;
	GetEmitterInstanceCPU()->m_isActive = 0;
	for (int i = 0; i < (int)m_childern.size(); i++)
	{
		KillParticles();
	}
}

Vec3 ParticleEmitter::GetWorldPosition() const
{
	if (m_parent != nullptr)
	{
		return m_parent->GetLocalToWorldMatrix().TransformPosition3D(m_position);
	}
	//effect could be nullptr if this emitter was spawned by another emitter
	if (m_effect == nullptr)
	{
		return m_position;
	}
	return m_effect->GetTransformMatrix().TransformPosition3D(m_position);
}

float ParticleEmitter::GetWorldScale() const
{
	if (m_parent != nullptr)
	{
		return m_parent->GetWorldScale() * m_scale;
	}
	if (m_effect != nullptr)
	{
		return m_effect->GetScale() * m_scale;
	}
	return m_scale;
}

Mat44 ParticleEmitter::GetWorldOrientationMatrix() const
{
	if (m_parent != nullptr)
	{
		Mat44 worldOrientation;
		Mat44 parentOrientation = m_parent->GetWorldOrientationMatrix();
		Mat44 emitterOrientation = m_orientationDegrees.GetAsMatrix_IFwd_JLeft_KUp();

		worldOrientation.Append(parentOrientation);
		worldOrientation.Append(emitterOrientation);

		return worldOrientation;
	}
	//effect could be nullptr if this emitter was spawned by another emitter
	if (m_effect == nullptr)
	{
		return m_orientationDegrees.GetAsMatrix_IFwd_JLeft_KUp();
	}
	
	Mat44 worldOrientation;
	Mat44 effectOrientation = m_effect->GetTransformMatrix().GetNormalizedIJKMatrix();
	Mat44 emitterOrientation = m_orientationDegrees.GetAsMatrix_IFwd_JLeft_KUp();

	worldOrientation.Append(effectOrientation);
	worldOrientation.Append(emitterOrientation);

	return worldOrientation;
}

Vec3 ParticleEmitter::GetLocalPosition()
{
	return m_position;
}

EulerAngles ParticleEmitter::GetLocalOrientationDegrees()
{
	return m_orientationDegrees;
}

void ParticleEmitter::DebugDrawEmitter()
{
	if (m_debugVertsNeedUpdate)
	{
		UpdateDebugVerts();
		m_debugVertsNeedUpdate = false;
	}
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants(GetLocalToWorldMatrix());
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->DrawVertexBuffer(m_debugDrawVerts, m_numDebugVerts, 0, VertexType::VERTEX_TYPE_PCU);	
}

void ParticleEmitter::SetLocalPosition(Vec3 const& localPosition)
{
	m_position = localPosition;
	UpdateTransform();
}

void ParticleEmitter::SetLocalOrientationDegrees(EulerAngles const& orientationDegrees)
{
	m_orientationDegrees = orientationDegrees;	
	UpdateTransform();
}

void ParticleEmitter::Translate(Vec3 const& displacment)
{
	Vec3 worldSpaceDisp = GetLocalToWorldMatrix().TransformVectorQuantity3D(displacment);
	m_position += worldSpaceDisp;
	UpdateTransform();
}

FloatGraph* ParticleEmitter::GetFloatGraphByType(FloatGraphType floatGraphType)
{
	EmitterInstanceGPU& emitterInstance = g_theParticleSystem->m_liveEmitterInstancesGPU[m_emitterInstanceIndex];

	int floatGraphIndex = emitterInstance.m_definitionIndex * (int)FloatGraphType::NUM_FLOATGRAPHS + (int)floatGraphType;
	return &g_theParticleSystem->m_floatGraphs[floatGraphIndex];

}

Float2Graph* ParticleEmitter::GetFloat2GraphByType(Float2GraphType floatGraphType)
{
	EmitterInstanceGPU& emitterInstance = g_theParticleSystem->m_liveEmitterInstancesGPU[m_emitterInstanceIndex];

	int floatGraphIndex = emitterInstance.m_definitionIndex * (int)Float2GraphType::NUM_FLOAT2GRAPHS + (int)floatGraphType;
	return &g_theParticleSystem->m_float2Graphs[floatGraphIndex];
}

Float3Graph* ParticleEmitter::GetFloat3GraphByType(Float3GraphType floatGraphType)
{
	EmitterInstanceGPU& emitterInstance = g_theParticleSystem->m_liveEmitterInstancesGPU[m_emitterInstanceIndex];

	int floatGraphIndex = emitterInstance.m_definitionIndex * (int)Float3GraphType::NUM_FLOAT3GRAPHS + (int)floatGraphType;
	return &g_theParticleSystem->m_float3Graphs[floatGraphIndex];

}

EmitterInstanceGPU* ParticleEmitter::GetEmitterInstanceGPU()
{
	return &g_theParticleSystem->m_liveEmitterInstancesGPU[m_emitterInstanceIndex];
}

EmitterInstanceCPU* ParticleEmitter::GetEmitterInstanceCPU()
{
	return &g_theParticleSystem->m_liveEmitterInstancesCPU[m_emitterInstanceIndex];
}

ParticleEmitterDefinition* ParticleEmitter::GetEmitterDefininition()
{
	EmitterInstanceGPU* emitterInstance = &g_theParticleSystem->m_liveEmitterInstancesGPU[m_emitterInstanceIndex];
	return &g_theParticleSystem->m_loadedEmitterDefinitions[emitterInstance->m_definitionIndex];
}

void ParticleEmitter::ResetChildren(ParticleEmitterDefinition const& emitterDef)
{
	for (int i = 0; i < (int)m_childern.size(); i++)
	{
		delete m_childern[i];
	}
	m_childern.clear();
	for (int i = 0; i < (int)emitterDef.m_childEmitters.size(); i++)
	{
		ParticleEmitterInitializer initializer;
		initializer.m_emitterIndex = emitterDef.m_childEmitters[i].m_childEmitterIndex;
		initializer.m_orientationDegrees = emitterDef.m_childEmitters[i].m_localOrientation;
		initializer.m_position = emitterDef.m_childEmitters[i].m_localPosition;

		ParticleEmitter* child = new ParticleEmitter(initializer, m_effect, this);
		m_childern.push_back(child);
	}
}

ParticleEmitter::ParticleEmitter(ParticleEmitterInitializer const& initializer, ParticleEffect* effect, ParticleEmitter* parent)
	: m_effect(effect)
	, m_position(initializer.m_position) 
	, m_scale(initializer.m_scale)
	, m_orientationDegrees(initializer.m_orientationDegrees)
	, m_parent(parent)
{
	ParticleEmitterDefinition const& def = initializer.GetConstantParticleEmitterDefinition();
	ResetChildren(def);
	m_emitterInstanceIndex = g_theParticleSystem->AddParticleEmitterToSystem(def, this);
	UpdateTransform();

	if (g_theParticleSystem->m_updateDefinitions[def.m_loadedDefinitionIndex].m_atlasUVMins == Vec2::ZERO &&
		g_theParticleSystem->m_updateDefinitions[def.m_loadedDefinitionIndex].m_atlasUVMaxs == Vec2::ZERO)
	{
		AABB2 spriteSheetBoundsInAtlas = g_theParticleSystem->GetImageBoundsInSpriteAtlas(def.m_spriteSheetFilePath);
		g_theParticleSystem->m_updateDefinitions[def.m_loadedDefinitionIndex].m_atlasUVMins = spriteSheetBoundsInAtlas.m_mins;
		g_theParticleSystem->m_updateDefinitions[def.m_loadedDefinitionIndex].m_atlasUVMaxs = spriteSheetBoundsInAtlas.m_maxs;
		g_theParticleSystem->m_updateDefinitions[def.m_loadedDefinitionIndex].isDirty = 1;
		g_theParticleSystem->m_renderDefinitions[def.m_loadedDefinitionIndex].isDirty = 1;
	}
}

EmitterUpdateDefinitionGPU* ParticleEmitter::GetEmitterUpdateDef()
{
	m_debugVertsNeedUpdate = true;
	EmitterInstanceGPU* emitterInstance = &g_theParticleSystem->m_liveEmitterInstancesGPU[m_emitterInstanceIndex];
	return &g_theParticleSystem->m_updateDefinitions[emitterInstance->m_definitionIndex];
}

EmitterRenderDefinitionGPU* ParticleEmitter::GetEmitterRenderDef()
{
	m_debugVertsNeedUpdate = true;
	EmitterInstanceGPU* emitterInstance = &g_theParticleSystem->m_liveEmitterInstancesGPU[m_emitterInstanceIndex];

	return &g_theParticleSystem->m_renderDefinitions[emitterInstance->m_definitionIndex];
}

Mat44 ParticleEmitter::GetLocalToWorldMatrix() const
{
	Mat44 localToWorldMatrix;
	localToWorldMatrix.AppendTranslation3D(GetWorldPosition());
	localToWorldMatrix.Append(GetWorldOrientationMatrix());
	localToWorldMatrix.AppendScaleUniform3D(GetWorldScale());
	return localToWorldMatrix;
}

void ParticleEmitter::UpdateTransform()
{
	Mat44 localToWorldMatrix = GetLocalToWorldMatrix();
	GetEmitterInstanceGPU()->m_localToWorldMatrix = localToWorldMatrix;
	GetEmitterInstanceGPU()->m_worldToLocalMatrix = localToWorldMatrix.GetOrthonormalInverse();
	for (int i = 0; i < m_childern.size(); i++)
	{
		m_childern[i]->UpdateTransform();
	}
	//DebugAddWorldBasis(localToWorldMatrix, 0.f);
}

ParticleEmitter::~ParticleEmitter()
{
	for (int i = 0; i < m_childern.size(); i++)
	{
		delete m_childern[i];
		m_childern[i] = nullptr;
	}
	GetEmitterInstanceGPU()->m_killParticles = 1;
	GetEmitterInstanceCPU()->m_isActive = 0;
	GetEmitterInstanceCPU()->m_emitter = nullptr;
	if (m_effect != nullptr)
	{
		for (int i = 0; i < (int)m_effect->m_emitters.size(); i++)
		{
			if (m_effect->m_emitters[i] == this)
			{
				m_effect->m_emitters.erase(m_effect->m_emitters.begin() + i);
				break;
			}
		}
	}


	delete m_debugDrawVerts;
}


void ParticleEmitter::UpdateDebugVerts()
{
	std::vector<Vertex_PCU> debugVerts;
	debugVerts.reserve(5000);

	AddVertsForBasis3D(debugVerts, Mat44());

	/*
	//point force
	if (GetEmitterUpdateDef()->m_pointForceStrength > 0.f && GetEmitterUpdateDef()->m_pointForceRadius > 0.f)
	{
		Vec3 pointForcePos;
		pointForcePos.SetFromFloats(GetEmitterUpdateDef()->m_pointForcePosition);
		AddVertsForSphere3D(debugVerts, pointForcePos, .25f, Rgba8::WHITE);
		if (GetEmitterUpdateDef()->m_pointForceAttract == 1)
		{
			AddVertsForSphere3D(debugVerts, pointForcePos, GetEmitterUpdateDef()->m_pointForceRadius, Rgba8(0, 0, 255, 60));
		}
		else
		{
			AddVertsForSphere3D(debugVerts, pointForcePos, GetEmitterUpdateDef()->m_pointForceRadius, Rgba8(255, 0, 0, 60));
		}
	}
	*/

	/*
	//linear force
	Vec3 linearForce;
	linearForce.SetFromFloats(GetEmitterUpdateDef()->m_linearForce.constantValue);
	if (linearForce.GetLength() > 0.f)
	{

		Vec3 linearForceStartPos =Vec3::ZERO;
		Vec3 linearForceEndPos = linearForceStartPos + linearForce * .15f;
		AddVertsForArrow3D(debugVerts, linearForceStartPos, linearForceEndPos, .03f, Rgba8::YELLOW);
	}
	*/

	//vortex force
	Vec3 vortexForceOrigin;
	Vec3 vortexForceAxis;
	/*
	float vortexForceStrength = GetEmitterUpdateDef()->m_vortexForce;
	if (vortexForceStrength > 0.f)
	{
		vortexForceAxis.SetFromFloats(GetEmitterUpdateDef()->m_vortexAxisDir);
		vortexForceOrigin = GetEmitterUpdateDef()->m_vortexAxisOrigin;
		AddVertsForSphere3D(debugVerts, vortexForceOrigin, .25f);
		AddVertsForArrow3D(debugVerts, vortexForceOrigin, vortexForceOrigin + (vortexForceAxis * vortexForceStrength * .15f), .03f, Rgba8(154, 112, 204));
		AddVertsForCylinder3D(debugVerts, vortexForceAxis * -100.f, vortexForceAxis * 100.f, GetEmitterUpdateDef()->m_vortexForceRadius, Rgba8(120, 100, 220, 60));
	}
	*/

	if (GetEmitterUpdateDef()->m_emissionType == (unsigned int)EmissionType::SPHERE)
	{
		AddVertsForSphere3D(debugVerts, Vec3::ZERO, GetEmitterUpdateDef()->m_emissionRadius, Rgba8(255, 255, 255, 50));
	}

	float lineWidth = .02f;
	if (GetEmitterUpdateDef()->m_emissionType == (unsigned int)EmissionType::BOX)
	{
		//emission box
		float boxHalfLength = GetEmitterUpdateDef()->m_boxDimensions[0] * .5f;
		float boxHalfWidth = GetEmitterUpdateDef()->m_boxDimensions[1] * .5f;
		float boxHalfHeight = GetEmitterUpdateDef()->m_boxDimensions[2] * .5f;

		Vec3 boxBackLeftBottom = Vec3(-boxHalfLength, boxHalfWidth, -boxHalfHeight);
		Vec3 boxBackRightBottom = Vec3(-boxHalfLength, -boxHalfWidth, -boxHalfHeight);
		Vec3 boxBackRightTop = Vec3(-boxHalfLength, -boxHalfWidth, boxHalfHeight);
		Vec3 boxBackLeftTop = Vec3(-boxHalfLength, boxHalfWidth, boxHalfHeight);
		Vec3 boxForwardLeftBottom = Vec3(boxHalfLength, boxHalfWidth, -boxHalfHeight);
		Vec3 boxForwardRightBottom = Vec3(boxHalfLength, -boxHalfWidth, -boxHalfHeight);
		Vec3 boxForwardRightTop = Vec3(boxHalfLength, -boxHalfWidth, boxHalfHeight);
		Vec3 boxForwardLeftTop = Vec3(boxHalfLength, boxHalfWidth, boxHalfHeight);

		AddVertsForBoxLine3D(debugVerts, boxBackLeftBottom, boxBackRightBottom, lineWidth, Rgba8::WHITE);
		AddVertsForBoxLine3D(debugVerts, boxBackRightBottom, boxBackRightTop, lineWidth, Rgba8::WHITE);
		AddVertsForBoxLine3D(debugVerts, boxBackRightTop, boxBackLeftTop, lineWidth, Rgba8::WHITE);
		AddVertsForBoxLine3D(debugVerts, boxBackLeftTop, boxBackLeftBottom, lineWidth, Rgba8::WHITE);
		AddVertsForBoxLine3D(debugVerts, boxBackLeftBottom, boxForwardLeftBottom, lineWidth, Rgba8::WHITE);
		AddVertsForBoxLine3D(debugVerts, boxBackRightBottom, boxForwardRightBottom, lineWidth, Rgba8::WHITE);
		AddVertsForBoxLine3D(debugVerts, boxForwardLeftBottom, boxForwardRightBottom, lineWidth, Rgba8::WHITE);
		AddVertsForBoxLine3D(debugVerts, boxForwardRightBottom, boxForwardRightTop, lineWidth, Rgba8::WHITE);
		AddVertsForBoxLine3D(debugVerts, boxForwardRightTop, boxForwardLeftTop, lineWidth, Rgba8::WHITE);
		AddVertsForBoxLine3D(debugVerts, boxForwardRightTop, boxBackRightTop, lineWidth, Rgba8::WHITE);
		AddVertsForBoxLine3D(debugVerts, boxForwardLeftTop, boxForwardLeftBottom, lineWidth, Rgba8::WHITE);
		AddVertsForBoxLine3D(debugVerts, boxForwardLeftTop, boxBackLeftTop, lineWidth, Rgba8::WHITE);
	}
	

	if (m_debugDrawVerts != nullptr)
	{
		delete m_debugDrawVerts;
	}
	m_debugDrawVerts = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU) * debugVerts.size());
	g_theRenderer->CopyCPUToGPU(debugVerts.data(), debugVerts.size() * sizeof(Vertex_PCU), m_debugDrawVerts);
	m_numDebugVerts = (int)debugVerts.size();
}


