#include "ParticleCommon.h"

StructuredBuffer<Particle> particleBuffer : register(t0);
StructuredBuffer<ParticleEmitterInstanceGPU> emitterInstances : register(t1);
StructuredBuffer<ParticleEmitterUpdateDefinitionGPU> updateDefs : register(t2);
StructuredBuffer<uint> meshParticleDrawList : register(t3);
ByteAddressBuffer counterBuffer : register(t4);
StructuredBuffer<MeshEntry> meshEntries : register(t5);

RWStructuredBuffer<uint> vertexCountBuffer : register(u0);


[numthreads(64, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x >= c_maxParticles || DTid.x >= counterBuffer.Load(PARTICLECOUNTER_OFFSET_MESH_DRAWCOUNT))
	{
		return;
	}
	uint particleIndex = meshParticleDrawList[DTid.x];
	Particle particle = particleBuffer[particleIndex];
	ParticleEmitterInstanceGPU emitterInstance = emitterInstances[particle.configIndex];
	ParticleEmitterUpdateDefinitionGPU config;
	config = updateDefs[emitterInstance.definitionIndex];
	if (config.partialMeshTriangles > 0)
	{
		vertexCountBuffer[DTid.x] = config.partialMeshTriangles * 3;
	}
	else
	{
		vertexCountBuffer[DTid.x] = meshEntries[config.particleMeshIndex].size;
	}
}