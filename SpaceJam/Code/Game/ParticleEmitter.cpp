#include "Game/ParticleEmitter.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/UAV.hpp"
#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/SRV.hpp"
#include "Engine/Renderer/IndirectArgsBuffer.hpp"
#include <d3d11.h>

ParticleEmitter::ParticleEmitter(ParticleEmitterConfig& config)
	: m_config(config)
{
	InitResources();
}

ParticleEmitter::~ParticleEmitter()
{
	delete m_sortConstantsCBO;
	delete m_particleConstantsCBO;
	delete m_particlesUAV;
	delete m_particlesSRV;
	delete m_particlesBuffer;
	delete m_particleAliveList1UAV;
	delete m_particleAliveList1Buffer;
	delete m_particleAliveList2UAV;
	delete m_particleAliveList2SRV;
	delete m_particleAliveList2Buffer;
	delete m_deadListUAV;
	delete m_deadListBuffer;
	delete m_counterUAV;
	delete m_counterSRV;
	delete m_counterBuffer;
	delete m_vbo;
	delete m_vboUAV;
	delete m_particleDistanceBuffer;
	delete m_particleDistanceUAV;
	delete m_particleDistanceSRV;
	delete m_particleDrawListBuffer;
	delete m_particleDrawListSRV;
	delete m_particleDrawListUAV;
}

void ParticleEmitter::InitResources()
{
	m_sortConstantsCBO = g_theRenderer->CreateConstantBuffer(sizeof(SortConstants));
	m_particleConstantsCBO = g_theRenderer->CreateConstantBuffer(sizeof(ParticleConstants));
	m_particlesBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(Particle), MAX_PARTICLES_PER_EMITTER, nullptr);
	m_particlesUAV = g_theRenderer->CreateUAV(m_particlesBuffer);
	m_particlesSRV = g_theRenderer->CreateSRV(m_particlesBuffer);

	CounterBuffer counterBuffer;
	m_counterBuffer = g_theRenderer->CreateRawBuffer(sizeof(CounterBuffer), (void *)&counterBuffer);
	m_counterUAV = g_theRenderer->CreateUAV(m_counterBuffer);
	m_counterSRV = g_theRenderer->CreateSRV(m_counterBuffer);
	unsigned int* aliveList1Buff = new unsigned int[MAX_PARTICLES_PER_EMITTER];
	unsigned int* aliveList2Buff = new unsigned int[MAX_PARTICLES_PER_EMITTER];
	unsigned int* deadListBuff = new unsigned int[MAX_PARTICLES_PER_EMITTER];
	unsigned int* drawListBuff = new unsigned int[MAX_PARTICLES_PER_EMITTER];
	for (unsigned int i = 0; i < MAX_PARTICLES_PER_EMITTER; i++)
	{
		aliveList1Buff[i] = 0;
		aliveList2Buff[i] = 0;
		deadListBuff[i] = MAX_PARTICLES_PER_EMITTER - (i + 1);
	}

	m_particleAliveList1Buffer = g_theRenderer->CreateStructuredBuffer(sizeof(unsigned int), MAX_PARTICLES_PER_EMITTER, (void *)aliveList1Buff);
	m_particleAliveList1UAV = g_theRenderer->CreateUAV(m_particleAliveList1Buffer);
	m_particleAliveList2Buffer = g_theRenderer->CreateStructuredBuffer(sizeof(unsigned int), MAX_PARTICLES_PER_EMITTER, (void*)aliveList2Buff);
	m_particleAliveList2UAV = g_theRenderer->CreateUAV(m_particleAliveList2Buffer);
	m_particleAliveList2SRV = g_theRenderer->CreateSRV(m_particleAliveList2Buffer);
	m_deadListBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(unsigned int), MAX_PARTICLES_PER_EMITTER, (void*)deadListBuff);
	m_deadListUAV = g_theRenderer->CreateUAV(m_deadListBuffer);
	m_vbo = g_theRenderer->CreateVertexBufferAsUAV(MAX_PARTICLES_PER_EMITTER * 6 * sizeof(Vertex_PCU));
	m_vboUAV = g_theRenderer->CreateUAV(m_vbo);
	m_particleDistanceBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(float), MAX_PARTICLES_PER_EMITTER, nullptr);
	m_particleDistanceUAV = g_theRenderer->CreateUAV(m_particleDistanceBuffer);
	m_particleDistanceSRV = g_theRenderer->CreateSRV(m_particleDistanceBuffer);
	m_particleDrawListBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(unsigned int), MAX_PARTICLES_PER_EMITTER, (void*)drawListBuff);
	m_particleDrawListUAV= g_theRenderer->CreateUAV(m_particleDrawListBuffer);
	m_particleDrawListSRV = g_theRenderer->CreateSRV(m_particleDrawListBuffer);

	delete[] aliveList1Buff;
	delete[] aliveList2Buff;
	delete[] deadListBuff;
	delete[] drawListBuff;
}

void ParticleEmitter::Update(float deltaSeconds)
{
	DetermineParticlesToEmit(deltaSeconds);
	EmitParticles(deltaSeconds);
	UpdateParticles(deltaSeconds);
}

Mat44 ParticleEmitter::GetModelMatrix()
{
	Mat44 m_modelMatrix;
	m_modelMatrix.AppendTranslation3D(m_config.m_position);
	m_modelMatrix.Append(m_config.m_orientationDegrees.GetAsMatrix_IFwd_JLeft_KUp());
	return m_modelMatrix;
}

void ParticleEmitter::DetermineParticlesToEmit(float deltaSeconds)
{
	float emissionForFrame = m_config.m_emissionRate * deltaSeconds;
	m_particlesToEmit = RoundDownToInt(emissionForFrame);
	m_owedParticlesAsFloat += emissionForFrame - (float)m_particlesToEmit;
	while (m_owedParticlesAsFloat > 1.f)
	{
		m_particlesToEmit++;
		m_owedParticlesAsFloat--;
	}
}

void ParticleEmitter::EmitParticles(float deltaSeconds)
{
	ParticleConstants particleConstants;
	particleConstants.m_deltaSeconds = deltaSeconds;
	particleConstants.maxSpeed = m_config.m_maxSpeed;
	particleConstants.m_acceleration = m_config.m_acceleration;
	particleConstants.m_emissionRadius = m_config.m_emissionRadius;
	particleConstants.m_emitterPosition = m_config.m_position;
	particleConstants.m_lifetime = m_config.m_lifetime;
	particleConstants.m_size = m_config.m_size;
	particleConstants.m_velocityXRange = m_config.m_velocityXRange;
	particleConstants.m_velocityYRange = m_config.m_velocityYRange;
	particleConstants.m_velocityZRange = m_config.m_velocityZRange;
	particleConstants.m_playerPosition = g_theGame->m_player->m_position;
	particleConstants.m_frameCount = m_frameCount;
	particleConstants.m_emittedParticles = (unsigned int)m_particlesToEmit;
	particleConstants.m_spriteSheetDimension[0] = m_config.m_spriteSheetDimension[0];
	particleConstants.m_spriteSheetDimension[1] = m_config.m_spriteSheetDimension[1];
	particleConstants.m_startIndex = m_config.m_spriteStartIndex;
	particleConstants.m_endIndex = m_config.m_spriteEndIndex;
	particleConstants.m_perlinNoiseForce = m_config.m_perlinNoiseForce;
	particleConstants.m_linearForce = m_config.m_linearForce;
	particleConstants.m_curlNoiseForce = m_config.m_curlNoiseForce;
	particleConstants.m_curlNoiseScale = m_config.m_curlNoiseScale;
	particleConstants.m_curlNoiseSampleSize = m_config.m_curlNoiseSampleSize;
	particleConstants.m_curlNoiseOctives = m_config.m_curlNoiseOctives;
	particleConstants.m_pointForceStrength = m_config.m_pointForceStrength;
	particleConstants.m_pointForcePosition.SetFromFloats(m_config.m_pointForcePosition);
	particleConstants.m_pointForceFalloffExponent = m_config.m_pointForceFalloffExponent;
	particleConstants.m_pointForceAttract = m_config.m_pointForceAttract ? 1 : 0;
	particleConstants.m_pointForceRadius = m_config.m_pointForceRadius;
	particleConstants.m_vortexAxisDir.x = m_config.m_vortexAxisDir[0];
	particleConstants.m_vortexAxisDir.y = m_config.m_vortexAxisDir[1];
	particleConstants.m_vortexAxisDir.z = m_config.m_vortexAxisDir[2];
	particleConstants.m_vortexAxisDir = particleConstants.m_vortexAxisDir.GetNormalized();
	particleConstants.m_vortexAxisOrigin.x = m_config.m_vortexAxisOrigin[0];
	particleConstants.m_vortexAxisOrigin.y = m_config.m_vortexAxisOrigin[1];
	particleConstants.m_vortexAxisOrigin.z = m_config.m_vortexAxisOrigin[2];
	particleConstants.m_vortexForce = m_config.m_vortexForce;
	particleConstants.m_vortexForceRadius = m_config.m_vortexForceRadius;
	particleConstants.m_dragForce = m_config.m_dragForce;


	if (m_config.m_sortParticles)
	{
		particleConstants.m_sortParticles = 1;
	}
	else
	{
		particleConstants.m_sortParticles = 0;
	}
	m_config.m_color.GetAsFloats(particleConstants.m_color);


	g_theRenderer->CopyCPUToGPU((void*)&particleConstants, m_particleConstantsCBO);
	g_theRenderer->BindConstantBuffer(k_particleConstantsSlot, m_particleConstantsCBO);
	g_theRenderer->SetCameraConstants(g_theGame->m_player->m_playerCamera);

	if (m_particlesToEmit == 0)
	{
		return;
	}

	ComputeShader* emitShader = g_theRenderer->CreateOrGetComputeShaderFromFile("Data/Shaders/ParticleEmit.hlsl");
	if (!emitShader)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get emit shader");
		return;
	}

	std::vector<SRV*> srvs;
	std::vector<UAV*> uavs;
	uavs.push_back(m_particlesUAV);
	uavs.push_back(m_particleAliveList1UAV);
	uavs.push_back(m_particleAliveList2UAV);
	uavs.push_back(m_deadListUAV);
	uavs.push_back(m_counterUAV);
	uavs.push_back(m_particleDrawListUAV);

	unsigned int threadGroupX = (unsigned int)ceil((double)m_particlesToEmit / 64.f);
	if (threadGroupX > D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION)
	{
		threadGroupX = D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
		ERROR_RECOVERABLE("Thread group exceded max size for emitting particles");
	}

	g_theRenderer->RunComputeShader(emitShader, srvs, uavs, threadGroupX, 1, 1);
}

void ParticleEmitter::UpdateParticles(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	int particlesToUpdate = m_particlesToEmit + m_aliveCountFromlastFrame;
	if (particlesToUpdate == 0)
	{
		return;
	}
	ComputeShader* simulateShader = g_theRenderer->CreateOrGetComputeShaderFromFile("Data/Shaders/ParticleSimulate.hlsl");
	if (!simulateShader)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get simulate shader");
		return;
	}
	std::vector<SRV*> srvs;
	std::vector<UAV*> uavs;
	uavs.push_back(m_particlesUAV);
	uavs.push_back(m_particleAliveList1UAV);
	uavs.push_back(m_particleAliveList2UAV);
	uavs.push_back(m_deadListUAV);
	uavs.push_back(m_counterUAV);
	uavs.push_back(m_vboUAV);
	uavs.push_back(m_particleDistanceUAV);
	uavs.push_back(m_particleDrawListUAV);

	unsigned int threadGroupX = (unsigned int)ceil((double)particlesToUpdate / 128.f);
	if (threadGroupX > D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION)
	{
		threadGroupX = D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
		ERROR_RECOVERABLE("Thread group exceded max size for updating particles");
	}
	g_theRenderer->RunComputeShader(simulateShader, srvs, uavs, threadGroupX, 1, 1);
	CounterBuffer* counterBuffer = (CounterBuffer*)g_theRenderer->ReadFromBuffer(m_counterBuffer);
	//this is how many particles need to be drawn
	m_aliveCountFromlastFrame = counterBuffer->aliveCountAfterSim;
	m_drawCount = counterBuffer->drawCount;
	m_culledCountLastFrame = m_aliveCountFromlastFrame - m_drawCount;
	SortParticlesGPU(counterBuffer->drawCount);
}


void ParticleEmitter::RenderAddVertsGPU()
{
	//double timeBeforeFunction = GetCurrentTimeSeconds();

	double timeBefore = GetCurrentTimeSeconds();
	DebugAddWorldBasis(GetModelMatrix(), 0.f, DebugRenderMode::ALWAYS);
	if (m_drawCount == 0)
	{
		return;
	}
	double timeDiff = GetCurrentTimeSeconds() - timeBefore;
	//DebuggerPrintf("Read from buffer time %.3f\n", timeDiff * 1000);
	if (m_config.m_sortParticles)
	{
		ComputeShader* addVertsShader = g_theRenderer->CreateOrGetComputeShaderFromFile("Data/Shaders/PopulateVBO.hlsl");
		if (!addVertsShader)
		{
			g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get populate vbo shader");
			return;
		}


		std::vector<SRV*> srvs;
		std::vector<UAV*> uavs;
		uavs.push_back(m_vboUAV);
		srvs.push_back(m_particlesSRV);
		srvs.push_back(m_particleDrawListSRV);
		srvs.push_back(m_counterSRV);

		unsigned int threadGroupX = (unsigned int)ceil((double)m_drawCount / 128.f);
		if (threadGroupX > D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION)
		{
			threadGroupX = D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
			ERROR_RECOVERABLE("Thread group exceded max size for updating particles");
		}
		g_theRenderer->RunComputeShader(addVertsShader, srvs, uavs, threadGroupX, 1, 1);
	}

	timeBefore = GetCurrentTimeSeconds();
	g_theRenderer->BindTexture(m_config.m_texture);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->DrawVertexBuffer(m_vbo, m_drawCount * 6);
	timeDiff = GetCurrentTimeSeconds() - timeBefore;
	//DebuggerPrintf("Draw verts time %.3f\n", timeDiff * 1000);

	//double timeDiffAfterFunction = GetCurrentTimeSeconds() - timeBeforeFunction;
	//DebuggerPrintf("Total render time %.3f\n", timeDiffAfterFunction * 1000);
}

void ParticleEmitter::EndFrame()
{
	//swap alive lists
	UAV* tempAliveList1UAV = m_particleAliveList1UAV;
	StructuredBuffer* tempAliveList1Buffer = m_particleAliveList1Buffer;

	m_particleAliveList1Buffer = m_particleAliveList2Buffer;
	m_particleAliveList1UAV = m_particleAliveList2UAV;

	m_particleAliveList2Buffer = tempAliveList1Buffer;
	m_particleAliveList2UAV = tempAliveList1UAV;

	ComputeShader* resetCounterShader = g_theRenderer->CreateOrGetComputeShaderFromFile("Data/Shaders/ResetCounter.hlsl");
	if (!resetCounterShader)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get reset counter shader");
		return;
	}

	//reset counter buffer
	std::vector<UAV*> uavs;
	std::vector<SRV*> srvs;
	uavs.push_back(m_counterUAV);
	g_theRenderer->RunComputeShader(resetCounterShader, srvs, uavs, 1, 1, 1);
	m_frameCount++;

	/*
	ID3D11Buffer* debugParticles = g_theRenderer->CreateAndCopyToDebugBuf(m_particlesBuffer);
	D3D11_MAPPED_SUBRESOURCE resource;
	g_theRenderer->MapBuffer(debugParticles, &resource);
	// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
	Particle* particles = (Particle*)resource.pData;
	g_theRenderer->UnmapBuffer(debugParticles);
	DX_SAFE_RELEASE(debugParticles);
	*/
}

void ParticleEmitter::SortParticlesGPU(int maxCount)
{
	//initialize sorting arguments
	ComputeShader* kickoffSortShader = g_theRenderer->CreateOrGetComputeShaderFromFile("Data/Shaders/KickoffSort.hlsl");
	if (!kickoffSortShader)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get kickoff sort shader");
		return;
	}
	std::vector<SRV*> srvs;
	std::vector<UAV*> uavs;
	InderectArgs args;
	IndirectArgsBuffer* sortIndirectArgs = g_theRenderer->CreateIndirectArgsbuffer(args);
	UAV* indirectArgsUAV = g_theRenderer->CreateUAV(sortIndirectArgs);
	srvs.push_back(m_counterSRV);
	uavs.push_back(indirectArgsUAV);

	g_theRenderer->RunComputeShader(kickoffSortShader, srvs, uavs, 1, 1, 1);

	srvs.clear();
	uavs.clear();
	srvs.push_back(m_counterSRV);
	srvs.push_back(m_particleDistanceSRV);
	uavs.push_back(m_particleDrawListUAV);

	bool bDone = true;

	// calculate how many threads we'll require:
	//   we'll sort 512 elements per CU (threadgroupsize 256)
	//     maybe need to optimize this or make it changeable during init
	//     TGS=256 is a good intermediate value
	unsigned int numThreadGroups = ((maxCount - 1) >> 9) + 1;
	if (numThreadGroups > 1)
	{
		bDone = false;
	}

	// sort all buffers of size 512 (and presort bigger ones)
	ComputeShader* sortShader = g_theRenderer->CreateOrGetComputeShaderFromFile("Data/Shaders/AMD_SortCS.hlsl");
	if (!sortShader)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get sort shader");
		return;
	}
	g_theRenderer->RunComputeShader(sortShader, srvs, uavs, sortIndirectArgs);

	ComputeShader* sortStepShader = g_theRenderer->CreateOrGetComputeShaderFromFile("Data/Shaders/AMD_SortStepCS.hlsl");
	if (!sortStepShader)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get sort step shader");
		return;
	}

	ComputeShader* sortInnerShader = g_theRenderer->CreateOrGetComputeShaderFromFile("Data/Shaders/AMD_SortInnerCS.hlsl");
	if (!sortInnerShader)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get sort inner shader");
		return;
	}

	int presorted = 512;
	while (!bDone)
	{
		//incremental sorting
		bDone = true;
		uint32_t numThreadGroupsStepShader = 0;

		if (maxCount > presorted)
		{
			if (maxCount > presorted * 2)
			{
				bDone = false;
			}

			uint32_t pow2 = presorted;
			while ((int)pow2 < maxCount)
			{
				pow2 *= 2;
			}
			numThreadGroupsStepShader = pow2 >> 9;
		}

		uint32_t nMergeSize = presorted * 2;
		for (uint32_t nMergeSubSize = nMergeSize >> 1; nMergeSubSize > 256; nMergeSubSize = nMergeSubSize >> 1)
		{
			SortConstants sort;
			sort.jobParamsX = nMergeSubSize;
			if (nMergeSubSize == nMergeSize >> 1)
			{
				sort.jobParamsY = 2 * nMergeSubSize - 1;
				sort.jobParamsZ = -1;
			}
			else
			{
				sort.jobParamsY = nMergeSubSize;
				sort.jobParamsZ = 1;
			}

			g_theRenderer->CopyCPUToGPU((void*)&sort, m_sortConstantsCBO);
			g_theRenderer->BindConstantBuffer(k_sortConstantsSlot, m_sortConstantsCBO);

			g_theRenderer->RunComputeShader(sortStepShader, srvs, uavs, numThreadGroupsStepShader, 1, 1);
		}

		g_theRenderer->RunComputeShader(sortInnerShader, srvs, uavs, numThreadGroupsStepShader, 1, 1);
		presorted *= 2;
	}
	delete indirectArgsUAV;
	delete sortIndirectArgs;
}
