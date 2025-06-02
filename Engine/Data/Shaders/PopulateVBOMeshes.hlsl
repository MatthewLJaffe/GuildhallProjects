#include "ParticleCommon.h"

RWByteAddressBuffer VertexBuffer : register(u0);

StructuredBuffer<ParticleEmitterInstanceGPU> emitterInstances : register(t0);
StructuredBuffer<ParticleEmitterUpdateDefinitionGPU> updateDefs : register(t1);
StructuredBuffer<Particle> particleBuffer : register(t2);
StructuredBuffer<uint> drawList : register(t3);
ByteAddressBuffer counterBuffer : register(t4);
StructuredBuffer<uint> meshVertexOffsetList : register(t5);
StructuredBuffer<MeshEntry> meshEntries : register(t6);
StructuredBuffer<MeshParticleVert> meshParticleVerts : register(t7);
StructuredBuffer<FloatGraph> floatGraphs : register(t8);
StructuredBuffer<Float2Graph> float2Graphs : register(t9);
StructuredBuffer<Float3Graph> float3Graphs : register(t10);

[numthreads(64, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x >= c_maxParticles || DTid.x >= counterBuffer.Load(PARTICLECOUNTER_OFFSET_MESH_DRAWCOUNT))
	{
		return;
	}
    int particleIndex = drawList[DTid.x];
    Particle particle = particleBuffer[particleIndex];

    ParticleEmitterInstanceGPU emitterInstance = emitterInstances[particle.configIndex];
	ParticleEmitterUpdateDefinitionGPU config;
	config = updateDefs[emitterInstance.definitionIndex];

	//over lifetime fields
    float normalizedLifePos =  particle.liveTime / Get1dNoiseInRange(config.lifetime.x, config.lifetime.y, 
    0, particleIndex + SEED_OFFSET_LIFETIME);
    normalizedLifePos = 1.f - normalizedLifePos;

    float4 color = config.colorOverLifetime[0].color;
    for (uint i = 0; i < config.numColorsOverLifetime; i++)
    {
        if (i == config.numColorsOverLifetime - 1)
        {
            color = config.colorOverLifetime[i].color;
            break;
        }

        if (config.colorOverLifetime[i].time > normalizedLifePos || config.colorOverLifetime[i + 1].time < normalizedLifePos)
        {
            continue;
        }


        float fractionInRange = (normalizedLifePos - config.colorOverLifetime[i].time) / (config.colorOverLifetime[i + 1].time - config.colorOverLifetime[i].time);
        color = lerp(config.colorOverLifetime[i].color, config.colorOverLifetime[i + 1].color, fractionInRange);
        
        break;
    }

    uint vertexColorAsUint = GetColorAsUint( color );
    
    float emissive = GetValueInFloatGraph(floatGraphs, FLOATGRAPH_EMISSIVE, 
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_EMISSIVE);

    float alpha = GetValueInFloatGraph(floatGraphs, FLOATGRAPH_ALPHA_OBSCURANCE,
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_ALPHA_OBSCURANCE);

    float panTextureContribution = GetValueInFloatGraph(floatGraphs, FLOATGRAPH_PAN_TEXTURE_CONTRIBUTION,
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_PAN_TEXTURE_CONTRIBUTION);

    float maxLife = Get1dNoiseInRange(config.lifetime.x, config.lifetime.y, 
    0, SEED_OFFSET_LIFETIME);
    float aliveTime = maxLife - particle.liveTime;

	//add verts
    int vertexSize = 64;
	MeshEntry meshToAdd = meshEntries[config.particleMeshIndex];
    float2 bottomLeftUV = particleBuffer[particleIndex].uvBottomLeft;
    float2 uvSize = particleBuffer[particleIndex].uvTopRight - bottomLeftUV;
    float3 scale = GetValueInFloat3Graph(float3Graphs, FLOAT3GRAPH_LIFETIME_SCALE_3D,
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_SCALE_3D);
    float3 rotation = GetValueInFloat3Graph(float3Graphs, FLOAT3GRAPH_LIFETIME_ROTATION_3D,
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_ROTATION_3D);
    float3 particlePos = particle.position;
    if (config.partialMeshTriangles > 0)
	{
        particlePos -= particle.emissionPosition;
	}

    float4x4 transform = {  1.f,	    0.f,	    0.f,	    0.f,
							0.f,	    1.f,	    0.f,	    0.f,
							0.f,	    0.f,	    1.f,	    0.f,
							0.f,	    0.f,	    0.f,	    1.f};

    float4x4 positionMatrix = { 1.f,	    0.f,	    0.f,	   particlePos.x,
							    0.f,	    1.f,	    0.f,	   particlePos.y,   
							    0.f,	    0.f,	    1.f,	   particlePos.z,
							    0.f,	    0.f,	    0.f,	   1.f};

    float4x4 scaleMatrix = {scale.x,	0.f,	    0.f,	    0.f,
							0.f,	    scale.y,	0.f,	    0.f,
							0.f,	    0.f,	    scale.z,	0.f,
							0.f,	    0.f,	    0.f,	    1.f};
    float4x4 rotationMatrix;
    float velocityLength = length(particle.velocity);
    if (config.orientToVelocity == 1 && velocityLength > .0001f)
    {
        float maxVelocityContribution = 1.f;
        float maxVelocityOrientation = 12.f;
        if (velocityLength > maxVelocityOrientation)
        {
            velocityLength = maxVelocityOrientation;
        }
        float velocityContribution = (velocityLength / maxVelocityOrientation) * maxVelocityContribution;
        float3 iBasis = lerp(float3(1.f, 0.f, 0.f), normalize(particle.velocity), velocityContribution);
        float3 jBasis;
        float3 kBasis;
        float3 worldUp = float3(0.f, 0.f, 1.f);
		if (abs(dot(iBasis, worldUp)) != 1.f)
		{
			jBasis = normalize(cross(worldUp, iBasis));
			kBasis = normalize(cross(iBasis, jBasis));
			
		}
		else
		{
			jBasis = normalize(cross(float3(0.f, 1.f, 0.f), iBasis));
			kBasis = normalize(cross(iBasis, jBasis));
		}
        
        rotationMatrix = float4x4(  iBasis.x,	jBasis.x,	kBasis.x,	0.f,
							        iBasis.y,	jBasis.y,	kBasis.y,	0.f,
							        iBasis.z,	jBasis.z,	kBasis.z,	0.f,
							        0.f,	    0.f,	    0.f,	    1.f );
    }
    else
    {
        rotationMatrix = GetEulerAnglesAsMatrix(rotation);
    }
    if (config.worldSimulation == 0)
    {
        transform = emitterInstance.localToWorldMatrix;
    }
    transform = mul(transform, positionMatrix);
    transform = mul(transform, scaleMatrix);
    transform = mul(transform, rotationMatrix);

    //partial mesh
    if (config.partialMeshTriangles > 0)
	{
		int partialMeshVerts = config.partialMeshTriangles * 3;
		MeshEntry meshToEmitFrom = meshEntries[config.particleMeshIndex];
		int startVertInMesh = partialMeshVerts * particle.idInEmitter % meshToEmitFrom.size;
		for (int i = startVertInMesh; i < startVertInMesh + partialMeshVerts; i++)
		{
			MeshParticleVert currentVert = meshParticleVerts[meshToEmitFrom.offset + i];
		    uint offsetInVBO = meshVertexOffsetList[DTid.x] + (i - startVertInMesh);
            float3 vertWorldPos = (float3)mul(transform, float4(currentVert.position, 1.f));
            float3 vertWorldNormal = (float3)mul(transform, float4(currentVert.normal, 0.f));

            VertexBuffer.Store(offsetInVBO * vertexSize,              asuint(vertWorldPos.x));
            VertexBuffer.Store(offsetInVBO * vertexSize + 4,          asuint(vertWorldPos.y));
            VertexBuffer.Store(offsetInVBO * vertexSize + 8,          asuint(vertWorldPos.z));
            VertexBuffer.Store(offsetInVBO * vertexSize + 12,         asuint(vertexColorAsUint));
            VertexBuffer.Store(offsetInVBO * vertexSize + 16,         asuint(bottomLeftUV.x + currentVert.uv.x * uvSize.x));
            VertexBuffer.Store(offsetInVBO * vertexSize + 20,         asuint(bottomLeftUV.y + currentVert.uv.y * uvSize.y));
            VertexBuffer.Store(offsetInVBO * vertexSize + 24,         asuint(emitterInstance.definitionIndex));
            VertexBuffer.Store(offsetInVBO * vertexSize + 28,         asuint(currentVert.uv.x));
            VertexBuffer.Store(offsetInVBO * vertexSize + 32,         asuint(currentVert.uv.y));
            VertexBuffer.Store(offsetInVBO * vertexSize + 36,         asuint(emissive));
            VertexBuffer.Store(offsetInVBO * vertexSize + 40,         asuint(alpha));
            VertexBuffer.Store(offsetInVBO * vertexSize + 44,         asuint(panTextureContribution));
            VertexBuffer.Store(offsetInVBO * vertexSize + 48,         asuint(aliveTime));
            VertexBuffer.Store(offsetInVBO * vertexSize + 52,         asuint(vertWorldNormal.x));
            VertexBuffer.Store(offsetInVBO * vertexSize + 56,         asuint(vertWorldNormal.y));
            VertexBuffer.Store(offsetInVBO * vertexSize + 60,         asuint(vertWorldNormal.z));
		}

	}
    else
    {
        for (uint vIdx = 0; vIdx < meshToAdd.size; vIdx++)
	    {
		    MeshParticleVert currentVert = meshParticleVerts[vIdx + meshToAdd.offset];
		    uint offsetInVBO = meshVertexOffsetList[DTid.x] + vIdx;
            float3 vertWorldPos = (float3)mul(transform, float4(currentVert.position, 1.f));
            float3 vertWorldNormal = (float3)mul(transform, float4(currentVert.normal, 0.f));

            VertexBuffer.Store(offsetInVBO * vertexSize,              asuint(vertWorldPos.x));
            VertexBuffer.Store(offsetInVBO * vertexSize + 4,          asuint(vertWorldPos.y));
            VertexBuffer.Store(offsetInVBO * vertexSize + 8,          asuint(vertWorldPos.z));
            VertexBuffer.Store(offsetInVBO * vertexSize + 12,         asuint(vertexColorAsUint));
            VertexBuffer.Store(offsetInVBO * vertexSize + 16,         asuint(bottomLeftUV.x + currentVert.uv.x * uvSize.x));
            VertexBuffer.Store(offsetInVBO * vertexSize + 20,         asuint(bottomLeftUV.y + currentVert.uv.y * uvSize.y));
            VertexBuffer.Store(offsetInVBO * vertexSize + 24,         asuint(emitterInstance.definitionIndex));
            VertexBuffer.Store(offsetInVBO * vertexSize + 28,         asuint(currentVert.uv.x));
            VertexBuffer.Store(offsetInVBO * vertexSize + 32,         asuint(currentVert.uv.y));
            VertexBuffer.Store(offsetInVBO * vertexSize + 36,         asuint(emissive));
            VertexBuffer.Store(offsetInVBO * vertexSize + 40,         asuint(alpha));
            VertexBuffer.Store(offsetInVBO * vertexSize + 44,         asuint(panTextureContribution));
            VertexBuffer.Store(offsetInVBO * vertexSize + 48,         asuint(aliveTime));
            VertexBuffer.Store(offsetInVBO * vertexSize + 52,         asuint(vertWorldNormal.x));
            VertexBuffer.Store(offsetInVBO * vertexSize + 56,         asuint(vertWorldNormal.y));
            VertexBuffer.Store(offsetInVBO * vertexSize + 60,         asuint(vertWorldNormal.z));
        }
    }
}