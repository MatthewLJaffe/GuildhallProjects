#pragma once
#include "ParticleCommon.h"


RWStructuredBuffer<uint> deadList : register(u0);

[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    deadList[DTid.x] = MAX_PARTICLES_PER_EMITTER - DTid.x;
}