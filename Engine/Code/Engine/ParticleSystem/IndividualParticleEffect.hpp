/*
#pragma once
#include "Engine/ParticleSystem/ParticleEffect.hpp"

class IndividualParticleEffect : public ParticleEffect
{
	friend class ParticleSystem;
public:
	ParticleEffectDefinition* GetEffectDefinition();
	void AddEmitter(EmitterInEffectDefinition const& config);
private:
	IndividualParticleEffect(ParticleEffectDefinition const& effectDef);
	ParticleEffectDefinition m_particleEffectDefinition;
};
*/