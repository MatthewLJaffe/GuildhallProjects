#pragma once
#include "Engine/ParticleSystem/ParticleEmitter.hpp"
#include "Engine/ParticleSystem/ParticleEmitterDefinition.hpp"

struct ParticleEmitterInitializer
{
	Vec3 m_position;
	EulerAngles m_orientationDegrees;
	float m_scale = 1.f;
	int m_emitterIndex = -1;
	ParticleEmitterDefinition& GetParticleEmitterDefinition();
	ParticleEmitterDefinition const& GetConstantParticleEmitterDefinition() const;

};

struct ParticleEffectDefinition
{
	std::string m_name;
	float m_scale = 1.f;
	std::vector<ParticleEmitterInitializer> m_particleEmitterDefs;
	EmitterUpdateDefinitionGPU* GetUpdateDefByName(std::string name);
	EmitterUpdateDefinitionGPU* GetUpdateDefByIndex(int index);
	EmitterRenderDefinitionGPU* GetRenderDefByName(std::string name);
	EmitterRenderDefinitionGPU* GetRenderDefByIndex(int index);
	int m_effectDefinitionIndex = -1;
};

class ParticleEffect
{
	friend class ParticleSystem;
public:
	Vec3 GetPosition();
	void SetWorldTransform(Mat44 const& transform);
	void SetScale(float uniformScale);
	float GetScale() const;
	void SetPosition(Vec3 const& position);
	EulerAngles GetOrientationDegrees();
	void SetOrientationDegrees(EulerAngles const& orientationDegrees);
	void Stop();
	void KillParticles();

	void Play(float duration = -1.f);
	bool IsActive();
	void DebugDrawParticleEffect();
	Mat44 GetTransformMatrix() const;
	ParticleEffectDefinition* GetEffectDefinition();
	void AddEmitter(ParticleEmitterInitializer& config);
	int m_definitionIndex = -1;
	std::vector<ParticleEmitter*> m_emitters;
	std::string m_name;
	~ParticleEffect();
	VertexBuffer* m_debugVerts = nullptr;
	int m_numDebugVerts = 0;

protected:
	bool m_useWorldTransform = false;
	Mat44 m_worldTransform;
	Vec3 m_position;
	EulerAngles m_orientationDegrees;
	float m_uniformScale = 1.f;
	ParticleEffect(ParticleEffectDefinition const& effectDef, int definitionIndex);
};