#pragma once
#include "Game/EngineBuildPreferences.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec4.hpp"

class SpriteSheet;
class BitmapFont;
class Window;
class Texture;
class Shader;
class VertexBuffer;
class ConstantBuffer;
class Image;
class IndexBuffer;
class ComputeShader;
class UAV;
class GPUBuffer;
class SRV;
class StructuredBuffer;
class MtlLib;
class IndirectArgsBuffer;
class Material;
class GPUMesh;
class CPUMesh;

struct ID3D11RasterizerState;
struct ID3D11RenderTargetView;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11InputLayout;
struct ID3D11BlendState;
struct ID3D11SamplerState;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;
struct ID3D11DepthStencilState;
struct ID3D11ComputeShader;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11Buffer;
struct D3D11_MAPPED_SUBRESOURCE;

#define DX_SAFE_RELEASE(dxObject)	\
{									\
	if (dxObject != nullptr)		\
	{								\
		dxObject->Release();		\
		dxObject = nullptr;			\
	}								\
}

#if defined(OPAQUE)
#undef OPAQUE
#endif

enum class BlendMode
{
	ALPHA,
	ADDITIVE,
	OPAQUE,
	ADDITIVE_EMISSIVE,
	ALPHA_ORDER_INDEPENDENT_TRANSPACENCY,
	OIT_COMPOSITE,
	PREMULTIPLIED_ALPHA,
	COUNT
};

enum class SamplerMode
{
	POINT_CLAMP,
	BILINEAR_WRAP,
	BILINEAR_CLAMP,
	COUNT
};

enum class RasterizeMode
{
	SOLID_CULL_NONE,
	SOLID_CULL_BACK,
	WIREFRAME_CULL_NONE,
	WIREFRAME_CULL_BACK,
	COUNT
};

enum class DepthMode
{
	DISABLED,
	ENABLED,
	ENABLED_NO_WRITE,
	COUNT
};

enum class VertexType
{
	VERTEX_TYPE_PCU,
	VERTEX_TYPE_PCUTBN,
	VERTEX_TYPE_PARTICLE,
	VERTEX_TYPE_MESH_PARTICLE,
};

enum class PrimaryRenderTargetType
{
	SDR,
	HDR
};

struct RenderConfig
{
	Window* m_window = nullptr;
	bool m_enableImgui = false;
	bool m_hdrEnabled = false;
	float m_hdrExposure = 1.f;
};

struct LightConstants
{
	Vec3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
	Vec3 padding;
	Vec3 worldEyePosition;
	float minimumFalloff = 0.f;
	float maximumFalloff = 0.f;
	float minimumFalloffMultiplier = 0.f;
	float maximumFalloffMultiplier = 0.f;
	int renderAmbientDebugFlag = 1;
	int renderDiffuseDebugFlag = 1;
	int renderSpecularDebugFlag = 1;
	int renderEmissiveDebugFlag = 1;
	int useDiffuseMapDebugFlag = 1;
	int useNormalMapDebugFlag = 1;
	int useSpecularMapDebugFlag = 1;
	int useGlossinessMapDebugFlag = 1;
	int useEmissiveMapDebugFlag = 1;
};


constexpr int MAX_NUM_POINT_LIGHTS = 512;

struct PointLight
{
	Vec3 position;
	float intensity = .0f;
	float linearAttenuation = 0.f;
	float exponentialAttenuation = 0.f;
	Vec2 pointLightPadding;
	Vec4 pointLightColor;
};

struct InderectArgs
{
	unsigned int xThreads = 1;
	unsigned int yThreads = 1;
	unsigned int zThreads = 1;
};

struct BlurSample
{
	Vec2 Offset;
	float Weight;
	int Padding;
};

static const int MaxSamples = 64;

struct BlurConstants
{
	Vec2 TexelSize;
	float LerpT;
	int NumSamples;
	BlurSample Samples[MaxSamples];
};

struct HDRConstants
{
	float m_exposure = 1.f;
	Vec3 padding;
};

class Renderer
{
public:
	Renderer(RenderConfig const& config);
	void StartUp();
	void BeginFrame() const;
	void EndFrame();
	void ShutDown();
	void ClearScreen(const Rgba8& clearColor) const;
	void SwapDepthTextures();
	void BeginCamera(Camera const& camera);
	void EndCamera(const Camera& camera) const;
	void DrawVertexArray(size_t numVertexes, const Vertex_PCU* const vertexes);
	void DrawVertexArray(size_t numVertexes, const Vertex_PCUTBN* const vertexes);
	void DrawVertexArrayIndexed(size_t numVertexes, const Vertex_PCUTBN* const vertexes, size_t numIndexes, const unsigned int* const indexes);
	void SetBlendMode(BlendMode blendMode);
	void SetSamplerMode(SamplerMode sampleMode);
	void SetPrimaryRenderTargetType(PrimaryRenderTargetType desiredRenderTargetType);
	Texture* CreateOrGetTextureFromFile(char const* imageFilePath);
	MtlLib* CreateOrGetMtlLibFromFile(char const* imageFilePath);
	BitmapFont* CreateOrGetBitmapFontFromFile(const char* bitmapFontFilePathWithNoExtension);
	void BindMaterial(Material* material);
	void BindTexture(Texture* texture);
	void BindSpecularGlossEmissiveMap(Texture* texture);
	void BindNormalMap(Texture* texture);
	Shader* GetShader(std::string shaderName);
	ComputeShader* GetComputeShader(std::string shaderName);
	Shader* CreateOrGetShaderFromFile(const char* shaderFilePath, VertexType vertexType = VertexType::VERTEX_TYPE_PCU);
	ComputeShader* CreateOrGetComputeShaderFromFile(const char* shaderFilePath);
	SpriteSheet* CreateOrGetSpriteSheetFromFile(const char* shaderFilePath, IntVec2 dimensions);
	Shader* CreateShader(char const* shaderName, VertexType vertType = VertexType::VERTEX_TYPE_PCU);
	Shader* CreateShader(char const* shaderName, char const* shaderSource, VertexType vertType = VertexType::VERTEX_TYPE_PCU);
	bool CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, 
		char const* source, char const* entryPoint, char const* target);
	void BindShader(Shader* shader);
	VertexBuffer* CreateVertexBuffer(const size_t size);
	VertexBuffer* CreateVertexBufferAsUAV(const size_t size);
	GPUBuffer* CreateRawBuffer(const size_t size, void* pInitData);
	IndirectArgsBuffer* CreateIndirectArgsbuffer(InderectArgs const& args);
	IndexBuffer* CreateIndexBuffer(const size_t size);
	void CopyCPUToGPU(void* data, size_t size, GPUBuffer*& gpuBuffer);
	void CopyCPUToGPU(const void* data, size_t size, VertexBuffer*& vbo);
	void CopyCPUToGPU(const void* data, size_t size, IndexBuffer*& ibo);
	void BindVertexBuffer(VertexBuffer* vbo, VertexType type = VertexType::VERTEX_TYPE_PCU);
	void BindIndexBuffer(IndexBuffer* ibo);
	void DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset = 0, VertexType type = VertexType::VERTEX_TYPE_PCU);
	void DrawVertexBufferIndexed(VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, VertexType type = VertexType::VERTEX_TYPE_PCU);
	ConstantBuffer* CreateConstantBuffer(const size_t size);
	void CopyCPUToGPU(const void* data, ConstantBuffer* cbo);
	void BindConstantBuffer(int slot, ConstantBuffer* cbo);
	void SetStatesIfChanged();
	void SetModelConstants(const Mat44& modelMatrix = Mat44(), const Rgba8& modelColor = Rgba8::WHITE);
	void SetLightingConstants(Vec3 const& sunDirection, float sunIntensity, float ambientIntensity);
	void SetLightingConstants(LightConstants const& lightConstants);
	void SetPointLights(int currNumPointLights, PointLight updatedPointLights[MAX_NUM_POINT_LIGHTS]);
	void SetRasterizeMode(RasterizeMode desiredMode);
	void SetDepthMode(DepthMode desiredDepthMode);
	ComputeShader* CreateComputeShader(const char* shaderName, const char* pFunctionName, 
		bool writeCompiledShaderToFile = false, std::string compiledShaderWritePath = "You should not default this argument!", bool readCompiledShader = false);
	StructuredBuffer* CreateStructuredBuffer( unsigned int uElementSize, unsigned int uCount, void const* pInitData, bool dynamic = false, bool canReadFrom = false);
	UAV* CreateUAV(GPUBuffer* gpuBuffer);
	SRV* CreateSRV(GPUBuffer* gpuBuffer);
	void* ReadFromBuffer(GPUBuffer* buffer);
	void UnmapBuffer(GPUBuffer* buffer);
	void* MapBufferForReadWrite(GPUBuffer* buffer);
	void MapBufferForWrite(GPUBuffer* buffer, D3D11_MAPPED_SUBRESOURCE* mappedResource);

	void UnmapBuffer(ID3D11Buffer* buffer);
	void SetCameraConstants(Camera const& camera);
	void BindPrevFrameDepthTextureToTRegister(unsigned int tReg);
	void RunComputeShader(ComputeShader* computeShader,
		std::vector<SRV*> const& srvs,
		std::vector<UAV*> const& uavs,
		unsigned int X, unsigned int Y, unsigned int Z);
	void SetComputeShaderSRVsAndUAVs(std::vector<SRV*> const& srvs, std::vector<UAV*> const& uavs);
	void RunComputeShader(ComputeShader* computeShader, unsigned int X, unsigned int Y, unsigned int Z);
	void RunComputeShader(ComputeShader* computeShader,
		std::vector<SRV*> const& srvs,
		std::vector<UAV*> const& uavs,
		IndirectArgsBuffer* indirectArgs);
	void ClearSRVSAndUAVS(unsigned int numSRVS, unsigned int numUAVS);
	void CreateEmissiveTextures();
	void RenderEmissive();
	void CompositeOIT();
	Texture* CreateTextureFromImage(const Image& image, bool renderTargetTexture = false, bool addToTextureList = false, bool recreateExistingTexture = false, bool use16BitFormat = false);
	void CreateOITRenderTargets();
	void SetPSSRVs(std::vector<SRV*> srvsToSet, int startSlot);
	void BindSRVsForTextureArray(std::vector<Texture*> textures, int startSlot);
	void BindTextureToSlot(Texture* texture, int startSlot);
	void BindSRVToSlot(SRV* srv, int startSlot);
	void InitializeHDR();
	void CompositeHDR();
	GPUMesh* CreateOrGetGPUMeshFromFile(std::string const& path, Mat44 const& transform);
	GPUMesh* CreateGPUMeshFromFile(std::string const& path, Mat44 const& transform);
	CPUMesh* CreateOrGetCPUMeshFromFile(std::string const& path, Mat44 const& transform);
	CPUMesh* CreateCPUMeshFromFile(std::string const& path, Mat44 const& transform);

protected:
	void* m_dxgiDebug = nullptr;
	void* m_dxgiDebugModule = nullptr;

	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
	IDXGISwapChain* m_swapChain = nullptr;
	VertexBuffer* m_immediateVBO = nullptr;
	VertexBuffer* m_immediateTBNVBO = nullptr;
	IndexBuffer* m_immediateIBO;
	ConstantBuffer* m_cameraCBO = nullptr;
	ConstantBuffer* m_modelCBO = nullptr;
	ConstantBuffer* m_lightingCBO = nullptr;
	ConstantBuffer* m_pointLightsCBO = nullptr;
	ConstantBuffer* m_blurCBO = nullptr;
	ConstantBuffer* m_hdrCBO = nullptr;
	ID3D11BlendState* m_blendState = nullptr;
	BlendMode m_desiredBlendMode = BlendMode::ALPHA;
	ID3D11BlendState* m_blendStates[(int) (BlendMode::COUNT)] = {};

	ID3D11SamplerState* m_samplerState = nullptr;
	SamplerMode m_desiredSamplermode = SamplerMode::POINT_CLAMP;
	ID3D11SamplerState* m_samplerStates[(int)(SamplerMode::COUNT)] = {};

	ID3D11RasterizerState* m_rasterizeStates[(int)RasterizeMode::COUNT] = {};
	RasterizeMode m_desiredRasterizerMode = RasterizeMode::SOLID_CULL_BACK;
	ID3D11RasterizerState* m_rasterizeState = nullptr;

	DepthMode m_desiredDepthMode = DepthMode::ENABLED;
	ID3D11DepthStencilState* m_depthMode = nullptr;
	ID3D11DepthStencilState* m_depthModes[(int)DepthMode::COUNT] = {};

	ID3D11DepthStencilView* m_depthStencilView = nullptr;
	ID3D11DepthStencilView* m_prevFrameDepthStencilView = nullptr;
	ID3D11Texture2D* m_depthStencilTexture = nullptr;
	ID3D11ShaderResourceView* m_depthStencilSRV = nullptr;
	ID3D11Texture2D* m_prevFrameDepthStencilTexture = nullptr;
	ID3D11ShaderResourceView* m_prevFrameDepthStencilSRV = nullptr;

	PrimaryRenderTargetType m_desiredRenderTargetType = PrimaryRenderTargetType::SDR;
	PrimaryRenderTargetType m_primaryRenderTargetType = PrimaryRenderTargetType::SDR;

	std::vector<Shader*> m_loadedShaders;
	std::vector<ComputeShader*> m_loadedComputeShaders;
	Shader* m_defaultShader = nullptr;
	Shader* m_currentShader = nullptr;
	Texture* m_defaultTexture = nullptr;
	Texture* m_currentTexture = nullptr;
	Texture* m_currentSpecularGlossEmissiveMap = nullptr;
	Texture* m_currentNormalMap = nullptr;

	Texture* m_emissiveRenderTexture = nullptr;
	Texture* m_blurredEmissiveRenderTexture = nullptr;
	std::vector<Texture*> m_blurDownTextures;
	std::vector<Texture*> m_blurUpTextures;
	VertexBuffer* m_fullScreenQuadNDCSpace = nullptr;

	//Order Independent Transparency
	Texture* m_OITAccumulationTexture = nullptr;
	Texture* m_OITRevealageTexture = nullptr;
	Texture* m_OITEmissiveAccumulationTexture = nullptr;
	Texture* m_HDRTexture = nullptr;


private:
	// private internal member functions will go here
	Texture* CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData);
	Texture* CreateTextureFromFile(char const* imageFilePath, bool renderTargetTexture = false);
	MtlLib* CreateMtlLibFromFile(char const* imageFilePath);
	Texture* GetTextureForFileName(std::string imageFilepath);
	BitmapFont* GetBitmapFontForFileName(std::string bitmapFontFilePathWithNoExtension);
	SpriteSheet* GetSpriteSheetForFileName(std::string spriteSheetFilepath);
	MtlLib* GetMtlLibForFileName(std::string mtlLibFilePath);
	ID3D11RenderTargetView* GetPrimaryRenderTargetView() const;
private:
	RenderConfig m_config;
	std::vector<Texture*> m_loadedTextures;
	std::vector<MtlLib*> m_loadedMtlLibs;
	std::vector<SpriteSheet*> m_loadedSpriteSheets;
	std::vector<BitmapFont*> m_loadedFonts;
	std::vector<CPUMesh*> m_loadedCPUMeshes;
	std::vector<GPUMesh*> m_loadedGPUMeshes;
	static const int k_HDRConstantsSlot = 7;
};

//void SetDebugName(ID3D11DeviceChild* object, char const* name);
