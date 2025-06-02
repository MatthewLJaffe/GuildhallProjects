#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/ParticleSystem/Particle.hpp"
#include "Engine/ParticleSystem/ParticleEmitterDefinition.hpp"

struct EmitterUpdateDefinitionGPU;
struct EmitterRenderDefinitionGPU;
struct ParticleEmitterInitializer;
class ParticleEffect;
class VertexBuffer;
class ParticleEmitter;

struct EmitterInstanceCPU
{
	ParticleEmitter* m_emitter = nullptr;
	float m_owedParticles = 0.f;
	float m_emitTimeLeft = -1.f;
	float m_emitterLifetime = -1.f;
	float m_delayTimeLeft = 0.f;
	float m_activeTimeLeft = -1.f;
	float m_currentRepeatTime = -1.f;
	int m_currentBurst = 0;
	float m_currentBurstInterval = -1.f;
	unsigned int m_seed = 0;
	Vec3 m_startPosition;
	int m_isActive = 1;
	int m_loadedEmitterDefIdx = -1;

	//for emitting from emitter
	bool m_spawnSubEmitters = false;
	int m_subEmitterDefIdx = -1;
	bool m_isSubEmitter = false;
	Vec3 m_lastFramePos;
};

struct EmitterInstanceGPU
{
	Mat44 m_localToWorldMatrix;

	Mat44 m_worldToLocalMatrix;

	int m_definitionIndex = -1;
	unsigned int m_particlesToEmitThisFrame = 0;
	unsigned int m_killParticles = 0;
	unsigned int m_emissionStartIdx = 0;

	int m_totalParticlesEmitted = 0;
	Vec3 m_emitterVelocity;
};

class ParticleEmitter
{
	friend class ParticleSystem;
	friend class ParticleEffect;
	friend class IndividualParticleEffect;
public:
	void Play(float duration);
	void Stop();
	void KillParticles();
	Vec3 GetWorldPosition() const;
	float GetWorldScale() const;
	Mat44 GetWorldOrientationMatrix() const;
	Vec3 GetLocalPosition();
	EulerAngles GetLocalOrientationDegrees();
	void DebugDrawEmitter();
	void SetLocalPosition(Vec3 const& localPosition);
	void SetLocalOrientationDegrees(EulerAngles const& orientationDegrees);
	void Translate(Vec3 const& displacment);

	FloatGraph* GetFloatGraphByType(FloatGraphType floatGraphType);
	Float2Graph* GetFloat2GraphByType(Float2GraphType floatGraphType);
	Float3Graph* GetFloat3GraphByType(Float3GraphType floatGraphType);
	EmitterUpdateDefinitionGPU* GetEmitterUpdateDef();
	EmitterRenderDefinitionGPU* GetEmitterRenderDef();

	Mat44 GetLocalToWorldMatrix() const;
	void UpdateTransform();
	EmitterInstanceGPU* GetEmitterInstanceGPU();
	EmitterInstanceCPU* GetEmitterInstanceCPU();
	ParticleEmitterDefinition* GetEmitterDefininition();
	void ResetChildren(ParticleEmitterDefinition const& emitterDef);

	~ParticleEmitter();
public:
	ParticleEffect* m_effect;

private:
	ParticleEmitter(ParticleEmitterInitializer const& initializer, ParticleEffect* effect, ParticleEmitter* parent = nullptr);


	void UpdateDebugVerts();
private:
	std::vector<ParticleEmitter*> m_childern;
	ParticleEmitter* m_parent = nullptr;
	Vec3 m_position = Vec3::ZERO;
	float m_scale = 1.f;
	EulerAngles m_orientationDegrees;
	int m_emitterInstanceIndex = -1;
	VertexBuffer* m_debugDrawVerts = nullptr;
	int m_numDebugVerts = 0;
	bool m_debugVertsNeedUpdate = true;
};