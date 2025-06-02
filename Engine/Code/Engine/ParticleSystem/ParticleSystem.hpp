#pragma once
#include "Engine/ParticleSystem/ParticleEffect.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/ParticleSystem/ParticlePhysicsObject.hpp"

class SRV;
class UAV;
class StructuredBuffer;
class Texture;
class GPUBuffer;
struct ParticleEmitterDefinition;
class ConstantBuffer;
class IndividualParticleEffect;
extern ParticleSystem* g_theParticleSystem;

constexpr unsigned int MAX_PARTICLES_PER_EMITTER = 4194240;

struct ParticleSystemConfig
{
	int m_maxParticles = MAX_PARTICLES_PER_EMITTER;
	IntVec2 m_spriteAtlasResolution = IntVec2(1, 1);
	Camera* m_playerCamera = nullptr;
	bool m_useImGUI = true;
	bool m_writeOutCompiledShaders = false;
	bool m_readInCompiledShaders = true;
};

struct ParticleConstants
{
	float m_deltaSeconds;
	Vec3 m_playerPosition;

	int m_spriteAtlasDimensions[2] = { 1, 1 };
	int m_frameCount;
	unsigned int m_emittedParticles;

	unsigned int m_maxParticles = MAX_PARTICLES_PER_EMITTER;
	unsigned int m_numEmitterConfigs = 0;
	float m_timeElapsed = 0.f;
	unsigned int m_numPhysicsObjects = 0;

	int m_windowDimensionsX = 0;
	int m_windowDimensionsY = 0;
	float m_nearPlane = 0.f;
	float m_farPlane = 0.f;

	Mat44 m_inverseViewProjMatrix;

	unsigned int m_numPhysicsAABB3s;
	Vec3 m_cameraIBasis;
};

struct CounterBuffer
{
	unsigned int aliveCount = 0;
	unsigned int aliveCountAfterSim = 0;
	unsigned int deadCount = 0;
	unsigned int drawCount = 0;

	unsigned int drawMeshCount = 0;
	Vec3 padding;
};

struct SortConstants
{
	unsigned int numElements = 0;
	int jobParamsX = 1;
	int jobParamsY = 1;
	int jobParamsZ = 1;
};

struct SpriteSheetEntry
{
	AABB2 m_uvBounds;
	std::string m_imageFilePath;
};

enum class EmissionType
{
	SPHERE = 0,
	BOX,
	MESH
};

enum class EmissionMode
{
	CONTINUOUS,
	BURST
};


struct MeshParticleVert
{
	MeshParticleVert() = default;
	MeshParticleVert(Vec3 const& position, Vec2 const& uv, Vec3 const& normal);
	Vec3 m_position;
	float padding = 0.f;

	Vec2 m_uv;
	Vec2 padding2;

	Vec3 m_normal;
	float padding3 = 0.f;
};

struct MeshEntry
{
	unsigned int m_size = 0;
	unsigned int m_offset = 0;
	Vec2 padding;
};

struct LoadedMeshEntry
{
	std::string m_name = "";
	int m_meshEntryIndex = 0;
};

class ParticleSystem
{
	friend class ParticleEmitter;
	friend class ParticlePhysicsObject;
	friend class ParticleEffect;
	friend struct ParticleEmitterDefinition;
	friend struct ParticleEffectDefinition;
	friend struct ParticleEmitterInitializer;
public:
	ParticleSystem(ParticleSystemConfig const& config);
	void Update(float deltaSeconds);
	void Render() const;
	void BeginFrame();
	void EndFrame();
	void Startup();
	void Shutdown();

	void ToggleParticleEditor(bool enableEditor);
	void ToggleParticleEditor();
	bool IsShowingEditor() const;
	void KillAllEmitters();

	AABB2 GetImageBoundsInSpriteAtlas(std::string imageFilePath);
	ParticleEffect* AddParticleEffectByFileName(std::string effectName, Vec3 const& position, Mat44 const& orientation, bool playImmediately = true, float playDuration = -1.f);
	ParticleEffect* AddParticleEffectByFileName(std::string effectName, Vec3 const& position, EulerAngles const& orientation = EulerAngles(), bool playImmediately = true, float playDuration = -1.f, bool useWorldTransform = false);
	void PlayParticleEffectByFileName(std::string effectName, Vec3 const& position, Mat44 const& orientation, float duration);
	void PlayParticleEffectByFileName(std::string effectName, Vec3 const& position, EulerAngles const& orientation, float duration);

	ParticlePhysicsObject* AddParticlePhysicsObject(Vec3 position, float radius, float forceMagnitude, float fallofExp = 2.f, bool attract = false);
	ParticlePhysicsAABB3* AddParticlePhysicsAABB3(Vec3 const& mins, Vec3 const& maxs);
	int AddParticleEmitterToSystem(ParticleEmitterDefinition const& definition, ParticleEmitter* emitter);
	void LoadEffectByFileName(std::string fileName);

public:
	ParticleSystemConfig m_config;
	unsigned int m_aliveCountFromlastFrame = 0;
	unsigned int m_deadCountFromLastFrame = 0;
	unsigned int m_culledCountLastFrame = 0;
	unsigned int m_drawCount = 0;
	unsigned int m_meshParticleDrawCount = 0;
	unsigned int m_totalMeshVertCount = 0;
	Texture* m_spriteAtlasTexture = nullptr;

private:
	//UI
	void UpdateIMGUI();
	void UpdatePlayPanelIMGUI();
	void UpdateParticleSystemIMGUI();
	void UpdateParticleEmitterIMGUI(ParticleEmitter* currentlyEditedEmitter, int& index);
	void UpdateSubEmittersIMGUI(ParticleEmitter* currentlyEditedEmitter, int& index, EmitterUpdateDefinitionGPU* updateDef,
		EmitterRenderDefinitionGPU* renderDef, EmitterInstanceGPU* emitterInstanceGPU, EmitterInstanceCPU* emitterInstanceCPU, int emitterDefIdx);
	void UpdateNormalParticleEmitterIMGUI(ParticleEmitter* currentlyEditedEmitter, int& index, EmitterUpdateDefinitionGPU* updateDef,
		EmitterRenderDefinitionGPU* renderDef, EmitterInstanceGPU* emitterInstanceGPU, EmitterInstanceCPU* emitterInstanceCPU, int emitterDefIdx);
	void UpdateColorOverLifetime(EmitterUpdateDefinitionGPU* emitterConfigGPU);
	void UpdateChildEmitters(ParticleEmitter* currentlyEditedEmitter, int emitterDefIdx);

	void CustomInputFloat(char const* fieldName, FloatGraph& fieldGraph, float defaultConstant);
	void CustomInputFloat2(char const* fieldName, Float2Graph& fieldGraph, Vec2 defaultConstant);
	void CustomInputFloat3(char const* fieldName, Float3Graph& fieldGraph, Vec3 defaultConstant);

	//Emitter Update
	void InitResources();
	void UpdateLiveEmitters(float deltaSeconds);
	void UpdateEmitterSRVData();
	void CreatePhysicsObjectsSRV();
	void EmitParticles(float deltaSeconds);
	void UpdateParticles(float deltaSeconds);
	void SortParticlesGPU(int maxCount, SRV* comparisonList, UAV* indexList);

	//XML loading
	ParticleEffectDefinition CreateOrGetEffectDefinitionFromFile(std::string const& fileName);
	ParticleEffectDefinition CreateEffectDefinitionFromFile(std::string const& fileName);
	int CreateOrGetEmitterDefinitionFromFile(std::string const& fileName);
	int CreateEmitterDefinitionFromFile(std::string const& fileName);
	void SaveCurrentlyEditedEffectAsXML();

	//Sprite atlas
	bool DoImageBoundsFitInSpriteAtlas(AABB2 const& imageBoundsUV);
	void CopyImageToSpriteAtlas(Image const& imageToAdd, AABB2 const& imageBoundsUV);
	int CreateOrGetMeshParticleEntry(std::string fileName);


	//helper
	//int GetVacantIndividualDefinitionEntryIndex();
	void InitializeDefaultEffectValues();
	void SetDefaultValueForFloatGraph(FloatGraph* floatGraph, float defaultValue);
	void SetDefaultValueForFloat2Graph(Float2Graph* floatGraph, Vec2 defaultValue);
	void SetDefaultValueForFloat3Graph(Float3Graph* floatGraph, Vec3 defaultValue);
	int EmplaceDefinitionDataForEmitter();
	Vec3 GetValueInFloat3Graph(Float3Graph& float3Graph, float normalizedTime, unsigned int seed, bool seperateAxis = true);
	Vec3 GetFloat3NoiseInRange(Vec3 min, Vec3 max, int index, unsigned int seed, bool seperateAxis = true);
private:
	unsigned int m_totalParticles = 0;
	unsigned int m_totalParticlesLastFrame = 0;
	int m_frameCount = 0;
	unsigned int m_currentSeed = 0;

	std::vector<EmitterUpdateDefinitionGPU> m_updateDefinitions;
	unsigned int m_prevUpdateDefEntries = 0;
	std::vector<EmitterRenderDefinitionGPU> m_renderDefinitions;
	unsigned int m_prevRenderDefEntries = 0;
	std::vector<EmitterInstanceGPU> m_liveEmitterInstancesGPU;
	std::vector<EmitterInstanceCPU> m_liveEmitterInstancesCPU;
	std::vector<ParticlePhysicsObjectGPU> m_livePhysicsObjects;
	std::vector<ParticlePhysicsAABB3GPU> m_livePhysicsAABB3s;

	//float graphs
	std::vector<FloatGraph> m_floatGraphs;
	unsigned int m_prevFloatGraphEntries = 0;
	std::vector<Float2Graph> m_float2Graphs;
	unsigned int m_prevFloat2GraphEntries = 0;
	std::vector<Float3Graph> m_float3Graphs;
	unsigned int m_prevFloat3GraphEntries = 0;

	std::vector<Float3Graph> m_emitterVelocityGraphs;


	std::vector<ParticleEffectDefinition> m_loadedEffectDefinitions;
	std::vector<ParticleEmitterDefinition> m_loadedEmitterDefinitions;

	//mesh data
	std::vector<MeshEntry> m_meshParticleEntriesGPU;
	std::vector<MeshParticleVert> m_meshParticleVerts;
	std::vector<LoadedMeshEntry> m_loadedMeshParticleEntries;

	std::vector<SpriteSheetEntry> m_spriteSheetEntries;
	Image m_spriteAtlasImage;

	bool m_showEditor = true;
	ParticleEffect* m_currentlyEditedEffect = nullptr;
	//ParticleEffectDefinition m_currentlyEditedEffectDefinition;
	bool m_debugDraw = true;

	static const int k_particleConstantsSlot = 5;
	static const int k_sortConstantsSlot = 6;
	int m_emitThreadsToSpawn = 0;

	ParticleConstants m_particleConstants;
	ConstantBuffer* m_particleConstantsCBO = nullptr;
	ConstantBuffer* m_sortConstantsCBO = nullptr;
	UAV* m_particlesUAV = nullptr;
	SRV* m_particlesSRV = nullptr;
	StructuredBuffer* m_particlesBuffer = nullptr;
	UAV* m_particleAliveList1UAV = nullptr;
	SRV* m_particleAliveList1SRV = nullptr;

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

	// mesh particle copy of distance and draw lists
	UAV* m_meshParticleDistanceUAV = nullptr;
	SRV* m_meshParticleDistanceSRV = nullptr;
	StructuredBuffer* m_meshParticleDistanceBuffer = nullptr;
	StructuredBuffer* m_meshParticleDrawListBuffer = nullptr;
	UAV* m_meshParticleDrawListUAV = nullptr;
	SRV* m_meshParticleDrawListSRV = nullptr;
	VertexBuffer* m_meshVBO = nullptr;
	UAV* m_meshVBOUAV = nullptr;

	//vertex count data for meshes
	UAV* m_meshOffsetListUAV = nullptr;
	SRV* m_meshOffsetListSRV = nullptr;
	StructuredBuffer* m_meshOffsetListBuffer = nullptr;

	StructuredBuffer* m_updateDefBuffer = nullptr;
	SRV* m_updateDefSRV = nullptr;
	StructuredBuffer* m_renderDefBuffer = nullptr;
	SRV* m_renderDefSRV = nullptr;
	StructuredBuffer* m_emitterInstanceBuffer = nullptr;
	SRV* m_emitterInstanceSRV = nullptr;

	StructuredBuffer* m_floatGraphsBuffer = nullptr;
	SRV* m_floatGraphsSRV = nullptr;
	StructuredBuffer* m_float2GraphsBuffer = nullptr;
	SRV* m_float2GraphsSRV = nullptr;
	StructuredBuffer* m_float3GraphsBuffer = nullptr;
	SRV* m_float3GraphsSRV = nullptr;

	StructuredBuffer* m_physicsObjectsBuffer = nullptr;
	SRV* m_physicsObjectsSRV = nullptr;

	StructuredBuffer* m_physicsAABB3Buffer = nullptr;
	SRV* m_physicsAABB3SRV = nullptr;

	StructuredBuffer* m_meshParticleEntryStructuredBuffer = nullptr;
	SRV* m_meshParticleEntrySRV = nullptr;

	StructuredBuffer* m_meshParticleVertsStructuredBuffer = nullptr;
	SRV* m_meshParticleVertsSRV = nullptr;

	RandomNumberGenerator* m_rng = nullptr;

	std::vector<ParticleEffect*> m_particleEffectsOwnedBySystem;
};