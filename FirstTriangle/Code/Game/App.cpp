#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#if defined(_DEBUG)
#define ENGINE_DEBUG_RENDER
#endif

#if defined(ENGINE_DEBUG_RENDER)
#include "dxgidebug.h"
#pragma comment(lib, "dxguid.lib")
#endif

#include "Game/App.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Window.hpp"

ID3D11Device* m_device = nullptr;
ID3D11DeviceContext* m_deviceContext = nullptr;
IDXGISwapChain* m_swapChain = nullptr;
ID3D11RenderTargetView* m_renderTargetView = nullptr;
ID3D11VertexShader* m_vertexShader = nullptr;
ID3D11PixelShader* m_pixelShader = nullptr;
ID3D11InputLayout* m_inputLayoutForVertex_PCU = nullptr;
ID3D11Buffer* m_vertexBuffer = nullptr;
ID3D11RasterizerState* m_rasterizeState = nullptr;

std::vector<uint8_t> m_vertexShaderByteCode;
std::vector<uint8_t> m_pixelShaderByteCode;

#if defined(ENGINE_DEBUG_RENDER)
void* m_dxgiDebug = nullptr;
void* m_dxgiDebugModule = nullptr;
#endif

App* g_theApp = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_theWindow = nullptr;
RandomNumberGenerator* g_randGen = nullptr;

App::App()
{ }

App::~App()
{ }

void App::Run()
{
	while (!m_isQuitting)
	{
		RunFrame();
	}
}


void App::StartUp()
{
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

	//Create engine subsystems and game
	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	InputConfig inputConfig;
	g_theInput = new InputSystem( inputConfig );

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_windowTitle = "FirstTriangle";
	windowConfig.m_clientAspect = 2.f;
	g_theWindow = new Window(windowConfig);
	g_theInput->m_config.m_window = g_theWindow;

	AudioConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );

	//Start up engine subsystems and game
	g_theEventSystem->Startup();
	g_theInput->StartUp();
	g_theWindow->StartUp();
	g_theAudio->StartUp();
	
	//Render startup
	unsigned int deviceFlags = 0;
#if defined(ENGINE_DEBUG_RENDER)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//create device swapchain and device context
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
	swapChainDesc.BufferDesc.Width = g_theWindow->GetClientDimensions().x;
	swapChainDesc.BufferDesc.Height = g_theWindow->GetClientDimensions().y;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = (HWND)g_theWindow->GetHwnd();
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

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

	backbuffer->Release();

	//define a raw string of HLSL code to be compiled.
	const char* shaderSource = R"(
	struct vs_input_t
	{
		float3 localPosition : POSITION;
		float4 color : COLOR;
		float2 uv : TEXCOORD;
	};

	struct v2p_t
	{
		float4 position : SV_Position;
		float4 color : COLOR;
		float2 uv : TEXCOORD;
	};

	v2p_t VertexMain(vs_input_t input)
	{
		v2p_t v2p;
		v2p.position = float4(input.localPosition, 1);
		v2p.color = input.color;
		v2p.uv = input.uv;
		return v2p;
	}
	
	float4 PixelMain(v2p_t input) : SV_Target0
	{
		return float4(input.color);
	}
	)";

	//Compile the vertex shader by calling D3DCompile
	DWORD shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined(ENGINE_DEBUG_RENDER)
	shaderFlags = D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif
	ID3DBlob* shaderBlob =  NULL;
	ID3DBlob* errorBlob = NULL;

	hr = D3DCompile(
		shaderSource, strlen(shaderSource), 
		"VertexShader", nullptr, nullptr,
		"VertexMain", "vs_5_0", shaderFlags, 0,
		&shaderBlob, &errorBlob
	);

	if (SUCCEEDED(hr))
	{
		m_vertexShaderByteCode.resize(shaderBlob->GetBufferSize());
		memcpy(
			m_vertexShaderByteCode.data(),
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
		ERROR_AND_DIE(Stringf("Could not compile vertex shader."));
	}

	shaderBlob->Release();
	if (errorBlob != NULL)
	{
		errorBlob->Release();
	}

	//Create the vertex shader
	hr = m_device->CreateVertexShader(
	m_vertexShaderByteCode.data(),
	m_vertexShaderByteCode.size(),
	NULL, &m_vertexShader
	);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create the vertex shader."));
	}

	//Compile pixel shader
	hr = D3DCompile(
		shaderSource, strlen(shaderSource),
		"PixelShader", nullptr, nullptr,
		"PixelMain", "ps_5_0", shaderFlags, 0,
		&shaderBlob, &errorBlob
	);
	if (SUCCEEDED(hr))
	{
		m_pixelShaderByteCode.resize(shaderBlob->GetBufferSize());
		memcpy(
			m_pixelShaderByteCode.data(),
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
		ERROR_AND_DIE((Stringf("Could not xompile pixel shader.")));
	}
	shaderBlob->Release();
	
	// Create pixel shader
	hr = m_device->CreatePixelShader(
		m_pixelShaderByteCode.data(),
		m_pixelShaderByteCode.size(),
		NULL, &m_pixelShader);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create pixel shader"));
	}

	//create input layout
	//create a local array of input element descriptions that defines the vertex layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
			0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM,
			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	//call Id3D11Device::CreateINputLayout
	UINT numElements = ARRAYSIZE(inputElementDesc);
	hr = m_device->CreateInputLayout(
		inputElementDesc, numElements,
		m_vertexShaderByteCode.data(),
		m_vertexShaderByteCode.size(),
		&m_inputLayoutForVertex_PCU
	);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create vertex layout.");
	}
	
	//Create Vertex Buffer
	//add a hard coded array of verticies to define our triangle
	Vertex_PCU verticies[] = {
		Vertex_PCU(Vec3(-0.50f, -0.50f, 0.0f), Rgba8(255, 255, 255, 255), Vec2(0.0f, 0.0f)),
		Vertex_PCU(Vec3(0.00f, 0.50f, 0.0f), Rgba8(255, 255, 255, 255), Vec2(0.0f, 0.0f)),
		Vertex_PCU(Vec3(0.50f, -0.50f, 0.0f), Rgba8(255, 255, 255, 255), Vec2(0.0f, 0.0f)),
	};

	//Create a local D3D11_BUFFER_DESC variable
	UINT vertexBufferSize = (UINT)sizeof(verticies);
	D3D11_BUFFER_DESC bufferDesc = {0};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = vertexBufferSize;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//Call ID3D11Device::CreateBuffer
	hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_vertexBuffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create vertex buffer.");
	}

	//Copy the vertex buffer data from the CPU to the GPU
	// Copy verticies
	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, verticies, vertexBufferSize);
	m_deviceContext->Unmap(m_vertexBuffer, 0);

	//SET THE VIEWPORT
	//Create a D3D11_VIEWPORT variable
	D3D11_VIEWPORT viewport = {0};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)g_theWindow->GetClientDimensions().x;
	viewport.Height = (float)g_theWindow->GetClientDimensions().y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//Call ID3D11DeviceContext::RSSetViewports
	m_deviceContext->RSSetViewports(1, &viewport);

	//SET THE RASTERIZER STATE
	//Create a local D3D11_RASTERIZER_DESC variable
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = true;

	//Call ID3D11Device::CreateRasterizerState
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizeState);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create rasterizer state.");
	}
	m_deviceContext->RSSetState(m_rasterizeState);

	//SET PIPELINE STATE set up the remaining pipeline state
	UINT stride = sizeof(Vertex_PCU);
	UINT startOffset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &startOffset);
	m_deviceContext->IASetInputLayout(m_inputLayoutForVertex_PCU);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	m_deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	//get current time for updating
	m_timeLastFrame = GetCurrentTimeSeconds();
}

void App::RunFrame()
{
	double currTime = GetCurrentTimeSeconds();
	float deltaSeconds = static_cast<float>(currTime - m_timeLastFrame);
	deltaSeconds = Clamp(deltaSeconds, 0.f, .1f);
	m_timeLastFrame = currTime;
	BeginFrame();
	HandleSpecialCommands();
	//game is unpaused or the O key was pressed to advance one frame
	if (!m_isPaused || m_runOnce)
	{
		if (m_isSlowMo)
			deltaSeconds *= .1f;
		Update(deltaSeconds);
	}
	//handle Run one command
	if (m_runOnce)
	{
		m_runOnce = false;
		m_isPaused = true;
	}
	Render();
	EndFrame();
}

void App::BeginFrame()
{
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theAudio->BeginFrame();
}

void App::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}


void App::Render() const
{
	//Set render target
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

	//Clear the screen
	Rgba8 clearColor(127, 127, 127, 255);
	float colorAsFloats[4];
	clearColor.GetAsFloats(colorAsFloats);
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, colorAsFloats);

	//Draw
	m_deviceContext->Draw(3, 0);

	//Present
	HRESULT hr;
	hr = m_swapChain->Present(0, 0);
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		ERROR_AND_DIE("Device has been lost, applicaiton will now terminate.");
	}
}


void App::EndFrame()
{
	g_theInput->EndFrame();
	g_theAudio->EndFrame();
}

void App::HandleSpecialCommands()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_isQuitting = true;
	}
}


void App::Shutdown()
{
	g_theAudio->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->ShutDown();

	delete g_randGen;
	delete g_theAudio;
	delete g_theInput;

	m_rasterizeState->Release();
	m_vertexBuffer->Release();
	m_vertexShader->Release();
	m_pixelShader->Release();
	m_inputLayoutForVertex_PCU->Release();
	m_renderTargetView->Release();
	m_swapChain->Release();
	m_deviceContext->Release();
	m_device->Release();

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