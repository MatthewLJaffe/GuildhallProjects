#include "ParticleCommon.h"

RWByteAddressBuffer indirectBuffers : register(u0);

[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    // retrieve GPU itemcount:
	uint itemCount = min(c_numElements, c_maxParticles);

	if (itemCount > 0)
	{
		// calculate threadcount:
		uint threadCount = ((c_numElements - 1) >> 9) + 1;

		// and prepare to dispatch the sort for the alive simulated particles:
		indirectBuffers.Store3(0, uint3(threadCount, 1, 1));
	}
	else
	{
		// dispatch nothing:
		indirectBuffers.Store3(0, uint3(0, 0, 0));
	}
}