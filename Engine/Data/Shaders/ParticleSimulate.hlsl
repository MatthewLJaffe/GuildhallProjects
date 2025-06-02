#include "ParticleCommon.h"

StructuredBuffer<uint> aliveList1 : register(t0);
StructuredBuffer<ParticleEmitterInstanceGPU> emitterInstances : register(t1);
StructuredBuffer<ParticleEmitterUpdateDefinitionGPU> updateDefs : register(t2);
StructuredBuffer<ParticlePhysicsObjectGPU> physicsObjects : register(t3);
StructuredBuffer<FloatGraph> floatGraphs : register(t4);
StructuredBuffer<Float2Graph> float2Graphs : register(t5);
StructuredBuffer<Float3Graph> float3Graphs : register(t6);
StructuredBuffer<ParticlePhysicsAABB3GPU> physicsAABB3s : register(t7);
Texture2D<float> depthTexture : register(t8);

RWStructuredBuffer<Particle> particleBuffer : register(u0);
RWStructuredBuffer<uint> aliveList2 : register(u1);
RWStructuredBuffer<uint> deadList : register(u2);
RWByteAddressBuffer counterBuffer : register(u3);
RWStructuredBuffer<float> particleDistanceBuffer : register(u4);
RWStructuredBuffer<uint> particleDrawList : register(u5);
RWStructuredBuffer<float> meshParticleDistanceBuffer : register(u6);
RWStructuredBuffer<uint> meshParticleDrawList : register(u7);

float3 SampleCurlNoise(float3 position, float delta, float curlNoiseScale, int noiseOctives)
{
    float span = 2.f*delta;
    float dXy =  (Compute3dPerlinNoise(position.x + delta, position.y, position.z, curlNoiseScale, noiseOctives, .5f, 2.f, true, 1) - 
                  Compute3dPerlinNoise(position.x - delta, position.y, position.z, curlNoiseScale, noiseOctives, .5f, 2.f, true, 1)) / span;
    float dXz =  (Compute3dPerlinNoise(position.x + delta, position.y, position.z, curlNoiseScale, noiseOctives, .5f, 2.f, true, 2) - 
                  Compute3dPerlinNoise(position.x - delta, position.y, position.z, curlNoiseScale, noiseOctives, .5f, 2.f, true, 2)) / span;
    float dYx = (Compute3dPerlinNoise(position.x, position.y + delta, position.z, curlNoiseScale, noiseOctives, .5f, 2.f, true, 0) - 
                 Compute3dPerlinNoise(position.x, position.y - delta, position.z, curlNoiseScale, noiseOctives, .5f, 2.f, true, 0)) / span;
    float dYz = (Compute3dPerlinNoise(position.x, position.y + delta, position.z, curlNoiseScale, noiseOctives, .5f, 2.f, true, 2) - 
                 Compute3dPerlinNoise(position.x, position.y - delta, position.z, curlNoiseScale, noiseOctives, .5f, 2.f, true, 2)) / span;
    float dZx = (Compute3dPerlinNoise(position.x, position.y, position.z + delta, curlNoiseScale, noiseOctives, .5f, 2.f, true, 0) - 
                 Compute3dPerlinNoise(position.x, position.y, position.z - delta, curlNoiseScale, noiseOctives, .5f, 2.f, true, 0)) / span;
    float dZy = (Compute3dPerlinNoise(position.x, position.y, position.z + delta, curlNoiseScale, noiseOctives, .5f, 2.f, true, 1) - 
                 Compute3dPerlinNoise(position.x, position.y, position.z - delta, curlNoiseScale, noiseOctives, .5f, 2.f, true, 1)) / span;
    float3 curl = float3(dYz - dZy, dZx - dXz, dXy - dYx);
    curl = normalize(curl);
    return curl;
}

float3 PushPointOutOfDepthBuffer(float3 position)
{
    float3 outPutDisp = float3(0.f, 0.f, 0.f);
    float4x4 viewProjMatrix = mul(ProjectionMatrix, ViewMatrix);
    float4 pos2D = mul(viewProjMatrix, float4(position, 1.f));
    pos2D.xyz /= pos2D.w;
    if (pos2D.x > -1 && pos2D.x < 1 && pos2D.y > -1 && pos2D.y < 1)
    {
        float2 uv = pos2D.xy * float2(.5f, -.5f) + float2(.5f, .5f);
        int2 pixelCoords = (int2)(uv * c_windowDimensions);
        float depth0 = depthTexture.Load(int3(pixelCoords, 0));
        float surfaceThickness = 1.f;
        float surfaceLinearDepth = computeLinearDepth(depth0);

        // check if position is inside surface
        if ( pos2D.w > surfaceLinearDepth && pos2D.w < surfaceLinearDepth + surfaceThickness)
        {       
            float uvPixelOffsetX = 1.f / (float)c_windowDimensions.x;
            float uvPixelOffsetY = 1.f / (float)c_windowDimensions.y;
            float2 uv1 = uv + float2(uvPixelOffsetX, 0.f);
            float2 uv2 = uv + float2(0.f, -uvPixelOffsetY);
            float2 pixelCoords1 = pixelCoords + int2(1, 0);
            float2 pixelCoords2 = pixelCoords + int2(0, -1);
            float depth1 = depthTexture.Load(int3(pixelCoords1, 0));
            float depth2 = depthTexture.Load(int3(pixelCoords2, 0));
                
            //reconstruct surface normal and resolve collision
            float3 p0 = reconstruct_position(uv, depth0, c_inverseViewProjMatrix);
		    float3 p1 = reconstruct_position(uv1, depth1, c_inverseViewProjMatrix);
		    float3 p2 = reconstruct_position(uv2, depth2, c_inverseViewProjMatrix);
            float3 normalRight = p1 - p0;
            float3 normalUp = p2 - p0;
            float3 surfaceNormal = normalize(cross(normalRight, normalUp));

            //push particle out of wall
            float3 dispFromPointToSurface = p0 - position;
            float distanceInsideSurface = dot(dispFromPointToSurface, surfaceNormal);
            outPutDisp = distanceInsideSurface * surfaceNormal;
        }
        //check if position needs to stay behind surface
        else if (pos2D.w > surfaceLinearDepth + surfaceThickness && pos2D.w < surfaceLinearDepth + 2*surfaceThickness)
        {
            float uvPixelOffsetX = 1.f / (float)c_windowDimensions.x;
            float uvPixelOffsetY = 1.f / (float)c_windowDimensions.y;
            float2 uv1 = uv + float2(uvPixelOffsetX, 0.f);
            float2 uv2 = uv + float2(0.f, -uvPixelOffsetY);
            float2 pixelCoords1 = pixelCoords + int2(1, 0);
            float2 pixelCoords2 = pixelCoords + int2(0, -1);
            float depth1 = depthTexture.Load(int3(pixelCoords1, 0));
            float depth2 = depthTexture.Load(int3(pixelCoords2, 0));
                
            //reconstruct surface normal and resolve collision
            float3 p0 = reconstruct_position(uv, depth0, c_inverseViewProjMatrix);
		    float3 p1 = reconstruct_position(uv1, depth1, c_inverseViewProjMatrix);
		    float3 p2 = reconstruct_position(uv2, depth2, c_inverseViewProjMatrix);
            float3 normalRight = p1 - p0;
            float3 normalUp = p2 - p0;
            float3 surfaceNormal = -normalize(cross(normalRight, normalUp));

            //pos to push out of should be on edge of "other side" of wall
            float3 posToPushOutOf = p0 + 2.f * surfaceThickness * surfaceNormal;
            float3 dispFromPointToSurface = posToPushOutOf - position;
            float distanceInsideSurface = dot(dispFromPointToSurface, surfaceNormal);
            outPutDisp = distanceInsideSurface * surfaceNormal;
        }
    }
    return outPutDisp;
}

[numthreads(64, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	if (DTid.x >= c_maxParticles || DTid.x >= counterBuffer.Load(PARTICLECOUNTER_OFFSET_ALIVECOUNT))
	{
		return;
	}
	uint particleIndex = aliveList1[DTid.x];
	Particle particle = particleBuffer[particleIndex];

	ParticleEmitterInstanceGPU emitterInstance = emitterInstances[particle.configIndex];
	ParticleEmitterUpdateDefinitionGPU config;
	config = updateDefs[emitterInstance.definitionIndex];


	particle.liveTime -= c_deltaSeconds;
	if (particle.liveTime < 0 || emitterInstance.killParticles == 1)
	{
		// kill:
		uint deadIndex;
		counterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_DEADCOUNT, 1, deadIndex);
		deadList[deadIndex] = particleIndex;
		return;
	}

    float normalizedLifePos =  particle.liveTime / Get1dNoiseInRange(config.lifetime.x, config.lifetime.y, 
    0, particleIndex + SEED_OFFSET_LIFETIME);
    normalizedLifePos = 1.f - normalizedLifePos;
    
    if (config.setLifetimePosition == 1)
    {
        particle.position = GetValueInFloat3Graph(float3Graphs, FLOAT3GRAPH_LIFETIME_POSITION, 
        emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_LIFETIME_POSITION);
    }
    else if (config.setLifetimeVelocity == 1)
    {
        if (config.velocityMode == 0)
        {
            particle.lifetimeVelocity = GetValueInFloat3Graph(float3Graphs, FLOAT3GRAPH_LIFETIME_VELOCITY, emitterInstance.definitionIndex,
            normalizedLifePos, particleIndex + SEED_OFFSET_LIFETIME_VELOCITY);
        }
        else if (config.velocityMode == 1)
        {
            float radialVelocity = GetValueInFloatGraph(floatGraphs, FLOATGRAPH_RADIAL_VELOCITY, emitterInstance.definitionIndex,
            normalizedLifePos, particleIndex + SEED_OFFSET_RADIAL_VELOCITY);
            if (particle.position.x != 0.f || particle.position.y != 0.f || particle.position.z != 0.f)
            {
                particle.lifetimeVelocity = normalize(particle.position) * radialVelocity;
            }
        }
    }
    float2 size = GetValueInFloat2Graph(float2Graphs, FLOAT2GRAPH_SIZE, 
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_SIZE, false);
    size *= emitterInstance.localToWorldMatrix._m33;
    float particleSize = size.x;
    if (size.y > size.x)
    {
        particleSize = size.y;
    }

    if (config.ignoreWorldPhysics == 0)
    {
        for (uint physIdx = 0; physIdx < c_numPhysicsObjects; physIdx++)
        {
            if (physicsObjects[physIdx].isActive == 0)
            {
                continue;
            }
            float3 localPhysicsPosition;
            if (config.worldSimulation == 1)
            {
                localPhysicsPosition = physicsObjects[physIdx].position;
            }
            else
            {
                localPhysicsPosition = (float3)mul( emitterInstance.worldToLocalMatrix, float4( physicsObjects[physIdx].position, 1.f ) );
            }
            float3 dispToPointForce = localPhysicsPosition - particle.position;
            float distSquared = dot(dispToPointForce, dispToPointForce);
            if (distSquared < physicsObjects[physIdx].radius * physicsObjects[physIdx].radius)
            {
                float distanceAsFraction = 1.f - length(dispToPointForce) / physicsObjects[physIdx].radius;
                float forceMultiplier = pow(distanceAsFraction, physicsObjects[physIdx].falloffExponent);
                if (physicsObjects[physIdx].attract == 1)
                {
                    particle.velocity += 
                    physicsObjects[physIdx].forceMagnitude * forceMultiplier * c_deltaSeconds * normalize(dispToPointForce);
                }
                else
                {
                    particle.velocity += 
                    physicsObjects[physIdx].forceMagnitude * forceMultiplier * c_deltaSeconds * -normalize(dispToPointForce);
                }
            }
        }
    }


    float3 linearForce = GetValueInFloat3Graph(float3Graphs, FLOAT3GRAPH_LINEAR_FORCE, emitterInstance.definitionIndex,
    normalizedLifePos, particleIndex + SEED_OFFSET_LINEAR_FORCE);

    particle.velocity += linearForce * c_deltaSeconds;
    
    if (config.perlinNoiseForce > 0.f)
    {
        float3 perlinNoiseDirection = float3(0.f, 0.f, 0.f);
        perlinNoiseDirection.x = Compute3dPerlinNoise(particle.position.x, particle.position.y, particle.position.z, 40.f, 7, .5f, 2.f, true, 0);
        perlinNoiseDirection.y = Compute3dPerlinNoise(particle.position.x, particle.position.y, particle.position.z, 40.f, 7, .5f, 2.f, true, 1);
        perlinNoiseDirection.z = Compute3dPerlinNoise(particle.position.x, particle.position.y, particle.position.z, 40.f, 7, .5f, 2.f, true, 2);
        perlinNoiseDirection = normalize(perlinNoiseDirection);
        particle.velocity += perlinNoiseDirection * config.perlinNoiseForce * c_deltaSeconds;
    }
    if (config.curlNoiseAffectPosition == 0)
    {
        float curlNoiseForce = GetValueInFloatGraph(floatGraphs, FLOATGRAPH_CURL_NOISE_FORCE, 
        emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_CURL_NOISE_FORCE);
        if (curlNoiseForce > 0.f)
        {
            float3 curlNoisePos = particle.position + config.curlNoisePan * c_timeElapsed;
            float3 curlForce = SampleCurlNoise(curlNoisePos, config.curlNoiseSampleSize, config.curlNoiseScale, config.curlNoiseOctives) * curlNoiseForce;
            particle.velocity += curlForce * c_deltaSeconds;
        }
    }


    float pointForce = GetValueInFloatGraph(floatGraphs, FLOATGRAPH_POINT_FORCE, 
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_POINT_FORCE);
    if (pointForce > 0.f && config.pointForceRadius > 0.f)
    {
        float3 dispToPointForce = config.pointForcePosition - particle.position;
        float distSquared = dot(dispToPointForce, dispToPointForce);
        if (distSquared < config.pointForceRadius * config.pointForceRadius)
        {
            float distanceAsFraction = 1.f - length(dispToPointForce) / config.pointForceRadius;
            float forceMultiplier = pow(distanceAsFraction, config.pointForceFalloffExponent);
            if (config.pointForceAttract == 1)
            {
                particle.velocity += pointForce * forceMultiplier * c_deltaSeconds * normalize(dispToPointForce);
            }
            else
            {
                particle.velocity += pointForce * forceMultiplier * c_deltaSeconds * -normalize(dispToPointForce);
            }
        }
    }
    
    float vortexForce = GetValueInFloatGraph(floatGraphs, FLOATGRAPH_VORTEX_FORCE, 
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_VORTEX_FORCE);
    if (vortexForce > 0.f)
    {
        float3 dispFromOrigin = particle.position - config.vortexAxisOrigin;
        float3 dispInAxisDir = config.vortexAxisDir * dot(config.vortexAxisDir, dispFromOrigin);
        float3 dispNotInAxisDir = dispFromOrigin - dispInAxisDir;
        //check if particle is inside vortex radius
        if (dot(dispNotInAxisDir, dispNotInAxisDir) < config.vortexForceRadius * config.vortexForceRadius)
        {
            float3 vortexForceDir = normalize( cross(config.vortexAxisDir, dispNotInAxisDir) );
            particle.velocity += vortexForce * vortexForceDir * c_deltaSeconds;
        }
    }
    if (config.returnToOriginForce > 0.f)
    {
        float3 dispToEmissionPos = particle.emissionPosition - particle.position;
        float distSquaredToEmissionPos = dot(dispToEmissionPos, dispToEmissionPos);

        //not at emission pos
        if (dot(dispToEmissionPos, dispToEmissionPos) > .0001f)
        {
            //delay return
            if (particle.currentDelayReturnTime > 0.f)
            {
                particle.currentDelayReturnTime -= c_deltaSeconds;
            }
            else
            {
                float ditanceToMoveThisFrame = config.returnToOriginForce * c_deltaSeconds;
                //particle is almost back so just set it to be back
                if (distSquaredToEmissionPos < (ditanceToMoveThisFrame * ditanceToMoveThisFrame))
                {
                    particle.currentDelayReturnTime = config.returnToOriginDelay;
                    particle.position =  particle.emissionPosition;
                    particle.velocity = float3(0.f, 0.f, 0.f);
                }
                //need to apply force
                else
                {
                    particle.velocity += normalize(dispToEmissionPos) * config.returnToOriginForce * c_deltaSeconds;
                }
            }
        }
    }
    float dragForce = GetValueInFloatGraph(floatGraphs, FLOATGRAPH_DRAG_FORCE, 
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_DRAG_FORCE);
    if (dragForce > 0.f)
    {
        float3 prevVelocity = particle.velocity;
        if (prevVelocity.x != 0.f || prevVelocity.y != 0.f || prevVelocity.z != 0.f)
        {
            particle.velocity -= normalize(particle.velocity) * dragForce * c_deltaSeconds;
            if (dot(prevVelocity, particle.velocity) <= 0.f)
            {
                particle.velocity = float3(0.f, 0.f, 0.f);
            }
        }
    }
    float maxSpeed = GetValueInFloatGraph(floatGraphs, FLOATGRAPH_MAX_SPEED, 
    emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_MAX_SPEED);

    if (dot(particle.velocity, particle.velocity) > maxSpeed * maxSpeed)
	{
		particle.velocity = normalize(particle.velocity) * maxSpeed;
	}

    float3 totalVelocity = particle.lifetimeVelocity + particle.velocity;
    if (particle.reflectionVector.x != 0.f && particle.reflectionVector.y != 0.f && particle.reflectionVector.z != 0.f)
    {
        totalVelocity = reflect(totalVelocity, particle.reflectionVector);
        totalVelocity *= .15f;
    }
    particle.position += totalVelocity * c_deltaSeconds;

    if (config.curlNoiseAffectPosition == 1)
    {
        float curlNoiseForce = GetValueInFloatGraph(floatGraphs, FLOATGRAPH_CURL_NOISE_FORCE, 
        emitterInstance.definitionIndex, normalizedLifePos, particleIndex + SEED_OFFSET_CURL_NOISE_FORCE);
        if (curlNoiseForce > 0.f)
        {
            float3 curlNoisePos = particle.position + config.curlNoisePan * c_timeElapsed;
            float3 curlForce = SampleCurlNoise(curlNoisePos, config.curlNoiseSampleSize, config.curlNoiseScale, config.curlNoiseOctives) * curlNoiseForce;
            particle.position += curlForce * c_deltaSeconds;
        }
    }

    float3 particleWorldPosition;
    if (config.worldSimulation == 1)
    {
        particleWorldPosition = particle.position;
    }
    else
    {
        particleWorldPosition = (float3)(mul(emitterInstance.localToWorldMatrix, float4(particle.position, 1.f)));
    }
    if (config.ignoreWorldPhysics == 0)
    {
        for (uint aabbIdx = 0; aabbIdx < c_numPhysicsAABB3s; aabbIdx++)
        {
            if (physicsAABB3s[aabbIdx].isActive == 0)
            {
                continue;
            }
            if (DoSphereAndAABBOverlap(particleWorldPosition, particleSize, physicsAABB3s[aabbIdx].mins, physicsAABB3s[aabbIdx].maxs))
            {
                float3 pointToPushOutOf = GetNearestPointOnAABB3D(particleWorldPosition, physicsAABB3s[aabbIdx].mins, physicsAABB3s[aabbIdx].maxs);
                float3 pushOutDir = normalize(particleWorldPosition - pointToPushOutOf);
                float3 pushOutDistance = particleSize - distance(particleWorldPosition, pointToPushOutOf);
                particleWorldPosition += pushOutDir * pushOutDistance;
            }
        }
        if (config.worldSimulation == 0)
        {
            particle.position = (float3)mul(emitterInstance.worldToLocalMatrix, float4(particleWorldPosition, 1.f));
        }
    }

    if (config.depthBufferCollisions == 1)
    {
        float3 outPutDisp = float3(0.f, 0.f, 0.f);
        float4x4 viewProjMatrix = mul(ProjectionMatrix, ViewMatrix);
        float4 pos2D = mul(viewProjMatrix, float4(particleWorldPosition, 1.f));
        pos2D.xyz /= pos2D.w;
        if (pos2D.x > -1 && pos2D.x < 1 && pos2D.y > -1 && pos2D.y < 1)
        {
            float2 uv = pos2D.xy * float2(.5f, -.5f) + float2(.5f, .5f);
            int2 pixelCoords = (int2)(uv * c_windowDimensions);
            float depth0 = depthTexture.Load(int3(pixelCoords, 0));
            float surfaceThickness = 1.f;
            float surfaceLinearDepth = computeLinearDepth(depth0);

            // check if position is inside surface
            if ( pos2D.w + particleSize*.5f > surfaceLinearDepth && pos2D.w + particleSize*.5f < surfaceLinearDepth + surfaceThickness)
            {       
                float uvPixelOffsetX = 1.f / (float)c_windowDimensions.x;
                float uvPixelOffsetY = 1.f / (float)c_windowDimensions.y;
                float2 uv1 = uv + float2(uvPixelOffsetX, 0.f);
                float2 uv2 = uv + float2(0.f, -uvPixelOffsetY);
                float2 pixelCoords1 = pixelCoords + int2(1, 0);
                float2 pixelCoords2 = pixelCoords + int2(0, -1);
                float depth1 = depthTexture.Load(int3(pixelCoords1, 0));
                float depth2 = depthTexture.Load(int3(pixelCoords2, 0));
                
                //reconstruct surface normal and resolve collision
                float3 p0 = reconstruct_position(uv, depth0, c_inverseViewProjMatrix);
		        float3 p1 = reconstruct_position(uv1, depth1, c_inverseViewProjMatrix);
		        float3 p2 = reconstruct_position(uv2, depth2, c_inverseViewProjMatrix);
                float3 normalRight = p1 - p0;
                float3 normalUp = p2 - p0;
                float3 surfaceNormal = normalize(cross(normalRight, normalUp));

                //reflect particle off wall
                if (config.worldSimulation == 1)
                {
                    particle.reflectionVector = surfaceNormal;
                }
                else
                {
                    particle.reflectionVector = (float3)mul(emitterInstance.worldToLocalMatrix, float4(surfaceNormal, 0.f));
                }
            }
            //check if position needs to stay behind surface
            else if (pos2D.w - particleSize*.5f > surfaceLinearDepth + surfaceThickness && pos2D.w - particleSize*.5f < surfaceLinearDepth + 2*surfaceThickness)
            {
                float uvPixelOffsetX = 1.f / (float)c_windowDimensions.x;
                float uvPixelOffsetY = 1.f / (float)c_windowDimensions.y;
                float2 uv1 = uv + float2(uvPixelOffsetX, 0.f);
                float2 uv2 = uv + float2(0.f, -uvPixelOffsetY);
                float2 pixelCoords1 = pixelCoords + int2(1, 0);
                float2 pixelCoords2 = pixelCoords + int2(0, -1);
                float depth1 = depthTexture.Load(int3(pixelCoords1, 0));
                float depth2 = depthTexture.Load(int3(pixelCoords2, 0));
                
                //reconstruct surface normal and resolve collision
                float3 p0 = reconstruct_position(uv, depth0, c_inverseViewProjMatrix);
		        float3 p1 = reconstruct_position(uv1, depth1, c_inverseViewProjMatrix);
		        float3 p2 = reconstruct_position(uv2, depth2, c_inverseViewProjMatrix);
                float3 normalRight = p1 - p0;
                float3 normalUp = p2 - p0;
                float3 surfaceNormal = -normalize(cross(normalRight, normalUp));

                //reflect particle off "other side" of the wall
                if (config.worldSimulation == 1)
                {
                    particle.reflectionVector = surfaceNormal;
                }
                else
                {
                    particle.reflectionVector = (float3)mul(emitterInstance.worldToLocalMatrix, float4(surfaceNormal, 0.f));
                }           
            }
        }
    }

    //calculate current index
    float spriteT = GetValueFromEasingFunciton(config.spriteEasingFunction, normalizedLifePos);
    uint currentIndex = (uint)floor(lerp((float)config.spriteStartIndex, 
    (float)config.spriteEndIndex + 1.f, spriteT));
    uint currentIndexX = currentIndex % config.spriteSheetdimensions.x;
    uint currentIndexY = currentIndex / config.spriteSheetdimensions.x;

    particle.uvBottomLeft.x = (float)currentIndexX / (float)config.spriteSheetdimensions.x;
	particle.uvBottomLeft.y = (float)((config.spriteSheetdimensions.y) - ( currentIndexY + 1)) / (float)(config.spriteSheetdimensions.y);
	particle.uvTopRight.x = (float)(currentIndexX + 1) / (float)config.spriteSheetdimensions.x;
	particle.uvTopRight.y = (float)(config.spriteSheetdimensions.y - currentIndexY) / (float)config.spriteSheetdimensions.y;
    float2 sheetUVDimensions = config.atlasUVMaxs - config.atlasUVMins;

    particle.uvBottomLeft.x = config.atlasUVMins.x + sheetUVDimensions.x * particle.uvBottomLeft.x;
    particle.uvBottomLeft.y = config.atlasUVMins.y + sheetUVDimensions.y * particle.uvBottomLeft.y;
    particle.uvTopRight.x = config.atlasUVMins.x + sheetUVDimensions.x * particle.uvTopRight.x;
    particle.uvTopRight.y = config.atlasUVMins.y + sheetUVDimensions.y * particle.uvTopRight.y;

	// write back simulated particle:
	particleBuffer[particleIndex] = particle;

	// add to new alive list:
	uint newAliveIndex;
	counterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_ALIVECOUNT_AFTERSIMULATION, 1, newAliveIndex);
	aliveList2[newAliveIndex] = particleIndex;

    //particle is billboarded quad
    if (config.particleMeshIndex == -1)
    {
        bool visible = IsParticleVisible(particleWorldPosition, size);
        if (visible)
        {
	        uint newDrawIndex;
            counterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_DRAWCOUNT, 1, newDrawIndex);
            particleDrawList[newDrawIndex] = particleIndex;

            float3 dispToParticle = particleWorldPosition - c_playerPosition;
            float distanceSquared = dot(dispToParticle, dispToParticle);
            particleDistanceBuffer[particleIndex] = -distanceSquared;
        }
    }

    //particle is mesh
    else
    {
	    uint newDrawIndex;
        counterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_MESH_DRAWCOUNT, 1, newDrawIndex);
        meshParticleDrawList[newDrawIndex] = particleIndex;

        float3 dispToParticle = particleWorldPosition - c_playerPosition;
        float distanceSquared = dot(dispToParticle, dispToParticle);
        meshParticleDistanceBuffer[particleIndex] = -distanceSquared;
    }
}
