#include "RNG.h"
SamplerState diffuseSampler : register(s0);

//------------------------------------------------------------------------------------------------
cbuffer LightConstants : register(b1)
{
	float3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
	float3 padding;
	float3 worldEyePosition;
	float minimumFalloff;
	float maximumFalloff;
	float minimumFalloffMultiplier;
	float maximumFalloffMultiplier;
	int renderAmbientDebugFlag;
	int renderDiffuseDebugFlag;
	int renderSpecularDebugFlag;
	int renderEmissiveDebugFlag;
	int useDiffuseMapDebugFlag;
	int useNormalMapDebugFlag;
	int useSpecularMapDebugFlag;
	int useGlossinessMapDebugFlag;
	int useEmissiveMapDebugFlag;
};

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

	int2 c_windowDimensions;
	float c_nearPlane;
	float c_farPlane;

	float4x4 c_inverseViewProjMatrix;

	uint c_numPhysicsAABB3s;
	float3 c_cameraIBasis;
};


cbuffer SortConstants : register(b6)
{
	uint c_numElements;
	int c_jobParamsX;
	int c_jobParamsY;
	int c_jobParamsZ;
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
	uint idInEmitter;

	float3 reflectionVector;
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

struct ParticlePhysicsAABB3GPU
{
	float3 mins;
	int isActive;

	float3 maxs;
	float padding;
};

struct MeshParticleVert
{
	float3 position;
	float padding;

	float2 uv;
	float2 padding2;

	float3 normal;
	float padding3;
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

	int totalParticlesEmitted;
	float3 emitterVelocity;
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
static const int FLOATGRAPH_ROTATION_1D = 9;
static const int NUM_FLOATGRAPHS = 10;

static const int FLOAT2GRAPH_SIZE = 0;
static const int NUM_FLOAT2GRAPHS = 1;

static const int FLOAT3GRAPH_LINEAR_FORCE = 0;
static const int FLOAT3GRAPH_LIFETIME_VELOCITY = 1;
static const int FLOAT3GRAPH_LIFETIME_POSITION = 2;
static const int FLOAT3GRAPH_LIFETIME_ROTATION_3D = 3;
static const int FLOAT3GRAPH_LIFETIME_SCALE_3D = 4;
static const int NUM_FLOAT3GRAPHS = 5;

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
	int stretchMode;
	int particleMeshIndex;

	int2 spriteSheetdimensions;
	uint setLifetimePosition;
	uint velocityMode;

	float2 atlasUVMins;
	float2 atlasUVMaxs;

	int partialMeshTriangles;
	uint depthBufferCollisions;
	uint inheritEmitterVelocity;
	float inheritVelocityPercentage;
};

struct ParticleEmitterRenderDefinitionGPU
{
	uint isDirty;
	float2 lifetimeRange;
	uint softParticles;

	float2 panTextureSpeed;
	float2 panTextureSampleScale;

	float2 panTextureAtlasUVMins;
	float2 panTextureAtlasUVMaxs;
};

static const uint PARTICLECOUNTER_OFFSET_ALIVECOUNT = 0;
static const uint PARTICLECOUNTER_OFFSET_ALIVECOUNT_AFTERSIMULATION = PARTICLECOUNTER_OFFSET_ALIVECOUNT + 4;
static const uint PARTICLECOUNTER_OFFSET_DEADCOUNT = PARTICLECOUNTER_OFFSET_ALIVECOUNT_AFTERSIMULATION + 4;
static const uint PARTICLECOUNTER_OFFSET_DRAWCOUNT = PARTICLECOUNTER_OFFSET_DEADCOUNT + 4;
static const uint PARTICLECOUNTER_OFFSET_MESH_DRAWCOUNT = PARTICLECOUNTER_OFFSET_DRAWCOUNT + 4;


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
static const uint SEED_OFFSET_ROTATION_1D = 15;
static const uint SEED_OFFSET_ROTATION_3D = 16;
static const uint SEED_OFFSET_SCALE_3D = 16;


static const int DATA_MODE_CONSTANT = 0;
static const int DATA_MODE_RANGE = 1;
static const int DATA_MODE_GRAPH = 2;


uint DenormalizeByte(float normalizedValue)
{
	//clamp in float in range 0 - 256
	float valueIn256Range = clamp(normalizedValue * 256.f, 0.f, 256.f);

	uint output = (uint)(valueIn256Range);
	//at the end of the spectrum return 1 
	if (valueIn256Range == 256.f)
	{
		output = 255;
	}
	//otherwise round down to get even distribution
	return output;
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
	bool output = false;
	if (clipPos.w == 0)
	{
		output = false;
	}
	float xNDC = clipPos.x / clipPos.w;
	float yNDC = clipPos.y / clipPos.w;
	float zNDC = clipPos.z / clipPos.w;
	if (xNDC >= -1.f && xNDC <= 1.f &&
		yNDC >= -1.f && yNDC <= 1.f &&
		zNDC >= -1.f && zNDC <= 1.f)
	{
		output = true;
	}

	return output;
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
	bool output = false;
	if (IsPositionInViewFrusum(worldFoward))
	{
		output = true;
	}
	else if (IsPositionInViewFrusum(worldBack))
	{
		output = true;
	}
	else if (IsPositionInViewFrusum(worldTop))
	{
		output = true;
	}
	else if (IsPositionInViewFrusum(worldBottom))
	{
		output = true;
	}
	else if (IsPositionInViewFrusum(worldRight))
	{
		output = true;
	}
	else if (IsPositionInViewFrusum(worldLeft))
	{
		output = true;
	}
	return output;
}

float GetValueInFloatGraph(StructuredBuffer<FloatGraph> floatGraphs, 
	int graphIndex, int defIndex, float normalizedTime, uint seed)
{
	
	FloatGraph graph = floatGraphs[defIndex * NUM_FLOATGRAPHS + graphIndex];
	float outputValue = graph.constantValue;

	if (graph.dataMode == DATA_MODE_RANGE)
	{
		outputValue = GetFloatNoiseInRange(graph.minValue, graph.maxValue, 0, seed);
	}
	else if (graph.dataMode == DATA_MODE_GRAPH)
	{
		outputValue = GetFloatNoiseInRange(graph.points[0].minValue, graph.points[0].maxValue, 0, seed);
		for (uint i = 0; i < graph.numPoints; i++)
		{
			if (i == graph.numPoints - 1)
			{
				outputValue = GetFloatNoiseInRange(graph.points[i].minValue, graph.points[i].maxValue, 0, seed);
				break;
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
			break;
		}
	}
	return outputValue;
}

float2 GetValueInFloat2Graph(StructuredBuffer<Float2Graph> float2Graphs,
	int graphIndex, int defIndex, float normalizedTime, uint seed, bool seperateAxis = true) 
{
	Float2Graph graph = float2Graphs[defIndex * NUM_FLOAT2GRAPHS + graphIndex];
	float2 outputValue = graph.constantValue;
	if (graph.dataMode == DATA_MODE_CONSTANT)
	{
		outputValue = graph.constantValue;
	}
	else if (graph.dataMode == DATA_MODE_RANGE)
	{
		outputValue = GetFloat2NoiseInRange(graph.minValue, graph.maxValue, 0, seed, seperateAxis);
	}
	else
	{
		outputValue = GetFloat2NoiseInRange(graph.points[0].minValue, graph.points[0].maxValue, 0, seed, seperateAxis);
		for (uint i = 0; i < graph.numPoints; i++)
		{
			if (i == graph.numPoints - 1)
			{
				outputValue = GetFloat2NoiseInRange(graph.points[i].minValue, graph.points[i].maxValue, 0, seed, seperateAxis);
				break;
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
			break;
		}
	}
	return outputValue;
}


float3 GetValueInFloat3Graph(StructuredBuffer<Float3Graph> float3Graphs,
	int graphIndex, int defIndex, float normalizedTime, uint seed, bool seperateAxis = true)
{
	Float3Graph graph = float3Graphs[defIndex * NUM_FLOAT3GRAPHS + graphIndex];
	float3 outputValue = graph.constantValue;
	if (graph.dataMode == DATA_MODE_CONSTANT)
	{
		outputValue = graph.constantValue;
	}
	else if (graph.dataMode == DATA_MODE_RANGE)
	{
		outputValue = GetFloat3NoiseInRange(graph.minValue, graph.maxValue, 0, seed, seperateAxis);
	}
	else
	{
		outputValue = GetFloat3NoiseInRange(graph.points[0].minValue, graph.points[0].maxValue, 0, seed, seperateAxis);
		for (uint i = 0; i < graph.numPoints; i++)
		{
			if (i == graph.numPoints - 1)
			{
				outputValue = GetFloat3NoiseInRange(graph.points[i].minValue, graph.points[i].maxValue, 0, seed, seperateAxis);
				break;
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
			break;
		}
	}
	return outputValue;
}

float4x4 GetEulerAnglesAsMatrix(in float3 eulerAngles)
{
	float degToRad = PI / 180.f;
	float2 zRotIBasis = float2(cos(eulerAngles.x * degToRad), sin(eulerAngles.x * degToRad));
	float2 zRotJBasis = float2(-zRotIBasis.y, zRotIBasis.x);
	float4x4 zRotation = {	zRotIBasis.x,	zRotJBasis.x,	0.f,	0.f,
							zRotIBasis.y,	zRotJBasis.y,	0.f,	0.f,
							0.f,			0.f,			1.f,	0.f,
							0.f,			0.f,			0.f,	1.f};
	float3 yRotIBasis = float3(cos(eulerAngles.y * degToRad), 0.f, -sin(eulerAngles.y * degToRad));
	float3 yRotJBasis = float3(0.f, 1.f, 0.f);
	float3 yRotKBasis = float3(-yRotIBasis.z, 0.f, yRotIBasis.x);
	float4x4 yRotation = {	yRotIBasis.x,	yRotJBasis.x,	yRotKBasis.x,	0.f,
							yRotIBasis.y,	yRotJBasis.y,	yRotKBasis.y,	0.f,
							yRotIBasis.z,	yRotJBasis.z,	yRotKBasis.z,	0.f,
							0.f,			0.f,			0.f,			1.f};
	float3 xRotIBasis = float3(1.f, 0.f, 0.f);
	float3 xRotJBasis = float3(0.f, cos(eulerAngles.z * degToRad), sin(eulerAngles.z * degToRad));
	float3 xRotKBasis = float3(0.f, -xRotJBasis.z, xRotJBasis.y);
	float4x4 xRotation = {	xRotIBasis.x,	xRotJBasis.x,	xRotKBasis.x,	0.f,
							xRotIBasis.y,	xRotJBasis.y,	xRotKBasis.y,	0.f,
							xRotIBasis.z,	xRotJBasis.z,	xRotKBasis.z,	0.f,
							0.f,			0.f,			0.f,			1.f};
	float4x4 rotationMatrix = mul(zRotation, yRotation);
	rotationMatrix = mul(rotationMatrix, xRotation);
	return rotationMatrix;
}

float computeLinearDepth(float z)
{
	float linZ = (c_nearPlane * c_farPlane) / (c_farPlane + z * c_nearPlane - z * c_farPlane);
	return linZ;
}

// Reconstructs world-space position from depth buffer
//	uv		: screen space coordinate in [0, 1] range
//	z		: depth value at current pixel
//	InvVP	: Inverse of the View-Projection matrix that was used to generate the depth value
inline float3 reconstruct_position(in float2 uv, in float z, in float4x4 inverse_view_projection)
{
	//get x and y into -1 to 1
	float x = uv.x * 2 - 1;
	float y = (1 - uv.y) * 2 - 1; //[1,0] to [-1,1]

	//ndc space position
	float4 position_s = float4(x, y, z, 1);
	float wValue = computeLinearDepth(z);
	float4 unWDividedPos = position_s * wValue;
	float3 newPos = (float3)mul(inverse_view_projection, unWDividedPos);
	return newPos;
}

float3 GetNearestPointOnAABB3D(in float3 pos, in float3 mins, in float3 maxs)
{
	return float3(clamp(pos.x, mins.x, maxs.x), clamp(pos.y, mins.y, maxs.y), clamp(pos.z, mins.z, maxs.z));
}

bool DoSphereAndAABBOverlap(in float3 sphereCenter, in float sphereRadius, in float3 mins, in float3 maxs)
{
	float3 nearestPoint = GetNearestPointOnAABB3D(sphereCenter, mins, maxs);
	float3 dispToPoint = nearestPoint - sphereCenter;
	return dot(dispToPoint, dispToPoint) < sphereRadius * sphereRadius;
}

