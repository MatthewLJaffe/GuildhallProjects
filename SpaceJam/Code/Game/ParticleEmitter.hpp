#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Particle.hpp"

constexpr unsigned int MAX_PARTICLES_PER_EMITTER = 8388480;

struct ParticleEmitterConfig
{
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientationDegrees;
	float m_emissionRate = 100000.000f;
	float m_emissionRadius = 10.00f;
	float m_lifetime = 20.f;
	FloatRange m_velocityXRange = FloatRange(-0.f, 0.f);
	FloatRange m_velocityYRange = FloatRange(-0.f, 0.f);
	FloatRange m_velocityZRange = FloatRange(-0.f, 0.f);
	float m_acceleration = 5.f;
	float m_maxSpeed = 10.f;
	Rgba8 m_color = Rgba8::WHITE;
	Texture* m_texture = nullptr;
	AABB2 m_uvs = AABB2::ZERO_TO_ONE;
	Vec2 m_size = Vec2(.1f, .1f);
	bool m_sortParticles = true;
	int m_spriteSheetDimension[2] = {9, 9};
	int m_spriteStartIndex = 0;
	int m_spriteEndIndex = 0;
	float m_perlinNoiseForce = 0.f;
	Vec3  m_linearForce = Vec3(0.f, 70.f, 70.f);
	float m_curlNoiseForce = 100.f;
	float m_curlNoiseScale = 20.f;
	float m_curlNoiseSampleSize = 20.f;
	int m_curlNoiseOctives = 7;
	float m_pointForceStrength = 200.f;
	float m_pointForcePosition[3] = { 0.f, 50.f, 50.f };
	float m_pointForceFalloffExponent = 1.5f;
	bool m_pointForceAttract = true;
	float m_pointForceRadius = 50.f;
	float m_vortexAxisDir[3] = {0.f, 0.f, 0.f};
	float m_vortexForce = 0.f;
	float m_vortexAxisOrigin[3] = { 0.f, 0.f, 0.f };
	float m_vortexForceRadius = 0.f;
	float m_dragForce = 0.f;
};

struct CounterBuffer
{
	unsigned int aliveCount = 0;
	unsigned int aliveCountAfterSim = 0;
	unsigned int deadCount = MAX_PARTICLES_PER_EMITTER;
	unsigned int drawCount = 0;
};

struct ParticleConstants
{
	float m_deltaSeconds;
	Vec3 m_playerPosition;

	float m_acceleration;
	Vec3 m_emitterPosition = Vec3::ZERO;

	float maxSpeed;
	float m_emissionRadius = 1.f;
	float m_lifetime = 2.f;
	float padding;

	FloatRange m_velocityXRange = FloatRange(-5.f, 5.f);
	FloatRange m_velocityYRange = FloatRange(-5.f, 5.f);

	FloatRange m_velocityZRange = FloatRange(-5.f, 5.f);
	Vec2 m_size = Vec2::ONE;

	int m_frameCount;
	unsigned int m_emittedParticles;
	unsigned int m_sortParticles = 0;
	float padding2;

	float m_color[4];

	int m_spriteSheetDimension[2] = {1, 1};
	int m_startIndex = 0;
	int m_endIndex = 0;

	float m_perlinNoiseForce = 0.f;
	Vec3 m_linearForce;

	float m_curlNoiseForce = 0.f;
	float m_curlNoiseScale = 0.f;
	float m_curlNoiseSampleSize = 0.f;
	int m_curlNoiseOctives = 1;

	float m_pointForceStrength;
	Vec3 m_pointForcePosition;

	float m_pointForceFalloffExponent;
	int m_pointForceAttract;
	float m_pointForceRadius;
	float m_vortexForce = 0.f;

	float m_vortexForceRadius = 0.f;
	Vec3 m_vortexAxisDir;

	Vec3 m_vortexAxisOrigin;
	float m_dragForce = 0.f;
};

struct ParticleGlobalVariables
{
	int liveParticlesFromLastFrame = 0;
	int emmitedParticlesThisFrame = 0;
	int lastLiveParticleIdx = 0;
	float padding;
};

struct SortConstants
{
	int jobParamsX = 1;
	int jobParamsY = 1;
	int jobParamsZ = 1;
	float padding;
};

class ParticleEmitter
{
public:
	ParticleEmitter(ParticleEmitterConfig& config);
	~ParticleEmitter();
	void Update(float deltaSeconds);
	void RenderAddVertsGPU();
	Mat44 GetModelMatrix();
	void EndFrame();
	ParticleEmitterConfig m_config;
	unsigned int m_aliveCountFromlastFrame = 0;
	unsigned int m_culledCountLastFrame = 0;
	unsigned int m_drawCount = 0;
private:
	std::vector<Particle> m_particles;
	unsigned int m_totalParticles = 0;
	unsigned int m_totalParticlesLastFrame = 0;

	void DetermineParticlesToEmit(float deltaSeconds);
	void EmitParticles(float deltaSeconds);
	void UpdateParticles(float deltaSeconds);
	void InitResources();
	void SortParticlesGPU(int maxCount);
	int m_particlesToEmit = 0;
	float m_owedParticlesAsFloat = 0.f;
	static const int k_particleConstantsSlot = 5;
	static const int k_sortConstantsSlot = 6;
	int m_frameCount = 0;

	ConstantBuffer* m_particleConstantsCBO = nullptr;
	ConstantBuffer* m_sortConstantsCBO = nullptr;
	UAV* m_particlesUAV = nullptr;
	SRV* m_particlesSRV = nullptr;
	StructuredBuffer* m_particlesBuffer = nullptr;
	UAV* m_particleAliveList1UAV = nullptr;
	StructuredBuffer* m_particleAliveList1Buffer = nullptr;
	UAV* m_particleAliveList2UAV = nullptr;
	SRV* m_particleAliveList2SRV = nullptr;
	StructuredBuffer* m_particleAliveList2Buffer = nullptr;
	UAV* m_deadListUAV = nullptr;
	StructuredBuffer* m_deadListBuffer = nullptr;
	UAV* m_counterUAV = nullptr;
	SRV* m_counterSRV = nullptr;
	GPUBuffer* m_counterBuffer = nullptr;
	VertexBuffer* m_vbo = nullptr;
	UAV* m_vboUAV = nullptr;
	UAV* m_particleDistanceUAV = nullptr;
	SRV* m_particleDistanceSRV = nullptr;
	StructuredBuffer* m_particleDistanceBuffer = nullptr;
	StructuredBuffer* m_particleDrawListBuffer = nullptr;
	UAV* m_particleDrawListUAV = nullptr;
	SRV* m_particleDrawListSRV = nullptr;
};