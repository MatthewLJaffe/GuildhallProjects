#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)

#include "Engine/Renderer/Renderer.hpp"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#if defined(ENGINE_DEBUG_RENDER)
#include "dxgidebug.h"
#pragma comment(lib, "dxguid.lib")
#endif

#include "ThirdParty/stb/stb_image.h"
#include "ThirdParty/imgui/imgui_impl_dx11.h"

#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/DefaultShader.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/ComputeShader.hpp"
#include "Engine/Renderer/UAV.hpp"
#include "Engine/Renderer/SRV.hpp"
#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/MtlLib.hpp"
#include "Engine/Renderer/IndirectArgsBuffer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_Particle.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Core/Vertex_MeshParticle.hpp"

struct CameraConstants
{
	Mat44 ViewMatrix;
	Mat44 ProjectionMatrix;
};

struct ModelConstants
{
	Mat44 modelMatrix;
	Vec4 modelColor;
};

struct PointLightConstants
{
	unsigned int numPointLights;
	Vec3 pointLightsPadding;
	PointLight pointLights[MAX_NUM_POINT_LIGHTS];
};


static const int k_lightConstantsSlot = 1;
static const int k_cameraConstantsSlot = 2;
static const int k_modelConstantsSlot = 3;
static const int k_pointLightConstantsSlot = 4;
static const int k_blurConstantsSlot = 6;

Renderer::Renderer(RenderConfig const& config)
	: m_config(config)
{}


void Renderer::DrawVertexArray(size_t numVertexes, Vertex_PCU const* vertexes)
{
	CopyCPUToGPU(vertexes, numVertexes * sizeof(Vertex_PCU), m_immediateVBO);
	DrawVertexBuffer(m_immediateVBO, (int)numVertexes, 0);
}

void Renderer::DrawVertexArray(size_t numVertexes, const Vertex_PCUTBN* const vertexes)
{
	CopyCPUToGPU(vertexes, numVertexes * sizeof(Vertex_PCUTBN), m_immediateTBNVBO);
	DrawVertexBuffer(m_immediateTBNVBO, (int)numVertexes, 0, VertexType::VERTEX_TYPE_PCUTBN);
}

void Renderer::DrawVertexArrayIndexed(size_t numVertexes, const Vertex_PCUTBN* const vertexes, size_t numIndexes, const unsigned int* const indexes)
{
	CopyCPUToGPU(vertexes, numVertexes * sizeof(Vertex_PCUTBN), m_immediateTBNVBO);
	CopyCPUToGPU(indexes, numIndexes * sizeof(unsigned int), m_immediateIBO);
	DrawVertexBufferIndexed(m_immediateTBNVBO, m_immediateIBO, (int)numIndexes, VertexType::VERTEX_TYPE_PCUTBN);
}

void Renderer::SetBlendMode(BlendMode blendMode)
{
	m_desiredBlendMode = blendMode;
}

void Renderer::SetSamplerMode(SamplerMode sampleMode)
{
	m_desiredSamplermode = sampleMode;
}

void Renderer::SetPrimaryRenderTargetType(PrimaryRenderTargetType desiredRenderTargetType)
{
	m_desiredRenderTargetType = desiredRenderTargetType;
}

void Renderer::StartUp()
{
	XmlDocument effectConfigDocument;
	GUARANTEE_OR_DIE(effectConfigDocument.LoadFile("Data/GameConfig.xml") == 0, Stringf("Failed to load effect config file Data/GameConfig.xml"));
	XmlElement* rootElement = effectConfigDocument.FirstChildElement("GameConfig");
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root element for effect config Data/GameConfig.xml"));
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*rootElement);

	//Create debug module
#if defined(ENGINE_DEBUG_RENDER)
	m_dxgiDebugModule = (void*)::LoadLibraryA("dxgidebug.dll");
	if (m_dxgiDebugModule == nullptr)
	{
		ERROR_AND_DIE("Could not load dxgidebug.dll");
	}
	typedef HRESULT(WINAPI* GetDebugModuleCB)(REFIID, void**);
	((GetDebugModuleCB)::GetProcAddress((HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface"))
		(__uuidof(IDXGIDebug), &m_dxgiDebug);

	if (m_dxgiDebug == nullptr)
	{
		ERROR_AND_DIE("Could not load debug module.");
	}
#endif

	unsigned int deviceFlags = 0;
#if defined(ENGINE_DEBUG_RENDER)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	deviceFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;

	//create device swapchain and device context
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferDesc.Width = m_config.m_window->GetClientDimensions().x;
	swapChainDesc.BufferDesc.Height = m_config.m_window->GetClientDimensions().y;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = (HWND)m_config.m_window->GetHwnd();
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags,
		nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, nullptr, &m_deviceContext);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create D3D 11 device and swap chain.");
	}

	// Get back buffer texture
	ID3D11Texture2D* backbuffer;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not get swap chain buffer.");
	}

	hr = m_device->CreateRenderTargetView(backbuffer, NULL, &m_renderTargetView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could create render target view for swap chain buffer.");
	}

	DX_SAFE_RELEASE(backbuffer);

	m_defaultShader = CreateShader("Default", DEFAULT_SHADER_SOURCE);
	BindShader(nullptr);

	//Create immediate VBO
	m_immediateVBO = CreateVertexBuffer(sizeof(Vertex_PCU));
	m_immediateTBNVBO = CreateVertexBuffer(sizeof(Vertex_PCUTBN));
	m_immediateIBO = CreateIndexBuffer(sizeof(unsigned int));

	//create constant buffer for camera constraints
	m_cameraCBO = CreateConstantBuffer(sizeof(CameraConstants));
	m_modelCBO = CreateConstantBuffer(sizeof(ModelConstants));
	m_lightingCBO = CreateConstantBuffer(sizeof(LightConstants));
	m_pointLightsCBO = CreateConstantBuffer(sizeof(PointLight) * MAX_NUM_POINT_LIGHTS + sizeof(unsigned int) + sizeof(Vec3));
	m_hdrCBO = CreateConstantBuffer(sizeof(HDRConstants));
	//create blend states
	D3D11_BLEND_DESC blendDesc = { };
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_device->CreateBlendState(&blendDesc, &m_blendStates[(int) BlendMode::OPAQUE]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateBlendState for BlendMode::OPAQUE failed.");
	}

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	hr = m_device->CreateBlendState(&blendDesc, &m_blendStates[(int)BlendMode::ALPHA]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateBlendState for BlendMode::ALPHA failed.");
	}

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	hr = m_device->CreateBlendState(&blendDesc, &m_blendStates[(int)BlendMode::ADDITIVE]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateBlendState for BlendMode::ADDITIVE failed.");
	}

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	hr = m_device->CreateBlendState(&blendDesc, &m_blendStates[(int)BlendMode::ADDITIVE_EMISSIVE]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateBlendState for BlendMode::ADDITIVE_EMISSIVE failed.");
	}

	D3D11_BLEND_DESC alphaOITBlendDesc = { };
	//alpha
	alphaOITBlendDesc.IndependentBlendEnable = true;
	alphaOITBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	alphaOITBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	alphaOITBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	alphaOITBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	alphaOITBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	alphaOITBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	alphaOITBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	alphaOITBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	//emissive
	alphaOITBlendDesc.RenderTarget[1].BlendEnable = TRUE;
	alphaOITBlendDesc.RenderTarget[1].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	alphaOITBlendDesc.RenderTarget[1].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	alphaOITBlendDesc.RenderTarget[1].BlendOp = D3D11_BLEND_OP_ADD;
	alphaOITBlendDesc.RenderTarget[1].SrcBlendAlpha = D3D11_BLEND_ONE;
	alphaOITBlendDesc.RenderTarget[1].DestBlendAlpha = D3D11_BLEND_ZERO;
	alphaOITBlendDesc.RenderTarget[1].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	alphaOITBlendDesc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	//accumulation (color)
	alphaOITBlendDesc.RenderTarget[2].BlendEnable = TRUE;
	alphaOITBlendDesc.RenderTarget[2].SrcBlend = D3D11_BLEND_ONE;
	alphaOITBlendDesc.RenderTarget[2].DestBlend = D3D11_BLEND_ONE;
	alphaOITBlendDesc.RenderTarget[2].BlendOp = D3D11_BLEND_OP_ADD;
	alphaOITBlendDesc.RenderTarget[2].SrcBlendAlpha = D3D11_BLEND_ONE;
	alphaOITBlendDesc.RenderTarget[2].DestBlendAlpha = D3D11_BLEND_ONE;
	alphaOITBlendDesc.RenderTarget[2].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	alphaOITBlendDesc.RenderTarget[2].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	//revealage
	alphaOITBlendDesc.RenderTarget[3].BlendEnable = TRUE;
	alphaOITBlendDesc.RenderTarget[3].SrcBlend = D3D11_BLEND_ZERO;
	alphaOITBlendDesc.RenderTarget[3].DestBlend = D3D11_BLEND_INV_SRC_COLOR;
	alphaOITBlendDesc.RenderTarget[3].BlendOp = D3D11_BLEND_OP_ADD;
	alphaOITBlendDesc.RenderTarget[3].SrcBlendAlpha = D3D11_BLEND_ZERO;
	alphaOITBlendDesc.RenderTarget[3].DestBlendAlpha = D3D11_BLEND_ZERO;
	alphaOITBlendDesc.RenderTarget[3].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	alphaOITBlendDesc.RenderTarget[3].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	//accumulation (emissive)
	alphaOITBlendDesc.RenderTarget[4].BlendEnable = TRUE;
	alphaOITBlendDesc.RenderTarget[4].SrcBlend = D3D11_BLEND_ONE;
	alphaOITBlendDesc.RenderTarget[4].DestBlend = D3D11_BLEND_ONE;
	alphaOITBlendDesc.RenderTarget[4].BlendOp = D3D11_BLEND_OP_ADD;
	alphaOITBlendDesc.RenderTarget[4].SrcBlendAlpha = D3D11_BLEND_ONE;
	alphaOITBlendDesc.RenderTarget[4].DestBlendAlpha = D3D11_BLEND_ONE;
	alphaOITBlendDesc.RenderTarget[4].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	alphaOITBlendDesc.RenderTarget[4].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_device->CreateBlendState(&alphaOITBlendDesc, &m_blendStates[(int)BlendMode::ALPHA_ORDER_INDEPENDENT_TRANSPACENCY]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateBlendState for BlendMode::ALPHA_ORDER_INDEPENDENT_TRANSPACENCY failed.");
	}

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_device->CreateBlendState(&blendDesc, &m_blendStates[(int)BlendMode::OIT_COMPOSITE]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateBlendState for BlendMode::OIT_COMPOSITE failed.");
	}

	D3D11_BLEND_DESC premultipliedAlphaBlendDesc = { };
	premultipliedAlphaBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	premultipliedAlphaBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	premultipliedAlphaBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	premultipliedAlphaBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	premultipliedAlphaBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	premultipliedAlphaBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	premultipliedAlphaBlendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
	premultipliedAlphaBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_device->CreateBlendState(&premultipliedAlphaBlendDesc, &m_blendStates[(int)BlendMode::PREMULTIPLIED_ALPHA]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateBlendState for BlendMode::PREMULTIPLIED_ALPHA failed.");
	}

	std::string defaultTextureName;
	uint8_t whiteTexels [16];
	for (int i = 0; i < 16; i++)
	{
		whiteTexels[i] = 255;
	}
	Image defaultTextureImage;
	defaultTextureImage.SetImageFilePath("Default");
	defaultTextureImage.SetDimensions(IntVec2(2, 2));

	std::vector<Rgba8> defaultTexels;
	for (int i = 0; i < 4; i++)
	{
		defaultTexels.push_back(Rgba8::WHITE);
	}
	defaultTextureImage.SetTexels(defaultTexels);

	m_defaultTexture = CreateTextureFromImage(defaultTextureImage);
	m_loadedTextures.push_back(m_defaultTexture);
	BindTexture(nullptr);

	//set sample states for texture
	D3D11_SAMPLER_DESC samplerDesc = { };
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerStates[(int) SamplerMode::POINT_CLAMP]);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateSamplerState for SamplerMode::POINT_CLAMP failed");
	}

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerStates[(int)SamplerMode::BILINEAR_WRAP]);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerStates[(int)SamplerMode::BILINEAR_CLAMP]);


	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateSamplerState for SamplerMode::BILINEAR_WRAP failed");
	}

	m_samplerState = m_samplerStates[(int)SamplerMode::POINT_CLAMP];
	m_deviceContext->PSSetSamplers(0, 1, &m_samplerState);
	m_deviceContext->CSSetSamplers(0, 1, &m_samplerState);

	//Create rasterize states
	D3D11_RASTERIZER_DESC rasterizeDesc = { };
	rasterizeDesc.FillMode = D3D11_FILL_SOLID;
	rasterizeDesc.CullMode = D3D11_CULL_NONE;
	rasterizeDesc.FrontCounterClockwise = true;
	rasterizeDesc.DepthClipEnable = true;
	rasterizeDesc.AntialiasedLineEnable = true;
	hr = m_device->CreateRasterizerState(&rasterizeDesc, &m_rasterizeStates[(int)RasterizeMode::SOLID_CULL_NONE]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateSamplerState for RasterizeMode::SOLID_CULL_NONE failed");
	}

	rasterizeDesc.FillMode = D3D11_FILL_SOLID;
	rasterizeDesc.CullMode = D3D11_CULL_BACK;
	hr = m_device->CreateRasterizerState(&rasterizeDesc, &m_rasterizeStates[(int)RasterizeMode::SOLID_CULL_BACK]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateSamplerState for RasterizeMode::SOLID_CULL_BACK failed");
	}

	rasterizeDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizeDesc.CullMode = D3D11_CULL_NONE;
	hr = m_device->CreateRasterizerState(&rasterizeDesc, &m_rasterizeStates[(int)RasterizeMode::WIREFRAME_CULL_NONE]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateSamplerState for RasterizeMode::WIREFRAME_CULL_NONE failed");
	}

	rasterizeDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizeDesc.CullMode = D3D11_CULL_BACK;
	hr = m_device->CreateRasterizerState(&rasterizeDesc, &m_rasterizeStates[(int)RasterizeMode::WIREFRAME_CULL_BACK]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("CreateSamplerState for RasterizeMode::WIREFRAME_CULL_BACK failed");
	}
	m_rasterizeState = m_rasterizeStates[(int)RasterizeMode::SOLID_CULL_BACK];
	m_deviceContext->RSSetState(m_rasterizeState);

	//create depth stencil texture and view
	D3D11_TEXTURE2D_DESC textureDesc = { };
	textureDesc.Width = m_config.m_window->GetClientDimensions().x;
	textureDesc.Height = m_config.m_window->GetClientDimensions().y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count = 1;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
	srDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;

	hr = m_device->CreateTexture2D(&textureDesc, nullptr, &m_depthStencilTexture);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create texture from depth stencil");
	}
	hr = m_device->CreateTexture2D(&textureDesc, nullptr, &m_prevFrameDepthStencilTexture);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create texture from depth stencil");
	}

	hr = m_device->CreateDepthStencilView(m_depthStencilTexture, &dsvDesc, &m_depthStencilView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could create depth stencil view.");
	}
	hr = m_device->CreateDepthStencilView(m_prevFrameDepthStencilTexture, &dsvDesc, &m_prevFrameDepthStencilView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could create depth stencil view.");
	}

	hr = m_device->CreateShaderResourceView(m_depthStencilTexture, &srDesc, &m_depthStencilSRV);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create depth stencil srv");
	}
	hr = m_device->CreateShaderResourceView(m_prevFrameDepthStencilTexture, &srDesc, &m_prevFrameDepthStencilSRV);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create depth stencil srv");
	}

	//DISABLED
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { };
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthModes[(int)DepthMode::DISABLED]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create depth stencil state");
	}

	//ENABLED
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthModes[(int)DepthMode::ENABLED]);

	//ENABLED NO WRITE
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthModes[(int)DepthMode::ENABLED_NO_WRITE]);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create depth stencil state");
	}
	m_depthMode = m_depthModes[(int)DepthMode::ENABLED];

	//Set Model constants
	SetModelConstants();

	//set up renderer backend for ImGui
	if (m_config.m_enableImgui)
	{
		ImGui_ImplDX11_Init(m_device, m_deviceContext);
	}

	CreateEmissiveTextures();
	CreateOITRenderTargets();
	InitializeHDR();
}

void Renderer::BeginFrame() const
{
	ID3D11ShaderResourceView* srvs[] = { nullptr };

	m_deviceContext->CSSetShaderResources(7, 1, srvs);
	m_deviceContext->PSSetShaderResources(7, 1, srvs);

	ID3D11RenderTargetView* RTVs[] =
	{
		GetPrimaryRenderTargetView(), //GetPrimaryRenderTargetView()
		m_emissiveRenderTexture->m_renderTargetView,
	};
	m_deviceContext->OMSetRenderTargets(2, RTVs, m_depthStencilView);
}

void Renderer::EndFrame()
{
	SwapDepthTextures();
	//Present
	HRESULT hr;
	hr = m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		ERROR_AND_DIE("Device has been lost, applicaiton will now terminate.");
	}
}

void Renderer::ClearScreen(const Rgba8& clearColor) const
{
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	float colorAsFloats[4];
	float blackAsFloats[4] = { 0.f, 0.f, 0.f, 1.f };
	float clearAccumulation[4] = { 0.f, 0.f, 0.f, 0.f };
	float clearRevealage[4] = { 1.f, 0.f, 0.f, 0.f };

	clearColor.GetAsFloats(colorAsFloats);
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, colorAsFloats);

	if (m_config.m_hdrEnabled)
	{
		m_deviceContext->ClearRenderTargetView(m_HDRTexture->m_renderTargetView, colorAsFloats);
	}

	m_deviceContext->ClearRenderTargetView(m_emissiveRenderTexture->m_renderTargetView, blackAsFloats);
	m_deviceContext->ClearRenderTargetView(m_blurredEmissiveRenderTexture->m_renderTargetView, blackAsFloats);
	m_deviceContext->ClearRenderTargetView(m_OITAccumulationTexture->m_renderTargetView, clearAccumulation);
	m_deviceContext->ClearRenderTargetView(m_OITEmissiveAccumulationTexture->m_renderTargetView, clearAccumulation);
	m_deviceContext->ClearRenderTargetView(m_OITRevealageTexture->m_renderTargetView, clearRevealage);

	for (int i = 0; i < m_blurDownTextures.size(); i++)
	{
		m_deviceContext->ClearRenderTargetView( m_blurDownTextures[i]->m_renderTargetView, blackAsFloats);

		if (i != (int)m_blurDownTextures.size() - 1)
		{
			m_deviceContext->ClearRenderTargetView(m_blurUpTextures[i]->m_renderTargetView, blackAsFloats);
		}
	}
}

void Renderer::SwapDepthTextures()
{
	ID3D11Texture2D* tempThisFrameDepthTexture = m_depthStencilTexture;
	ID3D11ShaderResourceView* tempThisFrameDepthSRV = m_depthStencilSRV;
	ID3D11DepthStencilView* tempThisFrameDSV = m_depthStencilView;
	m_depthStencilTexture = m_prevFrameDepthStencilTexture;
	m_depthStencilSRV = m_prevFrameDepthStencilSRV;
	m_depthStencilView = m_prevFrameDepthStencilView;
	m_prevFrameDepthStencilTexture = tempThisFrameDepthTexture;
	m_prevFrameDepthStencilSRV = tempThisFrameDepthSRV;
	m_prevFrameDepthStencilView = tempThisFrameDSV;
}

void Renderer::BeginCamera(Camera const& camera)
{
	g_gameConfigBlackboard.SetValue("aspectRatio", Stringf("%f", camera.GetAspectRatio()));
	if (camera.m_mode == Camera::eMode_Orthographic)
	{
		g_gameConfigBlackboard.SetValue("screenWidth", Stringf("%f", camera.GetOrthoDimensions().x));
		g_gameConfigBlackboard.SetValue("screenHeight", Stringf("%f", camera.GetOrthoDimensions().y));
	}
	SetCameraConstants(camera);

	//SET THE VIEWPORT
	//Create a D3D11_VIEWPORT variable
	D3D11_VIEWPORT viewport = { 0 };
	AABB2 dxViewport = camera.GetDirectXViewort();
	viewport.TopLeftX = dxViewport.m_mins.x;
	viewport.TopLeftY = dxViewport.m_mins.y;

	Vec2 viewPortDimensions = camera.m_normalizedViewport.GetDimensions();
	viewPortDimensions.x *= (float)Window::GetTheWindowInstance()->GetClientDimensions().x;
	viewPortDimensions.y *= (float)Window::GetTheWindowInstance()->GetClientDimensions().y;
	viewport.Width = (float)viewPortDimensions.x;
	viewport.Height = (float)viewPortDimensions.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//Call ID3D11DeviceContext::RSSetViewports
	m_deviceContext->RSSetViewports(1, &viewport);
}

void Renderer::EndCamera(const Camera& camera) const
{
	UNUSED(camera);
}

Texture* Renderer::CreateOrGetTextureFromFile(char const* imageFilePath)
{
	// See if we already have this texture previously loaded
	Texture* existingTexture = GetTextureForFileName(imageFilePath);
	if (existingTexture)
	{
		return existingTexture;
	}

	// Never seen this texture before!  Let's load it.
	Texture* newTexture = CreateTextureFromFile(imageFilePath);
	return newTexture;
}

MtlLib* Renderer::CreateOrGetMtlLibFromFile(char const* imageFilePath)
{
	std::string filePath = imageFilePath;
	TrimString(filePath, '\n');
	// See if we already have this texture previously loaded
	MtlLib* existingMtlLib = GetMtlLibForFileName(filePath);
	if (existingMtlLib)
	{
		return existingMtlLib;
	}

	// Never seen this texture before!  Let's load it.
	MtlLib* newMtlLib = CreateMtlLibFromFile(filePath.c_str());
	return newMtlLib;
}

Texture* Renderer::GetTextureForFileName(std::string imageFilePath)
{
	for (size_t i = 0; i < m_loadedTextures.size(); i++)
	{
		if (m_loadedTextures[i]->m_name == imageFilePath)
		{
			return m_loadedTextures[i];
		}
	}
	return nullptr;
}

BitmapFont* Renderer::CreateOrGetBitmapFontFromFile(const char* bitmapFontFilePathWithNoExtension)
{
	// See if we already have this texture previously loaded
	BitmapFont* existingFont = GetBitmapFontForFileName(bitmapFontFilePathWithNoExtension);
	if (existingFont)
	{
		return existingFont;
	}

	// Never seen this texture before!  Let's load it.
	std::string fontFilePath = std::string(bitmapFontFilePathWithNoExtension) + ".png";
	BitmapFont* newBitmapFont = new BitmapFont(bitmapFontFilePathWithNoExtension, CreateTextureFromFile(fontFilePath.c_str()));
	m_loadedFonts.push_back(newBitmapFont);
	return newBitmapFont;
}

void Renderer::BindMaterial(Material* material)
{
	if (material == nullptr)
	{
		BindTexture(nullptr);
		return;
	}
	BindTexture(material->m_diffuseTexture);
	if (material->m_specGlossEmitTexture != nullptr)
	{
		BindSpecularGlossEmissiveMap(material->m_specGlossEmitTexture);
	}
	if (material->m_normalTexture != nullptr)
	{
		BindNormalMap(material->m_normalTexture);
	}
	BindShader(material->m_shader);
}

BitmapFont* Renderer::GetBitmapFontForFileName(std::string bitmapFontFilePathWithNoExtension)
{
	for (size_t i = 0; i < m_loadedFonts.size(); i++)
	{
		if (m_loadedFonts[i]->m_fontFilePathNameWithNoExtension == bitmapFontFilePathWithNoExtension)
		{
			return m_loadedFonts[i];
		}
	}
	return nullptr;
}

SpriteSheet* Renderer::GetSpriteSheetForFileName(std::string spriteSheetFilepath)
{
	for (size_t i = 0; i < m_loadedSpriteSheets.size(); i++)
	{
		if (m_loadedSpriteSheets[i]->GetTexture()->GetImageFilePath() == spriteSheetFilepath)
		{
			return m_loadedSpriteSheets[i];
		}
	}
	return nullptr;
}

MtlLib* Renderer::GetMtlLibForFileName(std::string mtlLibFilePath)
{
	for (size_t i = 0; i < m_loadedMtlLibs.size(); i++)
	{
		if (m_loadedMtlLibs[i]->m_filePath == mtlLibFilePath)
		{
			return m_loadedMtlLibs[i];
		}
	}
	return nullptr;
}

ID3D11RenderTargetView* Renderer::GetPrimaryRenderTargetView() const
{
	if (!m_config.m_hdrEnabled || m_primaryRenderTargetType == PrimaryRenderTargetType::SDR)
	{
		return m_renderTargetView;
	}
	else
	{
		return m_HDRTexture->m_renderTargetView;
	}
}

Texture* Renderer::CreateTextureFromFile(char const* imageFilePath, bool renderTargetTexture)
{
	Texture* newTexture = CreateTextureFromImage(Image(imageFilePath), renderTargetTexture);
	m_loadedTextures.push_back(newTexture);
	return newTexture;
}

MtlLib* Renderer::CreateMtlLibFromFile(char const* imageFilePath)
{
	return new MtlLib(imageFilePath);
}

Texture* Renderer::CreateTextureFromImage(const Image& image, bool renderTargetTexture, bool addToTextureList, bool recreateExistingTexture, bool use16BitFormat)
{
	// Check if the load was successful
	GUARANTEE_OR_DIE(image.GetRawData(), Stringf("CreateTextureFromData failed for \"%s\" - texelData was null!", image.GetImageFilePath().c_str()));
	GUARANTEE_OR_DIE(image.GetDimensions().x > 0 && image.GetDimensions().y > 0, 
		Stringf("CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", image.GetImageFilePath().c_str(), image.GetDimensions().x, image.GetDimensions().y));

	Texture* newTexture = new Texture();
	newTexture->m_name = image.GetImageFilePath(); // NOTE: m_name must be a std::string, otherwise it may point to temporary data!
	newTexture->m_dimensions = image.GetDimensions();

	D3D11_TEXTURE2D_DESC textureDesc = { };
	textureDesc.Width = newTexture->m_dimensions.x;
	textureDesc.Height = newTexture->m_dimensions.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	if (use16BitFormat)
	{
		textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	}
	else
	{
		if (m_config.m_hdrEnabled)
		{
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		}
		else
		{
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
	}
	textureDesc.SampleDesc.Count = 1;
	if (!renderTargetTexture)
	{
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	}
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (renderTargetTexture)
	{
		textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	}
	D3D11_SUBRESOURCE_DATA textureData;
	textureData.pSysMem = (void*)image.GetRawData();
	textureData.SysMemPitch = 4 * newTexture->m_dimensions.x;
	if (renderTargetTexture)
	{
		HRESULT hr = m_device->CreateTexture2D(&textureDesc, nullptr, &newTexture->m_texture);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE(Stringf("CreateTextureFromImage failed for image file \"%s\".",
				image.GetImageFilePath().c_str()));
		}
	}
	else
	{
		HRESULT hr = m_device->CreateTexture2D(&textureDesc, &textureData, &newTexture->m_texture);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE(Stringf("CreateTextureFromImage failed for image file \"%s\".",
				image.GetImageFilePath().c_str()));
		}
	}

	HRESULT hr = m_device->CreateShaderResourceView(newTexture->m_texture,
		NULL, &newTexture->m_shaderResourceView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateShaderResourceView failed for image file \"%s\".",
			image.GetImageFilePath().c_str()));
	}
	if (renderTargetTexture)
	{
		hr = m_device->CreateRenderTargetView(newTexture->m_texture,
			NULL, &newTexture->m_renderTargetView);

		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE(Stringf("CreateRenderTargetView failed for texture"));
		}
	}
	if (addToTextureList)
	{
		m_loadedTextures.push_back(newTexture);
	}
	if (recreateExistingTexture)
	{
		for (int i = 0; i < m_loadedTextures.size(); i++)
		{
			if (m_loadedTextures[i]->GetImageFilePath() == newTexture->GetImageFilePath())
			{
				delete m_loadedTextures[i];
				m_loadedTextures[i] = newTexture;
				break;
			}
		}
	}
	return newTexture;
}

void Renderer::CreateOITRenderTargets()
{
	Image accumImage(m_config.m_window->GetClientDimensions(), Rgba8(0, 0, 0, 0));
	accumImage.SetImageFilePath("AccumulationImage");
	m_OITAccumulationTexture = CreateTextureFromImage(accumImage, true, true, false, true);
	accumImage.SetImageFilePath("EmissiveAccumulationImage");
	m_OITEmissiveAccumulationTexture = CreateTextureFromImage(accumImage, true, true, false, true);

	m_OITRevealageTexture = new Texture();
	m_OITRevealageTexture->m_name = "Revealage"; // NOTE: m_name must be a std::string, otherwise it may point to temporary data!
	m_OITRevealageTexture->m_dimensions = m_config.m_window->GetClientDimensions();


	D3D11_TEXTURE2D_DESC textureDesc = { };
	textureDesc.Width = m_OITRevealageTexture->m_dimensions.x;
	textureDesc.Height = m_OITRevealageTexture->m_dimensions.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;

	textureDesc.Format = DXGI_FORMAT_R8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;

	HRESULT hr = m_device->CreateTexture2D(&textureDesc, nullptr, &m_OITRevealageTexture->m_texture);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateTexture2D failed for reveal texture"));
	}

	hr = m_device->CreateShaderResourceView(m_OITRevealageTexture->m_texture,
		NULL, &m_OITRevealageTexture->m_shaderResourceView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateShaderResourceView failed for reveal texture"));
	}

	hr = m_device->CreateRenderTargetView(m_OITRevealageTexture->m_texture,
		NULL, &m_OITRevealageTexture->m_renderTargetView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateRenderTargetView failed for texture"));
	}

	m_loadedTextures.push_back(m_OITRevealageTexture);
}

Texture* Renderer::CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData)
{
	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("CreateTextureFromData failed for \"%s\" - texelData was null!", name));
	GUARANTEE_OR_DIE(bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf("CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel));
	GUARANTEE_OR_DIE(dimensions.x > 0 && dimensions.y > 0, Stringf("CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y));

	Texture* newTexture = new Texture();
	newTexture->m_name = name; // NOTE: m_name must be a std::string, otherwise it may point to temporary data!
	newTexture->m_dimensions = dimensions;

	D3D11_TEXTURE2D_DESC textureDesc = { };
	textureDesc.Width = dimensions.x;
	textureDesc.Height = dimensions.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA textureData;
	textureData.pSysMem = (void*)texelData;
	textureData.SysMemPitch = 4 * dimensions.x;

	HRESULT hr = m_device->CreateTexture2D(&textureDesc, &textureData, &newTexture->m_texture);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateTextureFromImage failed for image file \"%s\".",
			name));
	}

	hr = m_device->CreateShaderResourceView(newTexture->m_texture, 
		NULL, &newTexture->m_shaderResourceView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateShaderResourceView failed for image file \"%s\".",
			name));
	}
	return newTexture;
}

void Renderer::BindTexture(Texture* texture)
{
	//do nothing if the texture we want is already bound
	if (m_currentTexture != nullptr)
	{
		if (m_currentTexture == texture)
		{
			return;
		}
	}
	else if (m_currentTexture == m_defaultTexture && texture == nullptr)
	{
		return;
	}
	
	if (texture == nullptr)
	{
		m_currentTexture = m_defaultTexture;
	}
	else
	{
		m_currentTexture = texture;
	}
	m_deviceContext->PSSetShaderResources(0, 1, &m_currentTexture->m_shaderResourceView);
}

void Renderer::BindSpecularGlossEmissiveMap(Texture* texture)
{
	m_currentSpecularGlossEmissiveMap = texture;
	if (m_currentSpecularGlossEmissiveMap != nullptr)
	{
		m_deviceContext->PSSetShaderResources(1, 1, &m_currentSpecularGlossEmissiveMap->m_shaderResourceView);
	}
	else
	{
		m_deviceContext->PSSetShaderResources(1, 1, nullptr);
	}
}

void Renderer::BindNormalMap(Texture* texture)
{
	m_currentNormalMap = texture;

	if (m_currentNormalMap != nullptr)
	{
		m_deviceContext->PSSetShaderResources(2, 1, &m_currentNormalMap->m_shaderResourceView);
	}
	else
	{
		m_deviceContext->PSSetShaderResources(2, 1, nullptr);
	}
}

Shader* Renderer::GetShader(std::string shaderName)
{
	for (size_t i = 0; i < m_loadedShaders.size(); i++)
	{
		if (m_loadedShaders[i]->GetName() == shaderName)
		{
			return m_loadedShaders[i];
		}
	}
	return nullptr;
}

ComputeShader* Renderer::GetComputeShader(std::string shaderName)
{
	for (size_t i = 0; i < m_loadedComputeShaders.size(); i++)
	{
		if (m_loadedComputeShaders[i]->GetName() == shaderName)
		{
			return m_loadedComputeShaders[i];
		}
	}
	return nullptr;
}

Shader* Renderer::CreateOrGetShaderFromFile(const char* shaderFilePath, VertexType vertexType)
{
	for (size_t i = 0; i < m_loadedShaders.size(); i++)
	{
		if (shaderFilePath == m_loadedShaders[i]->m_config.m_name)
		{
			return m_loadedShaders[i];
		}
	}
	return CreateShader(shaderFilePath, vertexType);
}

ComputeShader* Renderer::CreateOrGetComputeShaderFromFile(const char* shaderFilePath)
{
	for (size_t i = 0; i < m_loadedComputeShaders.size(); i++)
	{
		if (shaderFilePath == m_loadedComputeShaders[i]->m_config.m_name)
		{
			return m_loadedComputeShaders[i];
		}
	}

	size_t size = strlen(shaderFilePath) + 1;
	wchar_t* wc = new wchar_t[size];
	size_t outSize;
	mbstowcs_s(&outSize, wc, size, shaderFilePath, size - 1);

	return CreateComputeShader(shaderFilePath, "CSMain");
}

SpriteSheet* Renderer::CreateOrGetSpriteSheetFromFile(const char* spriteSheetFilePath, IntVec2 dimensions)
{
	// See if we already have this texture previously loaded
	SpriteSheet* existingSpriteSheet = GetSpriteSheetForFileName(spriteSheetFilePath);
	if (existingSpriteSheet)
	{
		return existingSpriteSheet;
	}

	// Never seen this spritesheet before!  Let's load it.
	SpriteSheet* newSpriteSheet = new SpriteSheet(CreateOrGetTextureFromFile(spriteSheetFilePath), dimensions);
	m_loadedSpriteSheets.push_back(newSpriteSheet);
	return newSpriteSheet;
}


Shader* Renderer::CreateShader(char const* shaderName, VertexType vertexType)
{
	std::string shaderPath = std::string(shaderName) + std::string(".hlsl");
	std::string shaderSource;
	FileReadToString(shaderSource, shaderPath);
	return CreateShader(shaderName, shaderSource.c_str(), vertexType);
}

Shader* Renderer::CreateShader(char const* shaderName, char const* shaderSource, VertexType vertexType)
{
	ShaderConfig config;
	config.m_name = std::string(shaderName);
	Shader* createdShader = new Shader(config);
	std::vector<unsigned char> vertexByteCode;
	CompileShaderToByteCode(vertexByteCode, shaderName, shaderSource, createdShader->m_config.m_vertexEntrypoint.c_str(), "vs_5_0");
	HRESULT hr = m_device->CreateVertexShader(vertexByteCode.data(), vertexByteCode.size(), NULL, &createdShader->m_vertexShader);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create the vertex shader."));
	}

	std::vector<unsigned char> pixelByteCode;
	CompileShaderToByteCode(pixelByteCode, shaderName, shaderSource, createdShader->m_config.m_pixelEntryPoint.c_str(), "ps_5_0");
	hr = m_device->CreatePixelShader(pixelByteCode.data(), pixelByteCode.size(), NULL, &createdShader->m_pixelShader);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create the pixel shader."));
	}

	//create input layout
	
	//create a local array of input element descriptions that defines the vertex layout
	if (vertexType == VertexType::VERTEX_TYPE_PCU)
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		//call Id3D11Device::CreateINputLayout
		unsigned int numElements = ARRAYSIZE(inputElementDesc);
		hr = m_device->CreateInputLayout(inputElementDesc, numElements, vertexByteCode.data(), vertexByteCode.size(), &createdShader->m_inputLayout);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE("Could not create vertex layout.");
		}
	}
	if (vertexType == VertexType::VERTEX_TYPE_PCUTBN)
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		//call Id3D11Device::CreateInputLayout
		unsigned int numElements = ARRAYSIZE(inputElementDesc);
		hr = m_device->CreateInputLayout(
			inputElementDesc, numElements,
			vertexByteCode.data(),
			vertexByteCode.size(),
			&createdShader->m_inputLayout
		);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE("Could not create vertex layout.");
		}
	}
	if (vertexType == VertexType::VERTEX_TYPE_PARTICLE)
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"QUAD_TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"EMISSIVE", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"ALPHA", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"PAN_TEXTURE_CONTRIBUTION", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"PARTICLE_LIFETIME", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

			//call Id3D11Device::CreateINputLayout
			unsigned int numElements = ARRAYSIZE(inputElementDesc);
			hr = m_device->CreateInputLayout(inputElementDesc, numElements, vertexByteCode.data(), vertexByteCode.size(), &createdShader->m_inputLayout);
			if (!SUCCEEDED(hr))
			{
				ERROR_AND_DIE("Could not create vertex layout.");
			}
	}
	if (vertexType == VertexType::VERTEX_TYPE_MESH_PARTICLE)
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"QUAD_TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"EMISSIVE", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"ALPHA", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"PAN_TEXTURE_CONTRIBUTION", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"PARTICLE_LIFETIME", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		//call Id3D11Device::CreateINputLayout
		unsigned int numElements = ARRAYSIZE(inputElementDesc);
		hr = m_device->CreateInputLayout(inputElementDesc, numElements, vertexByteCode.data(), vertexByteCode.size(), &createdShader->m_inputLayout);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE("Could not create vertex layout.");
		}
	}

	m_loadedShaders.push_back(createdShader);
	return createdShader;
}

bool Renderer::CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target)
{
	//Compile the vertex shader by calling D3DCompile
	DWORD shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;

#if defined(ENGINE_DEBUG_RENDER)
	shaderFlags = D3DCOMPILE_DEBUG;
	//shaderFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3DBlob* shaderBlob = NULL;
	ID3DBlob* errorBlob = NULL;
	
	HRESULT hr = D3DCompile(source, strlen(source), name, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, target, shaderFlags, 0, &shaderBlob, &errorBlob);

	if (SUCCEEDED(hr))
	{
		outByteCode.resize(shaderBlob->GetBufferSize());
		memcpy(
			outByteCode.data(),
			shaderBlob->GetBufferPointer(),
			shaderBlob->GetBufferSize()
		);
	}
	else
	{
		if (errorBlob != NULL)
		{
			DebuggerPrintf((char*)errorBlob->GetBufferPointer());
		}
		ERROR_AND_DIE(Stringf("Could not compile shader."));
	}

	DX_SAFE_RELEASE(shaderBlob);
	DX_SAFE_RELEASE(errorBlob);

	return true;
}

void Renderer::BindShader(Shader* shader)
{
	//do nothing if the shader we want is already bound
	if (m_currentShader != nullptr)
	{
		if (m_currentShader == shader || (m_currentShader == m_defaultShader && shader == nullptr))
		{
			return;
		}
	}

	if (shader == nullptr)
	{
		m_currentShader = m_defaultShader;
	}
	else
	{
		m_currentShader = shader;
	}

	m_deviceContext->VSSetShader(m_currentShader->m_vertexShader, nullptr, 0);
	m_deviceContext->PSSetShader(m_currentShader->m_pixelShader, nullptr, 0);
	m_deviceContext->IASetInputLayout(m_currentShader->m_inputLayout);

}

VertexBuffer* Renderer::CreateVertexBuffer(const size_t size)
{
	VertexBuffer* bufferToCreate = new VertexBuffer(size);
	//Create a local D3D11_BUFFER_DESC variable
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (unsigned int)size;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//Call ID3D11Device::CreateBuffer
	HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &bufferToCreate->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create vertex buffer.");
	}
	return bufferToCreate;
}

VertexBuffer* Renderer::CreateVertexBufferAsUAV(const size_t size)
{
	VertexBuffer* bufferToCreate = new VertexBuffer(size);
	//Create a local D3D11_BUFFER_DESC variable
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = (unsigned int)size;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;

	//Call ID3D11Device::CreateBuffer
	HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &bufferToCreate->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create vertex buffer.");
	}
	return bufferToCreate;
}

GPUBuffer* Renderer::CreateRawBuffer(const size_t size, void* pInitData)
{
	GPUBuffer* bufferToCreate = new GPUBuffer(size);
	//Create a local D3D11_BUFFER_DESC variable
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = pInitData;

	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = (unsigned int)size;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	HRESULT hr;
	if (pInitData)
	{
		InitData.pSysMem = pInitData;
		hr = m_device->CreateBuffer(&bufferDesc, &InitData, &bufferToCreate->m_buffer);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE("Failed to create structured buffer");
		}
		else
		{
			return bufferToCreate;
		}
	}

	//Call ID3D11Device::CreateBuffer
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, &bufferToCreate->m_buffer);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create raw buffer.");
	}
	return bufferToCreate;
}

IndirectArgsBuffer* Renderer::CreateIndirectArgsbuffer(InderectArgs const& args)
{
	IndirectArgsBuffer* bufferToCreate = new IndirectArgsBuffer(sizeof(InderectArgs));
	//Create a local D3D11_BUFFER_DESC variable
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &args;

	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = (unsigned int)sizeof(InderectArgs);
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS | D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	HRESULT hr;
	hr = m_device->CreateBuffer(&bufferDesc, &InitData, &bufferToCreate->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Failed to create structured buffer");
	}
	else
	{
		return bufferToCreate;
	}
}

IndexBuffer* Renderer::CreateIndexBuffer(const size_t size)
{
	IndexBuffer* bufferToCreate = new IndexBuffer(size);
	//Create a local D3D11_BUFFER_DESC variable
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)size;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//Call ID3D11Device::CreateBuffer
	HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &bufferToCreate->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create vertex buffer.");
	}
	return bufferToCreate;
}

void Renderer::CopyCPUToGPU(void* data, size_t size, GPUBuffer*& gpuBuffer)
{
	if (gpuBuffer->m_size < size)
	{
		delete gpuBuffer;
		gpuBuffer = CreateRawBuffer(size, data);
	}
	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map(gpuBuffer->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, data, size);
	m_deviceContext->Unmap(gpuBuffer->m_buffer, 0);
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, VertexBuffer*& vbo)
{
	//Copy the vertex buffer data from the CPU to the GPU

	//check if the existing immediate vertex buffer is large enough for the data being passed in
	if (vbo->m_size < size)
	{
		//if it is not recreate the vertex buffer so it is sufficiently large
		delete vbo;
		vbo = CreateVertexBuffer(size);
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map(vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, data, size);
	m_deviceContext->Unmap(vbo->m_buffer, 0);
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, IndexBuffer*& ibo)
{
	//Copy the vertex buffer data from the CPU to the GPU

	//check if the existing immediate vertex buffer is large enough for the data being passed in
	if (ibo->m_size < size)
	{
		//if it is not recreate the vertex buffer so it is sufficiently large
		delete ibo;
		ibo = CreateIndexBuffer(size);
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map(ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, data, size);
	m_deviceContext->Unmap(ibo->m_buffer, 0);
}

void Renderer::BindVertexBuffer(VertexBuffer* vbo, VertexType type)
{
	
	UINT stride = 0;
	if (type == VertexType::VERTEX_TYPE_PCUTBN)
	{
		stride = sizeof(Vertex_PCUTBN);
	}
	else if (type == VertexType::VERTEX_TYPE_PCU)
	{
		stride = sizeof(Vertex_PCU);
	}
	else if (type == VertexType::VERTEX_TYPE_PARTICLE)
	{
		stride = sizeof(Vertex_Particle);
	}
	else if (type == VertexType::VERTEX_TYPE_MESH_PARTICLE)
	{
		stride = sizeof(Vertex_MeshParticle);
	}
	UINT startOffset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &stride, &startOffset);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::BindIndexBuffer(IndexBuffer* ibo)
{
	m_deviceContext->IASetIndexBuffer(ibo->m_buffer, DXGI_FORMAT_R32_UINT, 0);
}

void Renderer::DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset, VertexType vertType)
{
	BindVertexBuffer(vbo, vertType);
	SetStatesIfChanged();
	//Draw
	m_deviceContext->Draw(vertexCount, vertexOffset);
}

void Renderer::DrawVertexBufferIndexed(VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, VertexType type)
{
	BindVertexBuffer(vbo, type);
	BindIndexBuffer(ibo);
	SetStatesIfChanged();
	//Draw Indexed
	m_deviceContext->DrawIndexed(indexCount, 0, 0);
}

ConstantBuffer* Renderer::CreateConstantBuffer(const size_t size)
{
	ConstantBuffer* bufferToCreate = new ConstantBuffer(size);

	D3D11_BUFFER_DESC bufferDesc = { };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)size;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//bufferDesc.StructureByteStride

	//Call ID3D11Device::CreateBuffer
	HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &bufferToCreate->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create constant buffer.");
	}
	return bufferToCreate;
}



void Renderer::CopyCPUToGPU(const void* data, ConstantBuffer* cbo)
{
	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map(cbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, data, cbo->m_size);
	m_deviceContext->Unmap(cbo->m_buffer, 0);
}

void Renderer::BindConstantBuffer(int slot, ConstantBuffer* cbo)
{
	m_deviceContext->VSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	m_deviceContext->CSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	m_deviceContext->PSSetConstantBuffers(slot, 1, &cbo->m_buffer);
}

void Renderer::SetStatesIfChanged()
{
	if (m_blendStates[(int)m_desiredBlendMode] != m_blendState)
	{
		m_blendState = m_blendStates[(int)m_desiredBlendMode];
		float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		UINT sampleMask = 0xffffffff;
		m_deviceContext->OMSetBlendState(m_blendState, blendFactor, sampleMask);
	}

	if (m_samplerStates[(int)m_desiredSamplermode] != m_samplerState)
	{
		m_samplerState = m_samplerStates[(int)m_desiredSamplermode];
		m_deviceContext->PSSetSamplers(0, 1, &m_samplerState);
	}

	if (m_rasterizeStates[(int)m_desiredRasterizerMode] != m_rasterizeState)
	{
		m_rasterizeState = m_rasterizeStates[(int)m_desiredRasterizerMode];
		m_deviceContext->RSSetState(m_rasterizeState);
	}

	if (m_depthModes[(int)m_desiredDepthMode] != m_depthMode)
	{
		m_depthMode = m_depthModes[(int)m_desiredDepthMode];
		m_deviceContext->OMSetDepthStencilState(m_depthMode, 0);
	}
	if (m_primaryRenderTargetType != m_desiredRenderTargetType)
	{
		m_primaryRenderTargetType = m_desiredRenderTargetType;
		if (m_primaryRenderTargetType == PrimaryRenderTargetType::HDR && m_config.m_hdrEnabled)
		{
			ID3D11RenderTargetView* RTVs[] =
			{
				GetPrimaryRenderTargetView(), //GetPrimaryRenderTargetView()
				m_emissiveRenderTexture->m_renderTargetView,
			};
			m_deviceContext->OMSetRenderTargets(2, RTVs, m_depthStencilView);
		}
		else
		{
			m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
		}
	}
}

void Renderer::SetModelConstants(const Mat44& modelMatrix, const Rgba8& modelColor)
{
	ModelConstants modelConstants;
	modelConstants.modelMatrix = modelMatrix;

	float colorAsFloats[4];
	modelColor.GetAsFloats(colorAsFloats);
	modelConstants.modelColor.x = colorAsFloats[0];
	modelConstants.modelColor.y = colorAsFloats[1];
	modelConstants.modelColor.z = colorAsFloats[2];
	modelConstants.modelColor.w = colorAsFloats[3];

	CopyCPUToGPU((void*)&modelConstants, m_modelCBO);
	BindConstantBuffer(k_modelConstantsSlot, m_modelCBO);
}

void Renderer::SetLightingConstants(Vec3 const& sunDirection, float sunIntensity, float ambientIntensity)
{
	LightConstants lightConstants;
	lightConstants.SunDirection = sunDirection;
	lightConstants.SunIntensity = sunIntensity;
	lightConstants.AmbientIntensity = ambientIntensity;
	CopyCPUToGPU((void*)&lightConstants, m_lightingCBO);
	BindConstantBuffer(k_lightConstantsSlot, m_lightingCBO);
}

void Renderer::SetLightingConstants(LightConstants const& lightConstants)
{
	CopyCPUToGPU((void*)&lightConstants, m_lightingCBO);
	BindConstantBuffer(k_lightConstantsSlot, m_lightingCBO);
}

void Renderer::SetPointLights(int currNumPointLights, PointLight updatedPointLights[MAX_NUM_POINT_LIGHTS])
{
	PointLightConstants pointLightConstants;
	pointLightConstants.numPointLights = currNumPointLights;
	for (int i = 0; i < currNumPointLights; i++)
	{
		pointLightConstants.pointLights[i] = updatedPointLights[i];
	}
	CopyCPUToGPU((void*)&pointLightConstants, m_pointLightsCBO);
	BindConstantBuffer(k_pointLightConstantsSlot, m_pointLightsCBO);
}

void Renderer::SetRasterizeMode(RasterizeMode desiredMode)
{
	m_desiredRasterizerMode = desiredMode;
}

void Renderer::SetDepthMode(DepthMode desiredDepthMode)
{
	m_desiredDepthMode = desiredDepthMode;
}

ComputeShader* Renderer::CreateComputeShader(const char* shaderName, const char* pFunctionName, bool writeCompiledShaderToFile, std::string compiledPath, bool readCompiledShader)
{
	std::vector<uint8_t> compiledShaderBuff;
	const void* compiledShaderPtr = nullptr;
	SIZE_T compiledShaderSize = 0;
	if (readCompiledShader)
	{
		FileReadToBuffer(compiledShaderBuff, compiledPath);
		compiledShaderPtr = (void*)compiledShaderBuff.data();
		compiledShaderSize = (SIZE_T)compiledShaderBuff.size();
	}
	else
	{
		//"CSMain"
		size_t size = strlen(shaderName) + 1;
		wchar_t* pSrcFile = new wchar_t[size];
		size_t outSize;
		mbstowcs_s(&outSize, pSrcFile, size, shaderName, size - 1);

		if (!m_device)
		{
			return nullptr;
		}

		DWORD shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined(ENGINE_DEBUG_RENDER)
		shaderFlags = D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
		shaderFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

		const D3D_SHADER_MACRO defines[] =
		{
			"USE_STRUCTURED_BUFFERS", "1",
			"TEST_DOUBLE", "1",
			nullptr, nullptr
		};

		// We generally prefer to use the higher CS shader profile when possible as CS 5.0 is better performance on 11-class hardware
		LPCSTR pProfile = (m_device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";

		ID3DBlob* pErrorBlob = nullptr;
		ID3DBlob* pBlob = nullptr;
		HRESULT hr = D3DCompileFromFile(pSrcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, pFunctionName, pProfile,
			shaderFlags, 0, &pBlob, &pErrorBlob);
		if (SUCCEEDED(hr))
		{
			if (writeCompiledShaderToFile)
			{
				std::vector<unsigned char> shaderByteCode;
				shaderByteCode.resize(pBlob->GetBufferSize());
				memcpy(
					shaderByteCode.data(),
					pBlob->GetBufferPointer(),
					pBlob->GetBufferSize()
				);
				WriteBufferToFile(shaderByteCode, compiledPath);
			}
		}
		if (FAILED(hr))
		{
			if (pErrorBlob)
			{
				ERROR_AND_DIE(Stringf("Could not Compile Shader %s", (char*)pErrorBlob->GetBufferPointer()));
			}
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

			DX_SAFE_RELEASE(pErrorBlob);
			DX_SAFE_RELEASE(pBlob);

			return nullptr;
		}
		compiledShaderPtr = pBlob->GetBufferPointer();
		compiledShaderSize = pBlob->GetBufferSize();
	}

	ComputeShaderConfig config;
	config.m_computeEntryPoint = pFunctionName;
	config.m_name = (const char*)shaderName;
	ComputeShader* computeShader = new ComputeShader(config);

	HRESULT hr;
	hr = m_device->CreateComputeShader(compiledShaderPtr, compiledShaderSize, nullptr, &computeShader->m_computeShader);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Failed to Create compute shader from compiled byte code!");
	}
#if defined(ENGINE_DEBUG_RENDER)
	if (SUCCEEDED(hr))
	{
		(computeShader->m_computeShader)->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(pFunctionName), pFunctionName);
	}
#endif

	m_loadedComputeShaders.push_back(computeShader);
	return computeShader;
	
}

StructuredBuffer* Renderer::CreateStructuredBuffer(unsigned int uElementSize, unsigned int uCount, void const* pInitData, bool dynamic, bool canReadFrom)
{
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (dynamic && canReadFrom)
	{
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	}
	if (!dynamic)
	{
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	}
	desc.ByteWidth = uElementSize * uCount;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = uElementSize;
	if (dynamic)
	{
		desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
	}
	if (canReadFrom)
	{
		desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
	}
	StructuredBuffer* structuredBuffer = new StructuredBuffer(uElementSize * uCount);
	HRESULT hr;
	if (pInitData)
	{
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = pInitData;
		hr = m_device->CreateBuffer(&desc, &InitData, &structuredBuffer->m_buffer);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE("Failed to create structured buffer");
		}
		else
		{
			return structuredBuffer;
		}
	}
	hr = m_device->CreateBuffer(&desc, nullptr, &structuredBuffer->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Failed to create structured buffer");
	}
	else
	{
		return structuredBuffer;
	}
}


UAV* Renderer::CreateUAV(GPUBuffer* gpuBuffer)
{
	D3D11_BUFFER_DESC descBuf = {};
	gpuBuffer->m_buffer->GetDesc(&descBuf);

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;

	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// This is a Raw Buffer

		desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
		desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		desc.Buffer.NumElements = descBuf.ByteWidth / 4;
	}
	else
		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{
			// This is a Structured Buffer

			desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
			desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
		}
		else
		{
			ERROR_AND_DIE("Failed to create UAV buffer is not raw or structured");
		}
	UAV* uav = new UAV(gpuBuffer);
	m_device->CreateUnorderedAccessView(uav->m_gpuBuffer->m_buffer, &desc, &uav->m_uav);

	return uav;
}

SRV* Renderer::CreateSRV(GPUBuffer* gpuBuffer)
{
	D3D11_BUFFER_DESC descBuf = {};
	gpuBuffer->m_buffer->GetDesc(&descBuf);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;

	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// This is a Raw Buffer

		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
	}
	else
		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{
			// This is a Structured Buffer

			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
		}
		else
		{
			ERROR_AND_DIE("Failed to create SRV buffer is not raw or structured");
		}
	SRV* srv = new SRV(gpuBuffer);
	m_device->CreateShaderResourceView(srv->m_gpuBuffer->m_buffer, &desc, &srv->m_srv);
	return srv;
}

void Renderer::MapBufferForWrite(GPUBuffer* buffer, D3D11_MAPPED_SUBRESOURCE* mappedResource)
{
	HRESULT hr = m_deviceContext->Map(buffer->m_buffer, 0, D3D11_MAP_WRITE, 0, mappedResource);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Failed to map buffer for write");
	}
}

void Renderer::UnmapBuffer(GPUBuffer* buffer)
{
	m_deviceContext->Unmap(buffer->m_buffer, 0);
}

void* Renderer::MapBufferForReadWrite(GPUBuffer* buffer)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(buffer->m_buffer, 0, D3D11_MAP_READ_WRITE, 0, &mappedResource);
	return mappedResource.pData;
}

void* Renderer::ReadFromBuffer(GPUBuffer* buffer)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = m_deviceContext->Map(buffer->m_buffer, 0, D3D11_MAP_READ, 0, &mappedResource);
	if (!SUCCEEDED(hr))
	{
		hr = m_device->GetDeviceRemovedReason();
		ERROR_AND_DIE("Failed to map buffer for write");
	}
	m_deviceContext->Unmap(buffer->m_buffer, 0);
	return mappedResource.pData;
}

void Renderer::UnmapBuffer(ID3D11Buffer* buffer)
{
	m_deviceContext->Unmap(buffer, 0);
}

void Renderer::SetCameraConstants(Camera const& camera)
{
	CameraConstants cameraConstants;
	cameraConstants.ProjectionMatrix = camera.GetProjectionMatrix();
	cameraConstants.ViewMatrix = camera.GetViewMatrix();
	CopyCPUToGPU((void*)&cameraConstants, m_cameraCBO);
	BindConstantBuffer(k_cameraConstantsSlot, m_cameraCBO);
}

void Renderer::BindPrevFrameDepthTextureToTRegister(unsigned int tReg)
{
	/*
	ID3D11RenderTargetView* primaryRTV = GetPrimaryRenderTargetView();
	m_deviceContext->OMSetRenderTargets(1, &primaryRTV, nullptr);
	*/
	m_deviceContext->CSSetShaderResources(tReg, 1, &m_prevFrameDepthStencilSRV);
	m_deviceContext->PSSetShaderResources(tReg, 1, &m_prevFrameDepthStencilSRV);
}

void Renderer::RunComputeShader(ComputeShader* pComputeShader, 
	std::vector<SRV*> const& srvs, 
	std::vector<UAV*> const& uavs, 
	unsigned int X, unsigned int Y, unsigned int Z)
{
	m_deviceContext->CSSetShader(pComputeShader->m_computeShader, nullptr, 0);
	std::vector<ID3D11UnorderedAccessView*> rawUAVs;
	std::vector<ID3D11ShaderResourceView*> rawSRVs;
	for (int i = 0; i < (int)srvs.size(); i++)
	{
		rawSRVs.push_back(srvs[i]->m_srv);
	}
	for (int i = 0; i < (int)uavs.size(); i++)
	{
		rawUAVs.push_back(uavs[i]->m_uav);
	}
	m_deviceContext->CSSetShaderResources(0, (UINT)srvs.size(), rawSRVs.data());
	m_deviceContext->CSSetUnorderedAccessViews(0, (UINT)uavs.size(), rawUAVs.data(), nullptr);
	m_deviceContext->Dispatch(X, Y, Z);

	m_deviceContext->CSSetShader(nullptr, nullptr, 0);

	if (uavs.size() > 0)
	{
		ID3D11UnorderedAccessView** ppUAVnullptr = new ID3D11UnorderedAccessView * [uavs.size()];
		for (int i = 0; i < (int)uavs.size(); i++)
		{
			ppUAVnullptr[i] = nullptr;
		}
		m_deviceContext->CSSetUnorderedAccessViews(0, (UINT)uavs.size(), ppUAVnullptr, nullptr);
	}

	if (srvs.size() > 0)
	{
		ID3D11ShaderResourceView** ppSRVnullptr = new ID3D11ShaderResourceView * [srvs.size()];
		for (int i = 0; i < (int)srvs.size(); i++)
		{
			ppSRVnullptr[i] = nullptr;
		}
		m_deviceContext->CSSetShaderResources(0, (UINT)srvs.size(), ppSRVnullptr);
	}


	ID3D11Buffer* ppCBnullptr[1] = { nullptr };
	m_deviceContext->CSSetConstantBuffers(0, 1, ppCBnullptr);
}

void Renderer::SetComputeShaderSRVsAndUAVs(std::vector<SRV*> const& srvs, std::vector<UAV*> const& uavs)
{
	std::vector<ID3D11UnorderedAccessView*> rawUAVs;
	std::vector<ID3D11ShaderResourceView*> rawSRVs;
	for (int i = 0; i < (int)srvs.size(); i++)
	{
		rawSRVs.push_back(srvs[i]->m_srv);
	}
	for (int i = 0; i < (int)uavs.size(); i++)
	{
		rawUAVs.push_back(uavs[i]->m_uav);
	}
	m_deviceContext->CSSetShaderResources(0, (UINT)srvs.size(), rawSRVs.data());
	m_deviceContext->CSSetUnorderedAccessViews(0, (UINT)uavs.size(), rawUAVs.data(), nullptr);
}

void Renderer::RunComputeShader(ComputeShader* computeShader, unsigned int X, unsigned int Y, unsigned int Z)
{
	m_deviceContext->CSSetShader(computeShader->m_computeShader, nullptr, 0);
	m_deviceContext->Dispatch(X, Y, Z);
	m_deviceContext->CSSetShader(nullptr, nullptr, 0);

	ID3D11Buffer* ppCBnullptr[1] = { nullptr };
	m_deviceContext->CSSetConstantBuffers(0, 1, ppCBnullptr);
}

void Renderer::RunComputeShader(ComputeShader* pComputeShader, std::vector<SRV*> const& srvs, std::vector<UAV*> const& uavs, IndirectArgsBuffer* indirectArgs)
{
	m_deviceContext->CSSetShader(pComputeShader->m_computeShader, nullptr, 0);
	std::vector<ID3D11UnorderedAccessView*> rawUAVs;
	std::vector<ID3D11ShaderResourceView*> rawSRVs;
	for (int i = 0; i < (int)srvs.size(); i++)
	{
		rawSRVs.push_back(srvs[i]->m_srv);
	}
	for (int i = 0; i < (int)uavs.size(); i++)
	{
		rawUAVs.push_back(uavs[i]->m_uav);
	}
	m_deviceContext->CSSetShaderResources(0, (UINT)srvs.size(), rawSRVs.data());
	m_deviceContext->CSSetUnorderedAccessViews(0, (UINT)uavs.size(), rawUAVs.data(), nullptr);
	m_deviceContext->DispatchIndirect(indirectArgs->m_buffer, 0);

	m_deviceContext->CSSetShader(nullptr, nullptr, 0);

	if (uavs.size() > 0)
	{
		ID3D11UnorderedAccessView** ppUAVnullptr = new ID3D11UnorderedAccessView * [uavs.size()];
		for (int i = 0; i < (int)uavs.size(); i++)
		{
			ppUAVnullptr[i] = nullptr;
		}
		m_deviceContext->CSSetUnorderedAccessViews(0, (UINT)uavs.size(), ppUAVnullptr, nullptr);
	}

	if (srvs.size() > 0)
	{
		ID3D11ShaderResourceView** ppSRVnullptr = new ID3D11ShaderResourceView * [srvs.size()];
		for (int i = 0; i < (int)srvs.size(); i++)
		{
			ppSRVnullptr[i] = nullptr;
		}
		m_deviceContext->CSSetShaderResources(0, (UINT)srvs.size(), ppSRVnullptr);
	}


	ID3D11Buffer* ppCBnullptr[1] = { nullptr };
	m_deviceContext->CSSetConstantBuffers(0, 1, ppCBnullptr);
}

void Renderer::ClearSRVSAndUAVS(unsigned int numSRVS, unsigned int numUAVS)
{

	if (numUAVS > 0)
	{
		ID3D11UnorderedAccessView** ppUAVnullptr = new ID3D11UnorderedAccessView * [numUAVS];
		for (int i = 0; i < (int)numUAVS; i++)
		{
			ppUAVnullptr[i] = nullptr;
		}
		m_deviceContext->CSSetUnorderedAccessViews(0, (UINT)numUAVS, ppUAVnullptr, nullptr);

	}

	if (numSRVS > 0)
	{
		ID3D11ShaderResourceView** ppSRVnullptr = new ID3D11ShaderResourceView * [numSRVS];
		for (int i = 0; i < (int)numSRVS; i++)
		{
			ppSRVnullptr[i] = nullptr;
		}
		m_deviceContext->CSSetShaderResources(0, (UINT)numSRVS, ppSRVnullptr);
		m_deviceContext->PSSetShaderResources(0, (UINT)numSRVS, ppSRVnullptr);
	}
}

void Renderer::CreateEmissiveTextures()
{
	bool use16f = m_config.m_hdrEnabled;
	m_emissiveRenderTexture = CreateTextureFromImage(Image(m_config.m_window->GetClientDimensions(), Rgba8::WHITE), true, false, false, use16f);
	m_blurredEmissiveRenderTexture = CreateTextureFromImage(Image(m_config.m_window->GetClientDimensions(), Rgba8::WHITE), true, false, false, use16f);

	int currentHeight = m_config.m_window->GetClientDimensions().y / 2;
	while (currentHeight >= 32)
	{
		IntVec2 shrunkDimensions = IntVec2((int)(currentHeight * m_config.m_window->GetAspectRatio()), currentHeight);
		Texture* blurDownTexture = CreateTextureFromImage(Image(shrunkDimensions, Rgba8::WHITE), true, false, false, use16f);
		m_blurDownTextures.push_back(blurDownTexture);
		if (currentHeight >= 64)
		{
			Texture* blurUpTexture = CreateTextureFromImage(Image(shrunkDimensions, Rgba8::WHITE), true, false, false, use16f);
			m_blurUpTextures.push_back(blurUpTexture);
		}
		currentHeight /= 2;
	}
	std::vector<Vertex_PCU> fullScreenQuadVerts;
	AddVertsForQuad3D(fullScreenQuadVerts, Vec3(-1.f, -1.f, 0.f), Vec3(1.f, -1.f, 0.f), Vec3(1.f, 1.f, 0.f), Vec3(-1.f, 1.f, 0.f), Rgba8::WHITE, AABB2(Vec2(0.f, 1.f), Vec2(1.f, 0.f)));
	m_fullScreenQuadNDCSpace = CreateVertexBuffer(sizeof(Vertex_PCU) * 6);
	CopyCPUToGPU(fullScreenQuadVerts.data(), sizeof(Vertex_PCU) * 6, m_fullScreenQuadNDCSpace);
}

void Renderer::RenderEmissive()
{
	SetDepthMode(DepthMode::DISABLED);
	SetBlendMode(BlendMode::OPAQUE);
	SetSamplerMode(SamplerMode::BILINEAR_CLAMP);
	SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);

	BlurConstants blurDownConstants;
	blurDownConstants.LerpT = 1.f;
	blurDownConstants.NumSamples = 13;
	blurDownConstants.Samples[0] = { Vec2(-2.f, 2.f),	0.0323f };
	blurDownConstants.Samples[1] = { Vec2(0.f, 2.f),	0.0645f };
	blurDownConstants.Samples[2] = { Vec2(2.f, 2.f),	0.0323f };
	blurDownConstants.Samples[3] = { Vec2(-1.f, 1.f),	0.1290f };
	blurDownConstants.Samples[4] = { Vec2( 1.f, 1.f),	0.1290f };
	blurDownConstants.Samples[5] = { Vec2(-2.f, 0.f),	0.0645f };
	blurDownConstants.Samples[6] = { Vec2( 0.f, 0.f),	0.0968f };
	blurDownConstants.Samples[7] = { Vec2( 2.f, 0.f),	0.0645f };
	blurDownConstants.Samples[8] = { Vec2(-1.f, -1.f),	0.1290f };
	blurDownConstants.Samples[9] = { Vec2( 1.f, -1.f),	0.1290f };
	blurDownConstants.Samples[10] ={ Vec2(-2.f, -2.f),	0.0323f };
	blurDownConstants.Samples[11] ={ Vec2( 0.f, -2.f),	0.0645f };
	blurDownConstants.Samples[12] ={ Vec2(2.f, -2.f),	0.0323f };

	for (int i = 0; i < m_blurDownTextures.size(); i++)
	{
		ID3D11RenderTargetView* rtvs[] = { m_blurDownTextures[i]->m_renderTargetView, nullptr};
		m_deviceContext->OMSetRenderTargets(2, rtvs, nullptr);
		if (i != 0)
		{
			m_deviceContext->PSSetShaderResources(0, 1, &m_blurDownTextures[i - 1]->m_shaderResourceView);
		}
		else
		{
			m_deviceContext->PSSetShaderResources(0, 1, &m_emissiveRenderTexture->m_shaderResourceView);
		}
		blurDownConstants.TexelSize = Vec2(1.f / (float)m_blurDownTextures[i]->GetDimensions().x, 1.f / (float)m_blurDownTextures[i]->GetDimensions().y);
		if (m_blurCBO == nullptr)
		{
			m_blurCBO = CreateConstantBuffer(sizeof(BlurConstants));
		}
		CopyCPUToGPU((void*)&blurDownConstants, m_blurCBO);
		BindConstantBuffer(k_blurConstantsSlot, m_blurCBO);

		//set viewport to the size of the current blur down texture
		//Create a D3D11_VIEWPORT variable
		D3D11_VIEWPORT viewport = { 0 };
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)m_blurDownTextures[i]->GetDimensions().x;
		viewport.Height = (float)m_blurDownTextures[i]->GetDimensions().y;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		//Call ID3D11DeviceContext::RSSetViewports
		m_deviceContext->RSSetViewports(1, &viewport);
		BindShader(CreateOrGetShaderFromFile(GetShaderFilePath("BlurDown").c_str(), VertexType::VERTEX_TYPE_PCU));
		DrawVertexBuffer(m_fullScreenQuadNDCSpace, 6, 0, VertexType::VERTEX_TYPE_PCU);
	}

	//set the blur constants for blurring up
	blurDownConstants.NumSamples = 9;
	blurDownConstants.LerpT = .85f;
	blurDownConstants.Samples[0] = { Vec2(-1.f, 1.f),	0.0625f };
	blurDownConstants.Samples[1] = { Vec2( 0.f, 1.f),	0.1250f };
	blurDownConstants.Samples[2] = { Vec2( 1.f, 1.f),	0.0625f };
	blurDownConstants.Samples[3] = { Vec2(-1.f, 0.f),	0.1250f };
	blurDownConstants.Samples[4] = { Vec2( 0.f, 0.f),	0.2500f };
	blurDownConstants.Samples[5] = { Vec2( 1.f, 0.f),	0.1250f };
	blurDownConstants.Samples[6] = { Vec2(-1.f,-1.f),	0.0625f };
	blurDownConstants.Samples[7] = { Vec2( 0.f,-1.f),	0.1250f };
	blurDownConstants.Samples[8] = { Vec2( 1.f,-1.f),	0.0625f };

	//for each blur up texture in order from smallest to largest
	for (int i = (int)m_blurUpTextures.size() - 1; i >= 0; i--)
	{
		m_deviceContext->OMSetRenderTargets(1, &m_blurUpTextures[i]->m_renderTargetView, nullptr);

		if (i != (int)m_blurUpTextures.size() - 1)
		{
			ID3D11ShaderResourceView* srvs[] = { m_blurDownTextures[i]->m_shaderResourceView, m_blurUpTextures[i + 1]->m_shaderResourceView };
			blurDownConstants.TexelSize = Vec2(1.f / (float)m_blurUpTextures[i + 1]->GetDimensions().x, 1.f / (float)m_blurUpTextures[i + 1]->GetDimensions().y);
			m_deviceContext->PSSetShaderResources(0, 2, srvs);

		}
		else
		{
			Texture* smallestBlurDown = m_blurDownTextures[(int)m_blurDownTextures.size() - 1];
			ID3D11ShaderResourceView* srvs[] = { m_blurDownTextures[i]->m_shaderResourceView, smallestBlurDown->m_shaderResourceView };
			blurDownConstants.TexelSize = Vec2(1.f / (float)smallestBlurDown->GetDimensions().x, 1.f / (float)smallestBlurDown->GetDimensions().y);
			m_deviceContext->PSSetShaderResources(0, 2, srvs);
		}
		CopyCPUToGPU((void*)&blurDownConstants, m_blurCBO);
		BindConstantBuffer(k_blurConstantsSlot, m_blurCBO);

		//set viewport to the size of the current blur up texture
		//Create a D3D11_VIEWPORT variable
		D3D11_VIEWPORT viewport = { 0 };
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)m_blurUpTextures[i]->GetDimensions().x;
		viewport.Height = (float)m_blurUpTextures[i]->GetDimensions().y;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		
		//Call ID3D11DeviceContext::RSSetViewports
		m_deviceContext->RSSetViewports(1, &viewport);
		BindShader(CreateOrGetShaderFromFile(GetShaderFilePath("BlurUp").c_str(), VertexType::VERTEX_TYPE_PCU));
		DrawVertexBuffer(m_fullScreenQuadNDCSpace, 6, 0, VertexType::VERTEX_TYPE_PCU);
	}
	m_deviceContext->OMSetRenderTargets(1, &m_blurredEmissiveRenderTexture->m_renderTargetView, nullptr);
	ID3D11ShaderResourceView* srvs[] = { m_emissiveRenderTexture->m_shaderResourceView, m_blurUpTextures[0]->m_shaderResourceView };
	m_deviceContext->PSSetShaderResources(0, 2, srvs);
	blurDownConstants.TexelSize = Vec2(1.f / (float)m_blurUpTextures[0]->GetDimensions().x, 1.f / (float)m_blurUpTextures[0]->GetDimensions().y);

	CopyCPUToGPU((void*)&blurDownConstants, m_blurCBO);
	BindConstantBuffer(k_blurConstantsSlot, m_blurCBO);

	D3D11_VIEWPORT viewport = { 0 };
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)m_blurredEmissiveRenderTexture->GetDimensions().x;
	viewport.Height = (float)m_blurredEmissiveRenderTexture->GetDimensions().y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//Call ID3D11DeviceContext::RSSetViewports
	m_deviceContext->RSSetViewports(1, &viewport);
	BindShader(CreateOrGetShaderFromFile(GetShaderFilePath("BlurUp").c_str(), VertexType::VERTEX_TYPE_PCU));
	DrawVertexBuffer(m_fullScreenQuadNDCSpace, 6, 0, VertexType::VERTEX_TYPE_PCU);


	//bind the back buffer as the render target
	ID3D11RenderTargetView* rtvs[] = { GetPrimaryRenderTargetView(), nullptr };
	m_deviceContext->OMSetRenderTargets(2, rtvs, nullptr);

	//bind the blurred emissive texture as a shader resoure
	m_deviceContext->PSSetShaderResources(0, 1, &m_blurredEmissiveRenderTexture->m_shaderResourceView);

	//set the blend mode to additive
	SetBlendMode(BlendMode::ADDITIVE_EMISSIVE);

	//draw a full screen quad
	BindShader(CreateOrGetShaderFromFile(GetShaderFilePath("Composite").c_str(), VertexType::VERTEX_TYPE_PCU));
	DrawVertexBuffer(m_fullScreenQuadNDCSpace, 6, 0, VertexType::VERTEX_TYPE_PCU);

	SetBlendMode(BlendMode::OPAQUE);
}

void Renderer::CompositeOIT()
{	
	//bind the back buffer as the render target
	ID3D11RenderTargetView* rtvs[] = { GetPrimaryRenderTargetView(), m_emissiveRenderTexture->m_renderTargetView };
	m_deviceContext->OMSetRenderTargets(2, rtvs, nullptr);

	ID3D11ShaderResourceView* srvs[] = { m_OITAccumulationTexture->m_shaderResourceView, m_OITRevealageTexture->m_shaderResourceView, m_OITEmissiveAccumulationTexture->m_shaderResourceView };
	m_deviceContext->PSSetShaderResources(0, 3, srvs);

	//src blend SRC_ALPHA dst belnd ONE_MINUS_SRC_ALPHA
	SetBlendMode(BlendMode::ALPHA);
	//draw a full screen quad
	BindShader(CreateOrGetShaderFromFile(GetShaderFilePath("CompositePass").c_str(), VertexType::VERTEX_TYPE_PCU));
	DrawVertexBuffer(m_fullScreenQuadNDCSpace, 6, 0, VertexType::VERTEX_TYPE_PCU);

	ID3D11ShaderResourceView* nullSrvs[] = { nullptr, nullptr, nullptr };
	m_deviceContext->PSSetShaderResources(0, 3, nullSrvs);
}

void Renderer::SetPSSRVs(std::vector<SRV*> srvsToSet, int startSlot)
{
	std::vector<ID3D11ShaderResourceView*> rawSRVs;
	for (int i = 0; i < (int)srvsToSet.size(); i++)
	{
		rawSRVs.push_back(srvsToSet[i]->m_srv);
	}
	m_deviceContext->PSSetShaderResources(startSlot, (UINT)rawSRVs.size(), rawSRVs.data());
}

void Renderer::BindSRVsForTextureArray(std::vector<Texture*> textures, int startSlot)
{
	std::vector<ID3D11ShaderResourceView*> rawSRVs;
	for (int i = 0; i < (int)textures.size(); i++)
	{
		rawSRVs.push_back(textures[i]->m_shaderResourceView);
	}
	m_deviceContext->PSSetShaderResources((UINT)startSlot, (UINT)rawSRVs.size(), rawSRVs.data());
}

void Renderer::BindTextureToSlot(Texture* texture, int startSlot)
{
	m_deviceContext->PSSetShaderResources((UINT)startSlot, 1, &texture->m_shaderResourceView);
	m_deviceContext->CSSetShaderResources((UINT)startSlot, 1, &texture->m_shaderResourceView);
}

void Renderer::BindSRVToSlot(SRV* srv, int startSlot)
{
	m_deviceContext->PSSetShaderResources((UINT)startSlot, 1, &srv->m_srv);
	m_deviceContext->CSSetShaderResources((UINT)startSlot, 1, &srv->m_srv);
}

void Renderer::InitializeHDR()
{
	Image hdrImage(m_config.m_window->GetClientDimensions(), Rgba8(0, 0, 0, 0));
	hdrImage.SetImageFilePath("HDRImage");
	m_HDRTexture = CreateTextureFromImage(hdrImage, true, true, false, true);
}

void Renderer::CompositeHDR()
{
	if (m_config.m_hdrEnabled)
	{
		HDRConstants hdrConstants;
		hdrConstants.m_exposure = m_config.m_hdrExposure;
		CopyCPUToGPU((void*)&hdrConstants, m_hdrCBO);
		BindConstantBuffer(k_HDRConstantsSlot, m_hdrCBO);

		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
		m_deviceContext->PSSetShaderResources(0, 1, &m_HDRTexture->m_shaderResourceView);

		BindShader(CreateOrGetShaderFromFile(GetShaderFilePath("CompositeHDR").c_str(), VertexType::VERTEX_TYPE_PCU));
		DrawVertexBuffer(m_fullScreenQuadNDCSpace, 6, 0, VertexType::VERTEX_TYPE_PCU);
		BindShader(nullptr);
	}
}

GPUMesh* Renderer::CreateOrGetGPUMeshFromFile(std::string const& path, Mat44 const& transform)
{
	for (int i = 0; i < (int)m_loadedGPUMeshes.size(); i++)
	{
		if (m_loadedGPUMeshes[i]->m_fileName == path && m_loadedGPUMeshes[i]->m_transform == transform)
		{
			return m_loadedGPUMeshes[i];
		}
	}
	return CreateGPUMeshFromFile(path, transform);
}

GPUMesh* Renderer::CreateGPUMeshFromFile(std::string const& path, Mat44 const& transform)
{
	bool wasAlreadyLoaded = false;
	CPUMesh* cpuMesh = CreateOrGetCPUMeshFromFile(path, transform);
	GPUMesh* gpuMesh = new GPUMesh(cpuMesh);
	m_loadedGPUMeshes.push_back(gpuMesh);
	if (wasAlreadyLoaded)
	{
		delete cpuMesh;
	}
	return gpuMesh;
}

CPUMesh* Renderer::CreateOrGetCPUMeshFromFile(std::string const& path, Mat44 const& transform)
{
	for (int i = 0; i < (int)m_loadedCPUMeshes.size(); i++)
	{
		if (m_loadedCPUMeshes[i]->m_meshFileName == path && m_loadedCPUMeshes[i]->m_transform == transform)
		{
			return m_loadedCPUMeshes[i];
		}
	}
	return CreateCPUMeshFromFile(path, transform);
}

CPUMesh* Renderer::CreateCPUMeshFromFile(std::string const& path, Mat44 const& transform)
{
	Strings fileNameSplit = SplitStringOnDelimiter(path, '.');
	CPUMesh* cpuMesh = new CPUMesh(g_theRenderer, path, transform);
	m_loadedCPUMeshes.push_back(cpuMesh);
	return cpuMesh;
}


void Renderer::ShutDown()
{
	for (size_t i = 0; i < m_loadedShaders.size(); i++)
	{
		if (m_loadedShaders[i] != nullptr)
		{
			delete m_loadedShaders[i];
			m_loadedShaders[i] = nullptr;
		}
	}
	m_currentShader = nullptr;

	for (size_t i = 0; i < m_loadedComputeShaders.size(); i++)
	{
		if (m_loadedComputeShaders[i] != nullptr)
		{
			delete m_loadedComputeShaders[i];
			m_loadedComputeShaders[i] = nullptr;
		}
	}

	for (int i = 0; i < m_loadedCPUMeshes.size(); i++)
	{
		delete m_loadedCPUMeshes[i];
	}
	for (int i = 0; i < m_loadedGPUMeshes.size(); i++)
	{
		delete m_loadedGPUMeshes[i];
	}

	DX_SAFE_RELEASE(m_device);
	DX_SAFE_RELEASE(m_deviceContext);
	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_renderTargetView);
	DX_SAFE_RELEASE(m_prevFrameDepthStencilSRV);
	DX_SAFE_RELEASE(m_depthStencilView);
	DX_SAFE_RELEASE(m_prevFrameDepthStencilView);

	DX_SAFE_RELEASE(m_depthStencilTexture);
	DX_SAFE_RELEASE(m_prevFrameDepthStencilTexture);
	DX_SAFE_RELEASE(m_depthStencilSRV);

	delete m_immediateIBO;
	delete m_immediateVBO;
	delete m_immediateTBNVBO;
	delete m_cameraCBO;
	delete m_modelCBO;
	delete m_lightingCBO;
	delete m_pointLightsCBO;
	delete m_hdrCBO;
	delete m_blurCBO;

	for (size_t i = 0; i < m_loadedTextures.size(); i++)
	{
		delete m_loadedTextures[i];
		m_loadedTextures[i] = nullptr;
	}

	delete m_emissiveRenderTexture;
	delete m_blurredEmissiveRenderTexture;
	for (int i = 0; i < m_blurDownTextures.size(); i++)
	{
		delete m_blurDownTextures[i];
		if (i != (int)m_blurDownTextures.size() - 1)
		{
			delete m_blurUpTextures[i];
		}
	}
	delete m_fullScreenQuadNDCSpace;

	for (size_t i = 0; i < m_loadedFonts.size(); i++)
	{
		delete m_loadedFonts[i];
		m_loadedFonts[i] = nullptr;
	}

	for (size_t i = 0; i < m_loadedSpriteSheets.size(); i++)
	{
		delete m_loadedSpriteSheets[i];
		m_loadedSpriteSheets[i] = nullptr;
	}

	for (int i = 0; i < (int)BlendMode::COUNT; i++)
	{
		DX_SAFE_RELEASE(m_blendStates[i]);
	}

	for (int i = 0; i < (int)SamplerMode::COUNT; i++)
	{
		DX_SAFE_RELEASE(m_samplerStates[i]);
	}

	for (int i = 0; i < (int)RasterizeMode::COUNT; i++)
	{
		DX_SAFE_RELEASE(m_rasterizeStates[i]);
	}

	for (int i = 0; i < (int)DepthMode::COUNT; i++)
	{
		DX_SAFE_RELEASE(m_depthModes[i]);
	}

	//cleanup ImGui
	if (m_config.m_enableImgui)
	{
		ImGui_ImplDX11_Shutdown();
	}

	// Report error leaks and release debug module
#if defined (ENGINE_DEBUG_RENDER)
	((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(
		DXGI_DEBUG_ALL,
		(DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
	);

	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;

	::FreeLibrary((HMODULE)m_dxgiDebugModule);
	m_dxgiDebugModule = nullptr;
#endif

}