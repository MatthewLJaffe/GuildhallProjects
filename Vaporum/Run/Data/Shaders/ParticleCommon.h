#include "RNG.h"

cbuffer CameraConstants : register(b2)
{
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
};

cbuffer ParticleConstants : register(b5)
{
	float c_deltaSeconds;
	float3 c_playerPosition;

	int2 c_spriteAtlasDimensions;
	int c_frameCount;
	uint c_emittedParticles;

	uint c_maxParticles;
	uint c_numEmitterConfigs;
	float c_timeElapsed;
	uint c_numPhysicsObjects;
};

struct Particle
{
	float3 position;
	float liveTime;

	float3 velocity;
	uint configIndex;

	float2 uvBottomLeft;
	float2 uvTopRight;

	float3 emissionPosition;
	float currentDelayReturnTime;

	float3 lifetimeVelocity;
	float padding;
};

struct ColorOverLifeEntry
{
	float time;
	float3 padding;
	float4 color;
};

struct ParticlePhysicsObjectGPU
{
	float3 position;
	float forceMagnitude;

	int attract;
	float radius;
	float falloffExponent;
	int isActive;
};

struct MeshEmissionVert
{
	float3 position;
	float padding;
	float3 normal;
	float padding2;
};

struct MeshEntry
{
	uint size;
	uint offset;
	float2 padding;
};

struct ParticleEmitterInstanceGPU
{
	float4x4 localToWorldMatrix;

	float4x4 worldToLocalMatrix;

	int definitionIndex;
	uint particlesToEmitThisFrame;
	uint killParticles;
	uint emissionStartIdx;
};

struct FloatPoint
{
	float time;
	int easingFunction;
	float minValue;
	float maxValue;
};

struct Float2Point
{
	float time;
	int easingFunction;
	float2 padding;

	float2 minValue;
	float2 maxValue;
};

struct Float3Point
{
	float3 minValue;
	float time;

	float3 maxValue;
	int easingFunction;
};

struct FloatGraph
{
	FloatPoint points[16];

	uint numPoints;
	int dataMode;
	float constantValue;
	float minValue;

	float maxValue;
	uint isDirty;
	float2 padding;
};

struct Float2Graph
{
	Float2Point points[16];

	uint numPoints;
	int dataMode;
	float2 constantValue;

	float2 minValue;
	float2 maxValue;

	uint isDirty;
	float3 padding;
};

struct Float3Graph
{
	Float3Point points[16];

	int dataMode;
	float3 constantValue;

	float3 minValue;
	uint numPoints;

	float3 maxValue;
	uint isDirty;
};

static const int FLOATGRAPH_MAX_SPEED = 0;
static const int FLOATGRAPH_DRAG_FORCE = 1;
static const int FLOATGRAPH_CURL_NOISE_FORCE = 2;
static const int FLOATGRAPH_POINT_FORCE = 3;
static const int FLOATGRAPH_VORTEX_FORCE = 4;
static const int FLOATGRAPH_EMISSIVE = 5;
static const int FLOATGRAPH_ALPHA_OBSCURANCE = 6;
static const int FLOATGRAPH_PAN_TEXTURE_CONTRIBUTION = 7;
static const int FLOATGRAPH_RADIAL_VELOCITY = 8;
static const int NUM_FLOATGRAPHS = 9;

static const int FLOAT2GRAPH_SIZE = 0;
static const int NUM_FLOAT2GRAPHS = 1;

static const int FLOAT3GRAPH_LINEAR_FORCE = 0;
static const int FLOAT3GRAPH_LIFETIME_VELOCITY = 1;
static const int FLOAT3GRAPH_LIFETIME_POSITION = 2;
static const int NUM_FLOAT3GRAPHS = 3;

struct ParticleEmitterUpdateDefinitionGPU
{
	float emissionRate;
	uint emissionMode;
	uint repeat;
	float repeatTime;

	float emitTime;
	float2 lifetime;
	uint isDirty;

	uint emissionType;
	float3 boxDimensions;

	uint meshEntryEmissionIndex;
	float3 meshScale;

	float2 velocityXRange;
	float2 velocityYRange;

	float2 velocityZRange;
	float emissionRadius;
	int emissionDistribution;

	int curlNoiseOctives;
	float3 curlNoisePan;

	uint curlNoiseAffectPosition;
	uint stretchBillboard;
	float lengthPerSpeed;
	uint worldSimulation;

	float3 pointForcePosition;
	float pointForceFalloffExponent;

	int pointForceAttract;
	float pointForceRadius;
	float vortexForceRadius;
	float curlNoiseScale;

	float3 vortexAxisDir;
	uint numColorsOverLifetime;

	float3 vortexAxisOrigin;
	float curlNoiseSampleSize;

	float returnToOriginForce;
	float returnToOriginDelay;
	uint ignoreForcesWhenAtOrigin;
	float perlinNoiseForce;

	ColorOverLifeEntry colorOverLifetime[8];

	int spriteStartIndex;
	int spriteEndIndex;
	int orientToVelocity;
	uint setLifetimeVelocity;

	int spriteEasingFunction;
	uint ignoreWorldPhysics;
	float2 padding;

	int2 spriteSheetdimensions;
	uint setLifetimePosition;
	uint velocityMode;

	float2 atlasUVMins;
	float2 atlasUVMaxs;
};

struct ParticleEmitterRenderDefinitionGPU
{
	int textureToPanIndex;
	uint isDirty;
	float2 lifetimeRange;

	float2 panTextureSpeed;
	float2 panTextureSampleScale;
};

static const uint PARTICLECOUNTER_OFFSET_ALIVECOUNT = 0;
static const uint PARTICLECOUNTER_OFFSET_ALIVECOUNT_AFTERSIMULATION = PARTICLECOUNTER_OFFSET_ALIVECOUNT + 4;
static const uint PARTICLECOUNTER_OFFSET_DEADCOUNT = PARTICLECOUNTER_OFFSET_ALIVECOUNT_AFTERSIMULATION + 4;
static const uint PARTICLECOUNTER_OFFSET_DRAWCOUNT = PARTICLECOUNTER_OFFSET_DEADCOUNT + 4;

static const uint SEED_OFFSET_LIFETIME = 0;
static const uint SEED_OFFSET_SIZE = 1;
static const uint SEED_OFFSET_COLOR = 2;
static const uint SEED_OFFSET_MAX_SPEED = 3;
static const uint SEED_OFFSET_LINEAR_FORCE = 4;
static const uint SEED_OFFSET_DRAG_FORCE = 5;
static const uint SEED_OFFSET_CURL_NOISE_FORCE = 6;
static const uint SEED_OFFSET_POINT_FORCE = 7;
static const uint SEED_OFFSET_VORTEX_FORCE = 8;
static const uint SEED_OFFSET_EMISSIVE = 9;
static const uint SEED_OFFSET_ALPHA_OBSCURANCE = 10;
static const uint SEED_OFFSET_PAN_TEXTURE_CONTRIBUTION = 11;
static const uint SEED_OFFSET_LIFETIME_VELOCITY = 12;
static const uint SEED_OFFSET_LIFETIME_POSITION = 13;
static const uint SEED_OFFSET_RADIAL_VELOCITY = 14;

static const int DATA_MODE_CONSTANT = 0;
static const int DATA_MODE_RANGE = 1;
static const int DATA_MODE_GRAPH = 2;


uint DenormalizeByte(float normalizedValue)
{
	//clamp in float in range 0 - 256
	float valueIn256Range = clamp(normalizedValue * 256.f, 0.f, 256.f);
	//at the end of the spectrum return 1 
	if (valueIn256Range == 256.f)
	{
		return 255;
	}
	//otherwise round down to get even distribution
	return (uint)(valueIn256Range);
}

uint GetColorAsUint(float4 color)
{
	uint rComponent = DenormalizeByte(color.r) << 0 * 4;
	uint gComponent = DenormalizeByte(color.g) << 2 * 4;
	uint bComponent = DenormalizeByte(color.b) << 4 * 4;
	uint aComponent = DenormalizeByte(color.a) << 6 * 4;
	uint colorAsUint = rComponent | gComponent | bComponent | aComponent;
	return colorAsUint;
}

bool IsPositionInViewFrusum(float4 worldPosition)
{
	float4 viewPos = mul(ViewMatrix, worldPosition);
	float4 clipPos = mul(ProjectionMatrix, viewPos);
	if (clipPos.w == 0)
	{
		return false;
	}
	float xNDC = clipPos.x / clipPos.w;
	float yNDC = clipPos.y / clipPos.w;
	float zNDC = clipPos.z / clipPos.w;
	if (xNDC >= -1.f && xNDC <= 1.f &&
		yNDC >= -1.f && yNDC <= 1.f &&
		zNDC >= -1.f && zNDC <= 1.f)
	{
		return true;
	}

	return false;
}

bool IsParticleVisible(float3 position, float2 size)
{
	float4 worldPosition = float4(position.x, position.y, position.z, 1.f);
	float extent = size.x > size.y ? size.x : size.y;
	extent *= .75f;
	float4 worldFoward = worldPosition + float4(extent, 0.f, 0.f, 0.f);
	float4 worldBack = worldPosition - float4(extent, 0.f, 0.f, 0.f);
	float4 worldTop = worldPosition + float4(0.f, 0.f, extent, 0.f);
	float4 worldBottom = worldPosition - float4(0.f, 0.f, extent, 0.f);
	float4 worldRight = worldPosition + float4(0.f, extent, 0.f, 0.f);
	float4 worldLeft = worldPosition - float4(0.f, extent, 0.f, 0.f);
	if (IsPositionInViewFrusum(worldFoward))
	{
		return true;
	}
	if (IsPositionInViewFrusum(worldBack))
	{
		return true;
	}
	if (IsPositionInViewFrusum(worldTop))
	{
		return true;
	}
	if (IsPositionInViewFrusum(worldBottom))
	{
		return true;
	}
	if (IsPositionInViewFrusum(worldRight))
	{
		return true;
	}
	if (IsPositionInViewFrusum(worldLeft))
	{
		return true;
	}
	return false;
}

float GetValueInFloatGraph(StructuredBuffer<FloatGraph> floatGraphs, 
	int graphIndex, int defIndex, float normalizedTime, uint seed)
{
	FloatGraph graph = floatGraphs[defIndex * NUM_FLOATGRAPHS + graphIndex];
	float outputValue = graph.constantValue;
	if (graph.dataMode == DATA_MODE_CONSTANT)
	{
		outputValue = graph.constantValue;
	}
	else if (graph.dataMode == DATA_MODE_RANGE)
	{
		outputValue = GetFloatNoiseInRange(graph.minValue, graph.maxValue, 0, seed);
	}
	else
	{
		outputValue = GetFloatNoiseInRange(graph.points[0].minValue, graph.points[0].maxValue, 0, seed);
		for (uint i = 0; i < graph.numPoints; i++)
		{
			if (i == graph.numPoints - 1)
			{
				outputValue = GetFloatNoiseInRange(graph.points[i].minValue, graph.points[i].maxValue, 0, seed);
				return outputValue;
			}
			float startTime = graph.points[i].time;
			float endTime = graph.points[i + 1].time;
			if (startTime > normalizedTime || endTime < normalizedTime)
			{
				continue;
			}

			float fractionInRange = (normalizedTime - startTime) / (endTime - startTime);
			fractionInRange = GetValueFromEasingFunciton(graph.points[i].easingFunction, fractionInRange);
			float minValue = lerp(graph.points[i].minValue, graph.points[i + 1].minValue, fractionInRange);
			float maxValue = lerp(graph.points[i].maxValue, graph.points[i + 1].maxValue, fractionInRange);
			outputValue = GetFloatNoiseInRange(minValue, maxValue, 0, seed);
			return outputValue;
		}
	}
	return outputValue;
}

float2 GetValueInFloat2Graph(StructuredBuffer<Float2Graph> float2Graphs,
	int graphIndex, int defIndex, float normalizedTime, uint seed, bool seperateAxis = true) 
{
	Float2Graph graph = float2Graphs[defIndex * NUM_FLOAT2GRAPHS + graphIndex];

	if (graph.dataMode == DATA_MODE_CONSTANT)
	{
		return graph.constantValue;
	}

	if (graph.dataMode == DATA_MODE_RANGE)
	{
		return  GetFloat2NoiseInRange(graph.minValue, graph.maxValue, 0, seed, seperateAxis);
	}

	float2 outputValue = GetFloat2NoiseInRange(graph.points[0].minValue, graph.points[0].maxValue, 0, seed, seperateAxis);
	for (uint i = 0; i < graph.numPoints; i++)
	{
		if (i == graph.numPoints - 1)
		{
			outputValue = GetFloat2NoiseInRange(graph.points[i].minValue, graph.points[i].maxValue, 0, seed, seperateAxis);
			return outputValue;
		}
		float startTime = graph.points[i].time;
		float endTime = graph.points[i + 1].time;
		if (startTime > normalizedTime || endTime < normalizedTime)
		{
			continue;
		}

		float fractionInRange = (normalizedTime - startTime) / (endTime - startTime);
		fractionInRange = GetValueFromEasingFunciton(graph.points[i].easingFunction, fractionInRange);
		float2 minValue = lerp(graph.points[i].minValue, graph.points[i + 1].minValue, fractionInRange);
		float2 maxValue = lerp(graph.points[i].maxValue, graph.points[i + 1].maxValue, fractionInRange);
		outputValue = GetFloat2NoiseInRange(minValue, maxValue, 0, seed, seperateAxis);
		return outputValue;
	}
	return outputValue;
}

float3 GetValueInFloat3Graph(StructuredBuffer<Float3Graph> float3Graphs,
	int graphIndex, int defIndex, float normalizedTime, uint seed, bool seperateAxis = true)
{
	Float3Graph graph = float3Graphs[defIndex * NUM_FLOAT3GRAPHS + graphIndex];

	if (graph.dataMode == DATA_MODE_CONSTANT)
	{
		return graph.constantValue;
	}

	if (graph.dataMode == DATA_MODE_RANGE)
	{
		return  GetFloat3NoiseInRange(graph.minValue, graph.maxValue, 0, seed, seperateAxis);
	}

	float3 outputValue = GetFloat3NoiseInRange(graph.points[0].minValue, graph.points[0].maxValue, 0, seed, seperateAxis);
	for (uint i = 0; i < graph.numPoints; i++)
	{
		if (i == graph.numPoints - 1)
		{
			outputValue = GetFloat3NoiseInRange(graph.points[i].minValue, graph.points[i].maxValue, 0, seed, seperateAxis);
			return outputValue;
		}
		float startTime = graph.points[i].time;
		float endTime = graph.points[i + 1].time;
		if (startTime > normalizedTime || endTime < normalizedTime)
		{
			continue;
		}

		float fractionInRange = (normalizedTime - startTime) / (endTime - startTime);
		fractionInRange = GetValueFromEasingFunciton(graph.points[i].easingFunction, fractionInRange);
		float3 minValue = lerp(graph.points[i].minValue, graph.points[i + 1].minValue, fractionInRange);
		float3 maxValue = lerp(graph.points[i].maxValue, graph.points[i + 1].maxValue, fractionInRange);
		outputValue = GetFloat3NoiseInRange(minValue, maxValue, 0, seed, seperateAxis);
		return outputValue;
	}
	return outputValue;
}