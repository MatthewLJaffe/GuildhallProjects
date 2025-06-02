#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

struct FloatPoint
{
	float m_time = 0.f;
	int m_easingFunction = 0;
	float m_minValue = 0.f;
	float m_maxValue = 0.f;
};

struct Float2Point
{
	float m_time = 0.f;
	int m_easingFunction = 0;
	Vec2 padding;

	float m_minValue[2] = { 0.f, 0.f };
	float m_maxValue[2] = { 0.f, 0.f };
};

struct Float3Point
{
	float m_minValue[3] = { 0.f, 0.f, 0.f };
	float m_time = 0.f;

	float m_maxValue[3] = { 0.f, 0.f, 0.f };
	int m_easingFunction = 0;
};

struct FloatGraph
{
	FloatPoint points[16];

	unsigned int numPoints = 0;
	int dataMode = 0;
	float constantValue = -999.f;
	float minValue = 0.f;

	float maxValue = 0.f;
	unsigned int isDirty = 1;
	Vec2 padding;
};

struct Float2Graph
{
	Float2Point points[16];

	unsigned int numPoints = 0;
	int dataMode = 0;
	float constantValue[2] = {-999.f, -999.f};

	float minValue[2] = {0.f, 0.f};
	float maxValue[2] = {0.f, 0.f};

	unsigned int isDirty = 1;
	Vec3 padding;
};

struct Float3Graph
{
	Float3Point points[16];

	int dataMode = 0;
	float constantValue[3] = { -999.f, -999.f, -999.f };

	float minValue[3] = { 0.f, 0.f, 0.f };
	unsigned int numPoints = 0;

	float maxValue[3] = { 0.f, 0.f, 0.f };
	unsigned int isDirty = 1;
};

enum class FloatGraphType
{
	FLOATGRAPH_MAX_SPEED = 0,
	FLOATGRAPH_DRAG_FORCE,
	FLOATGRAPH_CURL_NOISE_FORCE,
	FLOATGRAPH_POINT_FORCE,
	FLOATGRAPH_VORTEX_FORCE,
	FLOATGRAPH_EMISSIVE,
	FLOATGRAPH_ALPHA_OBSCURANCE,
	FLOATGRAPH_PAN_TEXTURE_CONTRIBUTION,
	FLOATGRAPH_RADIAL_VELOCITY,
	FLOATGRAPH_ROTATION_1D,
	NUM_FLOATGRAPHS
};

enum class Float2GraphType
{
	FLOAT2GRAPH_SIZE = 0,
	NUM_FLOAT2GRAPHS
};

enum class Float3GraphType
{
	FLOAT3GRAPH_LINEAR_FORCE = 0,
	FLOAT3GRAPH_LIFETIME_VELOCITY,
	FLOAT3GRAPH_LIFETIME_POSITION,
	FLOAT3GRAPH_LIFETIME_ROTATION_3D,
	FLOAT3GRAPH_LIFETIME_SCALE_3D,
	NUM_FLOAT3GRAPHS
};

struct ColorOverLifeEntry
{
	float m_time = 0.f;
	Vec3 padding;
	float m_color[4] = { 1.f, 1.f, 1.f, 1.f };
};

struct EmitterUpdateDefinitionGPU
{
	float m_emissionRate = 100;
	unsigned int m_emissionMode = 0;
	unsigned int m_repeat = 0;
	float m_repeatTime = 1.f;

	float m_emitTime = -1.f;
	float m_lifetime[2] = {1.f, 1.f};
	unsigned int isDirty = 1;

	unsigned int m_emissionType = 0;
	float m_boxDimensions[3] = { 1.f, 1.f, 1.f };

	unsigned int m_meshEntryEmissionIndex = 0;
	float m_meshScale[3] = { 1.f, 1.f, 1.f };

	float m_velocityXRange[2] = { 0.f, 0.f };
	float m_velocityYRange[2] = { 0.f, 0.f };

	float m_velocityZRange[2] = { 0.f, 0.f };
	float m_emissionRadius = 1.f;
	int m_emissionDistribution = 0;

	int m_curlNoiseOctives = 1;
	float m_curlNoisePan[3] = { 0.f, 0.f, 0.f };

	unsigned int m_curlNoiseAffectPosition = 0;
	unsigned int m_stretchBillboard = 0;
	float m_lengthPerSpeed = 0.f;
	unsigned int m_worldSimulation = 0;

	float m_pointForcePosition[3] = { 0.f, 0.f, 0.f };
	float m_pointForceFalloffExponent = 2.f;

	int m_pointForceAttract = 1;
	float m_pointForceRadius = 0.f;
	float m_vortexForceRadius = 0.f;
	float m_curlNoiseScale = 10.f;

	float m_vortexAxisDir[3] = { 1.f, 0.f, 0.f };
	unsigned int m_numColorsOverLifetime = 1;

	Vec3 m_vortexAxisOrigin;
	float m_curlNoiseSampleSize = 10.f;

	float m_returnToOriginForce = 0.f;
	float m_returnToOriginDelay = 1.f;
	unsigned int m_ignoreForcesWhenAtOrigin = 0;
	float m_perlinNoiseForce = 0.f;

	ColorOverLifeEntry m_colorOverLifetime[8];

	int m_spriteStartIndex = 0;
	int m_spriteEndIndex = 0;
	int m_orientToVelocity = 0;
	unsigned int m_setLifetimeVelocity = 0;

	int m_spriteEasingFunction = 0;
	unsigned int m_ignoreWorldPhysics = 1;
	int m_stretchMode = 0;
	int m_particleMeshIndex = -1;

	int m_spriteSheetdimensions[2] = { 9, 9 };
	unsigned int m_setLifetimePosition = 0;
	unsigned int m_velocityMode = 0;

	Vec2 m_atlasUVMins;
	Vec2 m_atlasUVMaxs;

	int m_partialMeshTriangles = 0;
	unsigned int m_depthBufferCollisions = 0;
	unsigned int m_inheritEmitterVelocity = 0;
	float m_inheritVelocityPercentage = 1.f;
};

struct EmitterRenderDefinitionGPU
{
	unsigned int isDirty = 1;
	float m_lifetime[2];
	unsigned int m_softParticles = 1;

	float m_panTextureSpeed[2] = { 0.f, 0.f };
	float m_panTextureSampleScale[2] = { 1.f, 1.f };

	AABB2 m_panTextureUVBounds = AABB2(Vec2(.5f + .0001f, .0625f -.0001f), Vec2(.5625f - .0001f, 0.f + .0001f));
};

struct ChildEmitter
{
	std::string m_childEmitterName = "";
	std::string m_childEmitterFile = "";
	int m_childEmitterIndex = -1;
	Vec3 m_localPosition;
	EulerAngles m_localOrientation;
};


struct ParticleEmitterDefinition
{
	std::string m_name = "New Emitter";
	std::string m_spriteSheetFilePath = "Data/Images/sprite_sheet.png";
	std::string m_meshFilePath = "";
	std::string m_panTextureFilePath = "";
	std::string m_subEmitterFilePath = "";
	std::string m_particleMeshFilePath = "";

	std::vector<ChildEmitter> m_childEmitters;

	//index in a vector that represents memory copied from cpu to gpu that SHOULD ONLY EXIST IN ONE PLACE
	int m_loadedDefinitionIndex = -1;


	float m_emitterLifetime = -1.f;
	float m_emitterStartDelay = 0.f;
	int m_numBursts = 1;
	float m_burstInterval = .1f;
	bool m_emitEmitters = false;

	float m_minSubEmitterOrientation[3] = { 0.f, 0.f, 0.f };
	float m_maxSubEmitterOrientation[3] = { 0.f, 0.f, 0.f };
	int m_subEmitterDefinitionIndex = -1;
	int m_renderMode = 0;  //0=billboard, 1=3D mesh

	std::string GetAsXML();

	std::string GetFloatGraphAsXML(FloatGraphType floatGraphType);
	std::string GetFloat2GraphAsXML(Float2GraphType floatGraphType);
	std::string GetFloat3GraphAsXML(Float3GraphType floatGraphType);
	std::string GetChildEmittersAsXML();
	std::string GetEmitterVelocityAsXML();

	static int CreateEmitterDefinitionFromXML(XmlElement const* emitterXML);

	void InitializeDefaultFloatGraphValues(std::vector<FloatGraph>* localFloatGraphs = nullptr);
	void InitializeDefaultFloat2GraphValues(std::vector<Float2Graph>* localFloat2Graphs = nullptr);
	void InitializeDefaultFloat3GraphValues(std::vector<Float3Graph>* localFloat3Graphs = nullptr);

	void SetFloatGraphDefaultValues(FloatGraphType floatGraphType, float defaultValue);
	void SetFloat2GraphDefaultValues(Float2GraphType float2GraphType, Vec2 defaultValue);
	void SetFloat3GraphDefaultValues(Float3GraphType float3GraphType, Vec3 defaultValue);

	static void SetFloatGraphDefaultValues(FloatGraph& floatGraph, float defaultValue);
	static void SetFloat2GraphDefaultValues(Float2Graph& floatGraph, Vec2 defaultValue);
	static void SetFloat3GraphDefaultValues(Float3Graph& floatGraph, Vec3 defaultValue);

	void ParseFloatGraphFromXML(std::vector<FloatGraph>& floatGraphs, XmlElement const& floatGraphElement);
	void ParseFloat2GraphFromXML(std::vector<Float2Graph>& float2Graphs, XmlElement const& float2GraphElement);
	void ParseFloat3GraphFromXML(std::vector<Float3Graph>& float3Graphs, XmlElement const& float3GraphElement);
	void ParseEmitterVelocityFromXML(XmlElement const& float3GraphElement);
	void ParseChildrenFromXML(XmlElement const& float3GraphElement);
};