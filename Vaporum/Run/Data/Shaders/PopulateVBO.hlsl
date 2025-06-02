#include "ParticleCommon.h"

RWByteAddressBuffer VertexBuffer : register(u0);

StructuredBuffer<ParticleEmitterInstanceGPU> emitterInstances : register(t0);
StructuredBuffer<ParticleEmitterUpdateDefinitionGPU> updateDefs : register(t1);
StructuredBuffer<Particle> particleBuffer : register(t2);
StructuredBuffer<uint> drawList : register(t3);
ByteAddressBuffer counterBuffer : register(t4);
StructuredBuffer<FloatGraph> floatGraphs : register(t5);
StructuredBuffer<Float2Graph> float2Graphs : register(t6);
StructuredBuffer<Float3Graph> float3Graphs : register(t7);

[numthreads(64, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	if (DTid.x >= c_maxParticles || DTid.x >= counterBuffer.Load(PARTICLECOUNTER_OFFSET_DRAWCOUNT))
	{
		return;
	}
    int particleIndex = drawList[DTid.x];
    Particle particle = particleBuffer[particleIndex];

    ParticleEmitterInstanceGPU emitterInstance = emitterInstances[particle.configIndex];
	ParticleEmitterUpdateDefinitionGPU config;
	config = updateDefs[emitterInstance.definitionIndex];

    float3 particleCenterPos;
    if (config.worldSimulation == 1)
    {
        particleCenterPos = particle.position;
    }
    else
    {
       particleCenterPos = (float3)mul(emitterInstance.localToWorldMatrix, float4(particle.position, 1.f));
    } 
    float3 fullFacingIBasis = normalize(c_playerPosition - particleCenterPos);
    float3 fullFacingJBasis;
    float3 fullFacingKBasis;
    
    if ( abs( dot(fullFacingIBasis, float3(0.f, 0.f, 1.f)) ) != 1.f)
    {
        fullFacingJBasis = normalize( cross(float3(0.f, 0.f, 1.f), fullFacingIBasis) );
        fullFacingKBasis = normalize( cross(fullFacingIBasis, fullFacingJBasis) );
    }
    else
    {
        fullFacingJBasis = normalize( cross(float3(0.f, 1.f, 0.f), fullFacingIBasis) );
        fullFacingKBasis = normalize( cross(fullFacingIBasis, fullFacingJBasis) );
    }
    float3 totalVelocity = particle.velocity + particle.lifetimeVelocity;
    if (config.orientToVelocity == 1)
    {
        if (length(totalVelocity) > .01)
        {
            float3 velocityDir = normalize( (float3)mul( emitterInstance.localToWorldMatrix, float4(totalVelocity, 0.f) ) );
            float vJ = dot(velocityDir, fullFacingJBasis);
            float vK = dot(velocityDir, fullFacingKBasis);
            float theta = atan2(vK, vJ) + PI / 2.f;
            float2 newJBasisInOldJK = float2(cos(theta), sin(theta));
            float2 newKBasisInOldJK = float2(-newJBasisInOldJK.y, newJBasisInOldJK.x);
            float3 oldJBasis = fullFacingJBasis;
            float3 oldKBasis = fullFacingKBasis;
            fullFacingJBasis = newJBasisInOldJK.x * oldJBasis + newJBasisInOldJK.y * oldKBasis;
            fullFacingKBasis = newKBasisInOldJK.x * oldJBasis + newKBasisInOldJK.y * oldKBasis;
        }
    }
    float4 color = config.colorOverLifetime[0].color;
    float normalizedLifePos =  particle.liveTime / Get1dNoiseInRange(config.lifetime.x, config.lifetime.y, 
    0, particleIndex + SEED_OFFSET_LIFETIME);
    normalizedLifePos = 1.f - normalizedLifePos;
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

    float2 size = GetValueInFloat2Graph(float2Graphs, FLOAT2GRAPH_SIZE, 
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_SIZE, false);
    float uniformScale = abs(emitterInstance.localToWorldMatrix._m00);
    size *= uniformScale;
    
    /*
    float emissive = 1.f;
    float alpha = 0.f;
    float panTextureContribution = 0.f;
    float aliveTime = 0.f;
    */

    
    float emissive = GetValueInFloatGraph(floatGraphs, FLOATGRAPH_EMISSIVE, 
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_EMISSIVE);

    float alpha = GetValueInFloatGraph(floatGraphs, FLOATGRAPH_ALPHA_OBSCURANCE,
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_ALPHA_OBSCURANCE);

    float panTextureContribution = GetValueInFloatGraph(floatGraphs, FLOATGRAPH_PAN_TEXTURE_CONTRIBUTION,
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_PAN_TEXTURE_CONTRIBUTION);

    float maxLife = Get1dNoiseInRange(config.lifetime.x, config.lifetime.y, 
    0, SEED_OFFSET_LIFETIME);
    float aliveTime = maxLife - particle.liveTime;

    float3 bottomLeft;
    float3 bottomRight;
    float3 topRight;
    float3 topLeft;
    
    if (config.stretchBillboard == 0)
    {
        bottomLeft = particleCenterPos - .5f * size.x * fullFacingJBasis - .5f * size.y * fullFacingKBasis;
        bottomRight = particleCenterPos + .5f * size.x * fullFacingJBasis - .5f * size.y * fullFacingKBasis;
        topRight = particleCenterPos + .5f * size.x * fullFacingJBasis + .5f * size.y * fullFacingKBasis;
        topLeft = particleCenterPos - .5f * size.x * fullFacingJBasis + .5f * size.y * fullFacingKBasis;
    }
    else
    {
        float kBasisAlignment = dot(fullFacingKBasis, totalVelocity);
        float jBasisAlignment = dot(fullFacingJBasis, totalVelocity);
        if (abs(kBasisAlignment) > abs(jBasisAlignment))
        {
            kBasisAlignment *= config.lengthPerSpeed * uniformScale;
            if (kBasisAlignment > 0)
            {
                bottomLeft = particleCenterPos - .5f * size.x * fullFacingJBasis - (.5f * size.y + kBasisAlignment) * fullFacingKBasis;
                bottomRight = particleCenterPos + .5f * size.x * fullFacingJBasis - (.5f * size.y + kBasisAlignment) * fullFacingKBasis;
                topRight = particleCenterPos + .5f * size.x * fullFacingJBasis + .5f * size.y * fullFacingKBasis;
                topLeft = particleCenterPos - .5f * size.x * fullFacingJBasis + .5f * size.y * fullFacingKBasis;
            }
            else
            {
                topRight = particleCenterPos + .5f * size.x * fullFacingJBasis + (.5f * size.y + abs(kBasisAlignment)) * fullFacingKBasis;
                topLeft = particleCenterPos - .5f * size.x * fullFacingJBasis + (.5f * size.y + abs(kBasisAlignment)) * fullFacingKBasis;
                bottomLeft = particleCenterPos - .5f * size.x * fullFacingJBasis - .5f * size.y * fullFacingKBasis;
                bottomRight = particleCenterPos + .5f * size.x * fullFacingJBasis - .5f * size.y * fullFacingKBasis;
            }
        }
        else
        {
            jBasisAlignment *= config.lengthPerSpeed * uniformScale;
            if (jBasisAlignment > 0)
            {
                bottomLeft = particleCenterPos - ((.5f * size.x + jBasisAlignment) * fullFacingJBasis) - .5f * size.y * fullFacingKBasis;
                bottomRight = particleCenterPos + .5f * size.x * fullFacingJBasis - .5f * size.y * fullFacingKBasis;
                topRight = particleCenterPos + .5f * size.x * fullFacingJBasis + .5f * size.y * fullFacingKBasis;
                topLeft = particleCenterPos - ((.5f * size.x + jBasisAlignment) * fullFacingJBasis) + .5f * size.y * fullFacingKBasis;
            }
            else
            {
                topRight = particleCenterPos + (.5f * size.x + abs(jBasisAlignment)) * fullFacingJBasis + .5f * size.y * fullFacingKBasis;
                bottomRight = particleCenterPos + (.5f * size.x + abs(jBasisAlignment)) * fullFacingJBasis - .5f * size.y * fullFacingKBasis;
                topLeft = particleCenterPos - .5f * size.x * fullFacingJBasis + .5f * size.y * fullFacingKBasis;
                bottomLeft = particleCenterPos - .5f * size.x * fullFacingJBasis - .5f * size.y * fullFacingKBasis;
            }
        }
    }

   
    int vertsPerParticle = 6;
    int vertexSize = 52;
    
    VertexBuffer.Store(DTid.x * vertsPerParticle * vertexSize,              asuint(bottomLeft.x));
    VertexBuffer.Store(DTid.x * vertsPerParticle * vertexSize + 4,          asuint(bottomLeft.y));
    VertexBuffer.Store(DTid.x * vertsPerParticle * vertexSize + 8,          asuint(bottomLeft.z));
    VertexBuffer.Store(DTid.x * vertsPerParticle * vertexSize + 12,         asuint(vertexColorAsUint));
    VertexBuffer.Store(DTid.x * vertsPerParticle * vertexSize + 16,         asuint(particleBuffer[particleIndex].uvBottomLeft.x));
    VertexBuffer.Store(DTid.x * vertsPerParticle * vertexSize + 20,         asuint(particleBuffer[particleIndex].uvBottomLeft.y));
    VertexBuffer.Store(DTid.x * vertsPerParticle * vertexSize + 24,         asuint(emitterInstance.definitionIndex));
    VertexBuffer.Store(DTid.x * vertsPerParticle * vertexSize + 28,         asuint(0.f)); //pack that data 
    VertexBuffer.Store(DTid.x * vertsPerParticle * vertexSize + 32,         asuint(0.f));
    VertexBuffer.Store(DTid.x * vertsPerParticle * vertexSize + 36,         asuint(emissive));
    VertexBuffer.Store(DTid.x * vertsPerParticle * vertexSize + 40,         asuint(alpha));
    VertexBuffer.Store(DTid.x * vertsPerParticle * vertexSize + 44,         asuint(panTextureContribution));
    VertexBuffer.Store(DTid.x * vertsPerParticle * vertexSize + 48,         asuint(aliveTime));

    VertexBuffer.Store((DTid.x * vertsPerParticle + 1) * vertexSize,        asuint(bottomRight.x));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 1) * vertexSize + 4,    asuint(bottomRight.y));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 1) * vertexSize + 8,    asuint(bottomRight.z));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 1) * vertexSize + 12,   asuint(vertexColorAsUint));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 1) * vertexSize + 16,   asuint(particleBuffer[particleIndex].uvTopRight.x));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 1) * vertexSize + 20,   asuint(particleBuffer[particleIndex].uvBottomLeft.y));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 1) * vertexSize + 24,   asuint(emitterInstance.definitionIndex));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 1) * vertexSize + 28,   asuint(1.f));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 1) * vertexSize + 32,   asuint(0.f));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 1) * vertexSize + 36,   asuint(emissive));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 1) * vertexSize + 40,   asuint(alpha));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 1) * vertexSize + 44,   asuint(panTextureContribution));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 1) * vertexSize + 48,   asuint(aliveTime));

    VertexBuffer.Store((DTid.x * vertsPerParticle + 2) * vertexSize,        asuint(topRight.x));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 2) * vertexSize + 4,    asuint(topRight.y));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 2) * vertexSize + 8,    asuint(topRight.z));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 2) * vertexSize + 12,   asuint(vertexColorAsUint));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 2) * vertexSize + 16,   asuint(particleBuffer[particleIndex].uvTopRight.x));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 2) * vertexSize + 20,   asuint(particleBuffer[particleIndex].uvTopRight.y));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 2) * vertexSize + 24,   asuint(emitterInstance.definitionIndex));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 2) * vertexSize + 28,   asuint(1.f));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 2) * vertexSize + 32,   asuint(1.f));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 2) * vertexSize + 36,   asuint(emissive));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 2) * vertexSize + 40,   asuint(alpha));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 2) * vertexSize + 44,   asuint(panTextureContribution));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 2) * vertexSize + 48,   asuint(aliveTime));

    VertexBuffer.Store((DTid.x * vertsPerParticle + 3) * vertexSize,        asuint(bottomLeft.x));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 3) * vertexSize + 4,    asuint(bottomLeft.y));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 3) * vertexSize + 8,    asuint(bottomLeft.z));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 3) * vertexSize + 12,   asuint(vertexColorAsUint));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 3) * vertexSize + 16,   asuint(particleBuffer[particleIndex].uvBottomLeft.x));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 3) * vertexSize + 20,   asuint(particleBuffer[particleIndex].uvBottomLeft.y));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 3) * vertexSize + 24,   asuint(emitterInstance.definitionIndex));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 3) * vertexSize + 28,   asuint(0.f));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 3) * vertexSize + 32,   asuint(0.f));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 3) * vertexSize + 36,   asuint(emissive));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 3) * vertexSize + 40,   asuint(alpha));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 3) * vertexSize + 44,   asuint(panTextureContribution));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 3) * vertexSize + 48,   asuint(aliveTime));

    VertexBuffer.Store((DTid.x * vertsPerParticle + 4) * vertexSize,        asuint(topRight.x));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 4) * vertexSize + 4,    asuint(topRight.y));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 4) * vertexSize + 8,    asuint(topRight.z));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 4) * vertexSize + 12,   asuint(vertexColorAsUint));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 4) * vertexSize + 16,   asuint(particleBuffer[particleIndex].uvTopRight.x));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 4) * vertexSize + 20,   asuint(particleBuffer[particleIndex].uvTopRight.y));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 4) * vertexSize + 24,   asuint(emitterInstance.definitionIndex));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 4) * vertexSize + 28,   asuint(1.f));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 4) * vertexSize + 32,   asuint(1.f));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 4) * vertexSize + 36,   asuint(emissive));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 4) * vertexSize + 40,   asuint(alpha));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 4) * vertexSize + 44,   asuint(panTextureContribution));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 4) * vertexSize + 48,   asuint(aliveTime));

    VertexBuffer.Store((DTid.x * vertsPerParticle + 5) * vertexSize,        asuint(topLeft.x));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 5) * vertexSize + 4,    asuint(topLeft.y));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 5) * vertexSize + 8,    asuint(topLeft.z));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 5) * vertexSize + 12,   asuint(vertexColorAsUint));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 5) * vertexSize + 16,   asuint(particleBuffer[particleIndex].uvBottomLeft.x));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 5) * vertexSize + 20,   asuint(particleBuffer[particleIndex].uvTopRight.y));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 5) * vertexSize + 24,   asuint(emitterInstance.definitionIndex));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 5) * vertexSize + 28,   asuint(0.f));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 5) * vertexSize + 32,   asuint(1.f));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 5) * vertexSize + 36,   asuint(emissive));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 5) * vertexSize + 40,   asuint(alpha));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 5) * vertexSize + 44,   asuint(panTextureContribution));
    VertexBuffer.Store((DTid.x * vertsPerParticle + 5) * vertexSize + 48,   asuint(aliveTime));
}
