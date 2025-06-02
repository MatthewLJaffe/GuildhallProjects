#include "ParticleCommon.h"


RWByteAddressBuffer counterBuffer : register(u0);

[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    counterBuffer.Store(PARTICLECOUNTER_OFFSET_ALIVECOUNT, asuint(counterBuffer.Load(PARTICLECOUNTER_OFFSET_ALIVECOUNT_AFTERSIMULATION)));
    counterBuffer.Store(PARTICLECOUNTER_OFFSET_ALIVECOUNT_AFTERSIMULATION, asuint(0));
    counterBuffer.Store(PARTICLECOUNTER_OFFSET_DRAWCOUNT, asuint(0));

}