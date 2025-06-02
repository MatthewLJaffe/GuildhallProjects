/*

#include "Engine/ParticleSystem/IndividualParticleEffect.hpp"

ParticleEffectDefinition* IndividualParticleEffect::GetEffectDefinition()
{
	return &m_particleEffectDefinition;
}

IndividualParticleEffect::IndividualParticleEffect(ParticleEffectDefinition const& effectDef)
	: ParticleEffect(effectDef)
	, m_particleEffectDefinition(effectDef)
{
}

void IndividualParticleEffect::AddEmitter(EmitterInEffectDefinition const& initializer)
{
	ParticleEmitter* currEmitter = new ParticleEmitter(initializer, this);
	m_particleEffectDefinition.m_particleEmitterDefs.push_back(initializer);
	m_emitters.push_back(currEmitter);
}

*/