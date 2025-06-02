#include "ParticleCommon.h"

StructuredBuffer<ParticleEmitterInstanceGPU> emitterInstances : register(t0);
StructuredBuffer<ParticleEmitterUpdateDefinitionGPU> updateDefs : register(t1);
StructuredBuffer<MeshParticleVert> meshParticleVerts : register(t2);
StructuredBuffer<MeshEntry> meshEntries : register(t3);

RWStructuredBuffer<Particle> particleBuffer : register(u0);
RWStructuredBuffer<uint> aliveList1 : register(u1);
RWStructuredBuffer<uint> aliveList2 : register(u2);
RWStructuredBuffer<uint> deadList : register(u3);
RWByteAddressBuffer counterBuffer : register(u4);

int2 GetEmitterConfigIndex(uint DTidx)
{
	uint beginIndex = 0;
	uint endIndex = c_numEmitterConfigs;
	bool emitterIndexFound = false;
	while (!emitterIndexFound)
	{
		uint currentIndex = (beginIndex + endIndex) / 2;
		//search left side
		if (DTidx < emitterInstances[currentIndex].emissionStartIdx)
		{
			endIndex = currentIndex - 1;
		}
		//search right side
		else if (DTidx >= emitterInstances[currentIndex].emissionStartIdx + emitterInstances[currentIndex].particlesToEmitThisFrame)
		{
			beginIndex = currentIndex + 1;
		}
		//found
		else
		{
			int particleEmittedThisFrame = DTidx - emitterInstances[currentIndex].emissionStartIdx;
			int idInEmitter = emitterInstances[currentIndex].totalParticlesEmitted - (particleEmittedThisFrame + 1);
			return int2(currentIndex, idInEmitter);
		}
	}
	return int2(0, 0);
}

[numthreads(64, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	if (DTid.x >= c_emittedParticles || DTid.x >= c_maxParticles)
	{
		return;
	}

	int2 instanceIdxAndidInEmitter = GetEmitterConfigIndex(DTid.x);
	ParticleEmitterInstanceGPU emitterInstance = emitterInstances[instanceIdxAndidInEmitter.x];
	if (emitterInstance.killParticles == 1)
	{
		return;
	}
	
	//retrieve index in particle buffer
	int deadCount;
	counterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_DEADCOUNT, -1, deadCount);
	if(deadCount < 1)
	{
		counterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_DEADCOUNT, 1, deadCount);
		return;
	}
	uint newParticleIndex = deadList[deadCount - 1];

	RNG rng;
	rng.init(uint2(0, DTid.x), c_frameCount);

	ParticleEmitterUpdateDefinitionGPU config;
	config = updateDefs[emitterInstance.definitionIndex];

	
	Particle particle;
	particle.configIndex = instanceIdxAndidInEmitter.x;
	particle.idInEmitter = instanceIdxAndidInEmitter.y;
	particle.position = float3(0.f, 0.f, 0.f);

	//position is 
	if (config.partialMeshTriangles > 0)
	{
		int partialMeshVerts = config.partialMeshTriangles * 3;
		MeshEntry meshToEmitFrom = meshEntries[config.particleMeshIndex];
		int startVertInMesh = partialMeshVerts * particle.idInEmitter % meshToEmitFrom.size;
		float3 averagePos = float3(0.f, 0.f, 0.f);
		for (int i = startVertInMesh; i < startVertInMesh + partialMeshVerts; i++)
		{
			averagePos += meshParticleVerts[meshToEmitFrom.offset + i].position;
		}
		averagePos /= (float)partialMeshVerts;
		particle.position = averagePos;
	}
	//SPHERE
	else if (config.emissionType == 0)
	{
		float3 randomDirection = float3((rng.next_float() * 2 - 1), (rng.next_float() * 2 - 1), (rng.next_float() * 2 - 1));
		randomDirection = normalize(randomDirection);
		float randomLength = 
		GetValueFromEasingFunciton(config.emissionDistribution, rng.next_float()) * config.emissionRadius;
		particle.position = randomDirection * randomLength;
	}
	//BOX
	else if (config.emissionType == 1)
	{
		float normalizedXDist = rng.next_float();
		float xNegCoinToss = rng.next_float();
		float normalizedYDist = rng.next_float();
		float yNegCoinToss = rng.next_float();
		float normalizedZDist = rng.next_float();
		float zNegCoinToss = rng.next_float();
		normalizedXDist = GetValueFromEasingFunciton(config.emissionDistribution, normalizedXDist);
		normalizedYDist = GetValueFromEasingFunciton(config.emissionDistribution, normalizedYDist);
		normalizedZDist = GetValueFromEasingFunciton(config.emissionDistribution, normalizedZDist);

		particle.position.x = lerp(0.f, .5f*config.boxDimensions.x, normalizedXDist);
		particle.position.y = lerp(0.f, .5f*config.boxDimensions.y, normalizedYDist);
		particle.position.z = lerp(0.f, .5f*config.boxDimensions.z, normalizedZDist);
		if (xNegCoinToss > .5f)
		{
			particle.position.x *= -1;
		}
		if (yNegCoinToss > .5f)
		{
			particle.position.y *= -1;
		}
		if (zNegCoinToss > .5f)
		{
			particle.position.z *= -1;
		}
	}
	//MESH
	else if (config.emissionType == 2)
	{
		MeshEntry meshToEmitFrom = meshEntries[config.meshEntryEmissionIndex];
		uint numTriangels = meshToEmitFrom.size / 3;
		uint triangleToEmitFrom = rng.get_random_uint_in_range(0, numTriangels - 1);
		MeshParticleVert v1 = meshParticleVerts[meshToEmitFrom.offset + triangleToEmitFrom * 3 + 0];
		MeshParticleVert v2 = meshParticleVerts[meshToEmitFrom.offset + triangleToEmitFrom * 3 + 1];
		MeshParticleVert v3 = meshParticleVerts[meshToEmitFrom.offset + triangleToEmitFrom * 3 + 2];

		float v1Weight = rng.next_float();
		float v2Weight = rng.next_float();
		float v3Weight = rng.next_float();

		float totalWeight = v1Weight + v2Weight + v3Weight;

		v1Weight /= totalWeight;
		v2Weight /= totalWeight;
		v3Weight /= totalWeight;
		
		particle.position = v1.position * v1Weight + v2.position * v2Weight + v3.position * v3Weight;
		particle.position.x *= config.meshScale.x;
		particle.position.y *= config.meshScale.y;
		particle.position.z *= config.meshScale.z;
	}

	if (config.worldSimulation == 1)
	{
		particle.position = (float3)mul(emitterInstance.localToWorldMatrix, float4(particle.position, 1.f));
	}
	particle.emissionPosition = particle.position;
	particle.currentDelayReturnTime = config.returnToOriginDelay;
	particle.lifetimeVelocity = float3(0.f, 0.f, 0.f);

	particle.velocity.x = rng.get_random_float_in_range(config.velocityXRange.x, config.velocityXRange.y);
	particle.velocity.y = rng.get_random_float_in_range(config.velocityYRange.x, config.velocityYRange.y);
	particle.velocity.z = rng.get_random_float_in_range(config.velocityZRange.x, config.velocityZRange.y);

	particle.reflectionVector = float3(0.f, 0.f, 0.f);
	particle.padding = 0.f;
	
	if (config.inheritEmitterVelocity == 1)
	{
		if (config.worldSimulation == 1)
		{
			particle.velocity += emitterInstance.emitterVelocity * config.inheritVelocityPercentage;
		}
		else
		{
			float3 velocityToInheritWorldSpace = emitterInstance.emitterVelocity * config.inheritVelocityPercentage;
			float3 velocityInLocalSpace = (float3)mul(emitterInstance.worldToLocalMatrix, float4(velocityToInheritWorldSpace, 0.f));
			particle.velocity += velocityInLocalSpace;
		}
	}
	particle.liveTime = Get1dNoiseInRange(config.lifetime.x, config.lifetime.y, 0, newParticleIndex + SEED_OFFSET_LIFETIME);

	particle.uvBottomLeft = float2(0.f,0.f);
	particle.uvTopRight = float2(0.f,0.f);

	// write out the new particle:
	particleBuffer[newParticleIndex] = particle;

	// and add index to the alive list (push):
	uint aliveCount;
	counterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_ALIVECOUNT, 1, aliveCount);
	aliveList1[aliveCount] = newParticleIndex;
}
