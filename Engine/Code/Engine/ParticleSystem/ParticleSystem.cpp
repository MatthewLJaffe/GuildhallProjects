#include "Engine/ParticleSystem/ParticleSystem.hpp"
#include "ThirdParty/imgui/imgui.h"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndirectArgsBuffer.hpp"
#include "Engine/Renderer/SRV.hpp"
#include "Engine/Renderer/UAV.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Vertex_Particle.hpp"
#include "Engine/Renderer/ObjLoader.hpp"
#include "Engine/Renderer//Texture.hpp"
#include "Engine/ParticleSystem/ParticleEmitterDefinition.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/ParticleSystem/IndividualParticleEffect.hpp"
#include "ThirdParty/Squirrel/RawNoise.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include <d3d11.h>
#include <chrono>

ParticleSystem* g_theParticleSystem = nullptr;


ParticleSystem::ParticleSystem(ParticleSystemConfig const& config)
	: m_config(config)
	, m_showEditor(config.m_useImGUI)
{
}

void ParticleSystem::Startup()
{
	XmlDocument effectConfigDocument;
	GUARANTEE_OR_DIE(effectConfigDocument.LoadFile("Data/GameConfig.xml") == 0, Stringf("Failed to load effect config file Data/GameConfig.xml"));
	XmlElement* rootElement = effectConfigDocument.FirstChildElement("GameConfig");
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root element for effect config Data/GameConfig.xml"));
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*rootElement);

	m_spriteAtlasImage = Image(m_config.m_spriteAtlasResolution, Rgba8::MAGENTA);
	m_spriteAtlasImage.SetImageFilePath("ParticleSystemSpriteAtlas");
	m_spriteAtlasTexture = g_theRenderer->CreateTextureFromImage(m_spriteAtlasImage, false, true, false);
	m_particleConstants.m_spriteAtlasDimensions[0] = m_config.m_spriteAtlasResolution.x;
	m_particleConstants.m_spriteAtlasDimensions[1] = m_config.m_spriteAtlasResolution.y;
	InitResources();
	ToggleParticleEditor(m_showEditor);
	std::string shaderFilepath = g_gameConfigBlackboard.GetValue("engineDataPath", "not found");
	
	shaderFilepath += "/Shaders/";
	m_currentSeed = (unsigned int)std::time(0);
	g_theRenderer->CreateShader(GetShaderFilePath("Emissive").c_str(), VertexType::VERTEX_TYPE_PARTICLE);
	g_theRenderer->CreateShader(GetShaderFilePath("ParticleMesh").c_str(), VertexType::VERTEX_TYPE_MESH_PARTICLE);


	bool readInShaders = g_gameConfigBlackboard.GetValue("readInPrecompiledShaders", false);
	bool writeOutShaders = g_gameConfigBlackboard.GetValue("writeOutCompiledShaders", false);

	g_theRenderer->CreateComputeShader(GetShaderFilePath("ResetCounter.hlsl").c_str(), "CSMain", 
		writeOutShaders, GetShaderFilePath("Compiled/ResetCounter.cso"), readInShaders);
	g_theRenderer->CreateComputeShader(GetShaderFilePath("ParticleEmit.hlsl").c_str(), "CSMain", 
		writeOutShaders, GetShaderFilePath("Compiled/ParticleEmit.cso"), readInShaders);
	g_theRenderer->CreateComputeShader(GetShaderFilePath("ParticleSimulate.hlsl").c_str(), "CSMain", 
		writeOutShaders, GetShaderFilePath("Compiled/ParticleSimulate.cso"), readInShaders);
	g_theRenderer->CreateComputeShader(GetShaderFilePath("KickoffSort.hlsl").c_str(), "CSMain", 
		writeOutShaders, GetShaderFilePath("Compiled/KickoffSort.cso"), readInShaders);
	g_theRenderer->CreateComputeShader(GetShaderFilePath("AMD_SortCS.hlsl").c_str(), "CSMain",
		writeOutShaders, GetShaderFilePath("Compiled/AMD_SortCS.cso"), readInShaders);
	g_theRenderer->CreateComputeShader(GetShaderFilePath("AMD_SortStepCS.hlsl").c_str(), "CSMain", 
		writeOutShaders, GetShaderFilePath("Compiled/AMD_SortStepCS.cso"), readInShaders);
	g_theRenderer->CreateComputeShader(GetShaderFilePath("AMD_SortInnerCS.hlsl").c_str(), "CSMain", 
		writeOutShaders, GetShaderFilePath("Compiled/AMD_SortInnerCS.cso"), readInShaders);
	g_theRenderer->CreateComputeShader(GetShaderFilePath("PopulateVBO.hlsl").c_str(), "CSMain", 
		writeOutShaders, GetShaderFilePath("Compiled/PopulateVBO.cso"), readInShaders);
	g_theRenderer->CreateComputeShader(GetShaderFilePath("PopulateVertexCounts.hlsl").c_str(), "CSMain", 
		writeOutShaders, GetShaderFilePath("Compiled/PopulateVertexCounts.cso"), readInShaders);
	g_theRenderer->CreateComputeShader(GetShaderFilePath("PopulateVBOMeshes.hlsl").c_str(), "CSMain", 
		writeOutShaders, GetShaderFilePath("Compiled/PopulateVBOMeshes.cso"), readInShaders);
}

void ParticleSystem::InitResources()
{
	m_sortConstantsCBO = g_theRenderer->CreateConstantBuffer(sizeof(SortConstants));
	m_particleConstantsCBO = g_theRenderer->CreateConstantBuffer(sizeof(ParticleConstants));
	m_particlesBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(Particle), m_config.m_maxParticles, nullptr);
	m_particlesUAV = g_theRenderer->CreateUAV(m_particlesBuffer);
	m_particlesSRV = g_theRenderer->CreateSRV(m_particlesBuffer);

	CounterBuffer counterBuffer;
	counterBuffer.deadCount = m_config.m_maxParticles;
	counterBuffer.aliveCount = 0;
	counterBuffer.aliveCountAfterSim = 0;
	counterBuffer.drawCount = 0;
	m_counterBuffer = g_theRenderer->CreateRawBuffer(sizeof(CounterBuffer), (void*)&counterBuffer);
	m_counterUAV = g_theRenderer->CreateUAV(m_counterBuffer);
	m_counterSRV = g_theRenderer->CreateSRV(m_counterBuffer);
	unsigned int* aliveList1Buff = new unsigned int[m_config.m_maxParticles];
	unsigned int* aliveList2Buff = new unsigned int[m_config.m_maxParticles];
	unsigned int* deadListBuff = new unsigned int[m_config.m_maxParticles];
	unsigned int* drawListBuff = new unsigned int[m_config.m_maxParticles];

	for (unsigned int i = 0; i < (unsigned int)m_config.m_maxParticles; i++)
	{
		aliveList1Buff[i] = 0;
		aliveList2Buff[i] = 0;
		deadListBuff[i] = m_config.m_maxParticles - (i + 1);
	}

	m_particleAliveList1Buffer = g_theRenderer->CreateStructuredBuffer(sizeof(unsigned int), m_config.m_maxParticles, (void*)aliveList1Buff);
	m_particleAliveList1UAV = g_theRenderer->CreateUAV(m_particleAliveList1Buffer);
	m_particleAliveList1SRV = g_theRenderer->CreateSRV(m_particleAliveList1Buffer);
	m_particleAliveList2Buffer = g_theRenderer->CreateStructuredBuffer(sizeof(unsigned int), m_config.m_maxParticles, (void*)aliveList2Buff);
	m_particleAliveList2UAV = g_theRenderer->CreateUAV(m_particleAliveList2Buffer);
	m_particleAliveList2SRV = g_theRenderer->CreateSRV(m_particleAliveList2Buffer);
	m_deadListBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(unsigned int), m_config.m_maxParticles, (void*)deadListBuff);
	m_deadListUAV = g_theRenderer->CreateUAV(m_deadListBuffer);
	
	m_vbo = g_theRenderer->CreateVertexBufferAsUAV(m_config.m_maxParticles * 6 * sizeof(Vertex_Particle));
	m_vboUAV = g_theRenderer->CreateUAV(m_vbo);
	m_meshVBO = g_theRenderer->CreateVertexBufferAsUAV(m_config.m_maxParticles * 6 * sizeof(Vertex_Particle));
	m_meshVBOUAV = g_theRenderer->CreateUAV(m_meshVBO);

	m_particleDistanceBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(float), m_config.m_maxParticles, nullptr);
	m_particleDistanceUAV = g_theRenderer->CreateUAV(m_particleDistanceBuffer);
	m_particleDistanceSRV = g_theRenderer->CreateSRV(m_particleDistanceBuffer);
	m_particleDrawListBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(unsigned int), m_config.m_maxParticles, (void*)drawListBuff);
	m_particleDrawListUAV = g_theRenderer->CreateUAV(m_particleDrawListBuffer);
	m_particleDrawListSRV = g_theRenderer->CreateSRV(m_particleDrawListBuffer);

	m_meshParticleDistanceBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(float), m_config.m_maxParticles, nullptr);
	m_meshParticleDistanceUAV = g_theRenderer->CreateUAV(m_meshParticleDistanceBuffer);
	m_meshParticleDistanceSRV = g_theRenderer->CreateSRV(m_meshParticleDistanceBuffer);
	m_meshParticleDrawListBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(unsigned int), m_config.m_maxParticles, (void*)drawListBuff);
	m_meshParticleDrawListUAV = g_theRenderer->CreateUAV(m_meshParticleDrawListBuffer);
	m_meshParticleDrawListSRV = g_theRenderer->CreateSRV(m_meshParticleDrawListBuffer);

	CreateOrGetMeshParticleEntry("Data/Models/Cube_Textured.xml");

	//add one empty copy to structures that get copied cpu to gpu so that there is no issue
	m_livePhysicsObjects.push_back(ParticlePhysicsObjectGPU());
	m_livePhysicsAABB3s.emplace_back();
	ParticleEmitterDefinition emitterDef;
	EmplaceDefinitionDataForEmitter();

	delete[] aliveList1Buff;
	delete[] aliveList2Buff;
	delete[] deadListBuff;
	delete[] drawListBuff;

	m_rng = new RandomNumberGenerator();
}


void ParticleSystem::Update(float deltaSeconds)
{
	if (m_showEditor)
	{
		if (m_currentlyEditedEffect == nullptr)
		{
			ToggleParticleEditor(true);
		}
		UpdateIMGUI();
	}
	UpdateLiveEmitters(deltaSeconds);
	UpdateEmitterSRVData();
	CreatePhysicsObjectsSRV();
	EmitParticles(deltaSeconds);
	UpdateParticles(deltaSeconds);
	
	//reset kill signals
	for (int i = 0; i < (int)m_liveEmitterInstancesGPU.size(); i++)
	{
		m_liveEmitterInstancesGPU[i].m_killParticles = 0;
	}

	for (int i = 0; i < m_particleEffectsOwnedBySystem.size(); i++)
	{
		if (m_particleEffectsOwnedBySystem[i] != nullptr && !m_particleEffectsOwnedBySystem[i]->IsActive())
		{
			delete m_particleEffectsOwnedBySystem[i];
			m_particleEffectsOwnedBySystem[i] = nullptr;
		}
	}
}

void ParticleSystem::UpdateIMGUI()
{
	//ImGui::ShowDemoWindow();
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
	{
		UpdateParticleSystemIMGUI();
		UpdatePlayPanelIMGUI();
		for (int i = 0; i < (int)m_currentlyEditedEffect->m_emitters.size(); i++)
		{
			ParticleEmitter* currentlyEditedEmitter = m_currentlyEditedEffect->m_emitters[i];
			UpdateParticleEmitterIMGUI(currentlyEditedEmitter, i);
		}
	}
}

void ParticleSystem::UpdatePlayPanelIMGUI()
{
	ImGui::Begin("Play Panel");
	bool play = ImGui::Button("Play");
	ImGui::SameLine();
	static bool repeat =  false;
	static float playDuration = 10.f;
	ImGui::Checkbox("Repeat", &repeat);
	if (!repeat)
	{
		ImGui::InputFloat("Duration", &playDuration);
	}
	bool stop = ImGui::Button("Stop");
	ImGui::End();
	if (play)
	{
		float duration = repeat ? -1.f : playDuration;
		m_currentlyEditedEffect->Play(duration);
	}
	if (stop)
	{
		m_currentlyEditedEffect->Stop();
	}
}

void ParticleSystem::UpdateParticleSystemIMGUI()
{
	ImGui::Begin("Particle System");
	char effectBuffer[1024];
	std::string effectName = m_currentlyEditedEffect->GetEffectDefinition()->m_name;

	for (int ch = 0; ch < (int)effectName.size(); ch++)
	{
		effectBuffer[ch] = effectName[ch];
	}
	effectBuffer[effectName.size()] = '\0';
	ImGui::InputText("Effect Name", effectBuffer, 1024);
	std::string nameHere = m_currentlyEditedEffect->GetEffectDefinition()->m_name;
	m_currentlyEditedEffect->GetEffectDefinition()->m_name = std::string(effectBuffer);

	float effectPositionAsFloats[3] = { 0.f, 0.f, 0.f };
	m_currentlyEditedEffect->m_position.GetFromFloats(effectPositionAsFloats);
	ImGui::InputFloat3("Effect Position", effectPositionAsFloats, "%.3f");
	Vec3 newPosition(effectPositionAsFloats);
	m_currentlyEditedEffect->SetPosition(newPosition);

	float effectOrientationAsFloats[3] = { 0.f, 0.f, 0.f };
	m_currentlyEditedEffect->m_orientationDegrees.GetFromFloats(effectOrientationAsFloats);
	ImGui::SliderFloat3("Effect Orientation", effectOrientationAsFloats, 0.f, 360.f, "%.3f");
	EulerAngles effectOrientation;
	effectOrientation.SetFromFloats(effectOrientationAsFloats);
	m_currentlyEditedEffect->SetOrientationDegrees(effectOrientation);

	float currentScale = m_currentlyEditedEffect->GetScale();
	ImGui::InputFloat("Effect Scale", &currentScale);
	m_currentlyEditedEffect->SetScale(currentScale);

	ImGui::Checkbox("Debug Draw", &m_debugDraw);
	bool addEmitterPressed = ImGui::Button("Add Emitter");
	if (addEmitterPressed)
	{
		ParticleEmitterInitializer addedEmitter;
		int emitterDefIndex = EmplaceDefinitionDataForEmitter();
		addedEmitter.m_emitterIndex = emitterDefIndex;
		m_currentlyEditedEffect->AddEmitter(addedEmitter);
	}

	bool saveEffect = ImGui::Button("Save Effect");
	if (saveEffect)
	{
		SaveCurrentlyEditedEffectAsXML();
	}

	bool loadEffect = ImGui::Button("Load Effect");
	if (loadEffect)
	{
		std::string filePath = Window::GetTheWindowInstance()->GetFileFromWindowsExplorer();
		if (filePath == "")
		{

		}
		else if (SplitStringOnDelimiter(filePath, '.')[1] != "xml")
		{
			ERROR_RECOVERABLE("Effect must be a .xml file");
		}
		else
		{
			ParticleEffectDefinition newEffectConfig = CreateOrGetEffectDefinitionFromFile(filePath);

			if (m_currentlyEditedEffect->GetEffectDefinition()->m_name != newEffectConfig.m_name)
			{
				delete m_currentlyEditedEffect;
				m_currentlyEditedEffect = new ParticleEffect(newEffectConfig, newEffectConfig.m_effectDefinitionIndex);
				m_currentlyEditedEffect->Stop();
				m_currentlyEditedEffect->Play(-1.f);
			}
		}
	}
	bool loadEmitter = ImGui::Button("Load Emitter");
	if (loadEmitter)
	{
		std::string filePath = Window::GetTheWindowInstance()->GetFileFromWindowsExplorer();
		if (filePath == "")
		{

		}
		else if (SplitStringOnDelimiter(filePath, '.')[1] != "xml")
		{
			ERROR_RECOVERABLE("Emitter must be a .xml file");
		}
		else
		{
			int defIdxForEmitterToAdd = CreateOrGetEmitterDefinitionFromFile(filePath);
			ParticleEmitterInitializer addedEmitter;
			addedEmitter.m_emitterIndex = defIdxForEmitterToAdd;

			m_currentlyEditedEffect->AddEmitter(addedEmitter);
		}
	}

	ImGui::Text("Live Particles: %d", (int)m_aliveCountFromlastFrame);
	ImGui::Text("Dead Particles: %d", (int)m_deadCountFromLastFrame);
	ImGui::Text("Culled Particles: %d", (int)m_culledCountLastFrame);
	ImGui::End();
}


void ParticleSystem::UpdateParticleEmitterIMGUI(ParticleEmitter* currentlyEditedEmitter, int& index)
{
	int emitterDefIdx = m_currentlyEditedEffect->GetEffectDefinition()->m_particleEmitterDefs[index].m_emitterIndex;

	bool emitterWindowOpen = true;
	std::string emitterName = Stringf("Emitter %d", index);
	ImGui::Begin(emitterName.c_str(), &emitterWindowOpen);
	float emitterPositionAsFloats[3] = { 0.f, 0.f, 0.f };
	currentlyEditedEmitter->m_position.GetFromFloats(emitterPositionAsFloats);

	char nameBuffer[1024];
	std::string currentName = m_loadedEmitterDefinitions[emitterDefIdx].m_name;

	for (int ch = 0; ch < (int)currentName.size(); ch++)
	{
		nameBuffer[ch] = currentName[ch];
	}
	nameBuffer[currentName.size()] = '\0';
	ImGui::InputText("Emitter Name", nameBuffer, 1024);
	m_loadedEmitterDefinitions[emitterDefIdx].m_name = std::string(nameBuffer);

	ImGui::InputFloat3("Emitter Position", emitterPositionAsFloats, "%.3f");
	Vec3 localPosition;
	localPosition.SetFromFloats(emitterPositionAsFloats);
	if (localPosition != currentlyEditedEmitter->GetLocalPosition())
	{
		m_currentlyEditedEffect->GetEffectDefinition()->m_particleEmitterDefs[index].m_position = localPosition;
		currentlyEditedEmitter->SetLocalPosition(localPosition);
	}
	float emitterOrientationAsFloats[3] = { 0.f, 0.f, 0.f };
	currentlyEditedEmitter->m_orientationDegrees.GetFromFloats(emitterOrientationAsFloats);

	ImGui::SliderFloat3("Emitter Orientation", emitterOrientationAsFloats, 0.f, 360.f, "%.0f");
	EulerAngles localOrientation;
	localOrientation.SetFromFloats(emitterOrientationAsFloats);
	if (localOrientation != currentlyEditedEmitter->GetLocalOrientationDegrees())
	{
		m_currentlyEditedEffect->GetEffectDefinition()->m_particleEmitterDefs[index].m_orientationDegrees = localOrientation;
		currentlyEditedEmitter->SetLocalOrientationDegrees(localOrientation);
	}
	m_currentlyEditedEffect->GetEffectDefinition()->m_particleEmitterDefs[index].m_orientationDegrees = localOrientation;

	EmitterUpdateDefinitionGPU* updateDef = currentlyEditedEmitter->GetEmitterUpdateDef();
	EmitterRenderDefinitionGPU* renderDef = currentlyEditedEmitter->GetEmitterRenderDef();
	EmitterInstanceGPU* emitterInstanceGPU = currentlyEditedEmitter->GetEmitterInstanceGPU();
	EmitterInstanceCPU* emitterInstanceCPU = currentlyEditedEmitter->GetEmitterInstanceCPU();
	ParticleEmitterDefinition* emitterDef = currentlyEditedEmitter->GetEmitterDefininition();

	if (ImGui::CollapsingHeader("Emitter Properties"))
	{
		bool emitterHasLifetime = emitterInstanceCPU->m_emitterLifetime != -1.f;
		ImGui::Checkbox("Emitter Has Lifetime", &emitterHasLifetime);
		float prevEmitterLifetime = emitterInstanceCPU->m_emitterLifetime;
		if (!emitterHasLifetime && emitterInstanceCPU->m_emitterLifetime != -1.f)
		{
			emitterInstanceCPU->m_emitterLifetime = -1.f;
			m_loadedEmitterDefinitions[emitterDefIdx].m_emitterLifetime = emitterInstanceCPU->m_emitterLifetime;
		}
		if (emitterHasLifetime)
		{
			if (emitterInstanceCPU->m_emitterLifetime == -1.f)
			{
				emitterInstanceCPU->m_emitterLifetime = 10.f;
			}
			ImGui::InputFloat("Emitter Lifetime", &emitterInstanceCPU->m_emitterLifetime);
			m_loadedEmitterDefinitions[emitterDefIdx].m_emitterLifetime = emitterInstanceCPU->m_emitterLifetime;
		}
		if (prevEmitterLifetime != emitterInstanceCPU->m_emitterLifetime)
		{
			emitterInstanceCPU->m_emitTimeLeft = emitterInstanceCPU->m_emitterLifetime;
		}

		float prevStartDelay = emitterDef->m_emitterStartDelay;
		ImGui::InputFloat("Start Delay", &emitterDef->m_emitterStartDelay);
		if (prevStartDelay != emitterDef->m_emitterStartDelay)
		{
			emitterInstanceCPU->m_delayTimeLeft = emitterDef->m_emitterStartDelay;
		}

		CustomInputFloat3("Emitter Velocity", m_emitterVelocityGraphs[emitterInstanceGPU->m_definitionIndex], Vec3::ZERO);

		const char* emitType[] = { "Particle", "Emitter" };
		ImGui::ListBox("Emission Types", (int*)&emitterInstanceCPU->m_spawnSubEmitters, emitType, IM_ARRAYSIZE(emitType), 2);
		m_loadedEmitterDefinitions[emitterDefIdx].m_emitEmitters = emitterInstanceCPU->m_spawnSubEmitters;
		if (m_loadedEmitterDefinitions[emitterDefIdx].m_emitEmitters)
		{
			if (ImGui::Button("Select Emitter to Spawn"))
			{
				std::string emitterToSpawnFile = Window::GetTheWindowInstance()->GetFileFromWindowsExplorer();
				if (emitterToSpawnFile != "")
				{
					std::string fileExtension = SplitStringOnDelimiter(emitterToSpawnFile, '.')[1];

					if (fileExtension != "xml")
					{
						ERROR_RECOVERABLE("must be a .xml file");
					}
					else
					{
						m_loadedEmitterDefinitions[emitterDefIdx].m_subEmitterFilePath = emitterToSpawnFile;
						XmlDocument emitterDocument;
						GUARANTEE_OR_DIE(emitterDocument.LoadFile(emitterToSpawnFile.c_str()) == 0, Stringf("Failed to load emmiter def file %s", emitterToSpawnFile.c_str()));
						XmlElement* rootElement = emitterDocument.FirstChildElement("Emitter");
						GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root element for emmiter def file %s", emitterToSpawnFile.c_str()));
						std::string spawnEmitterName = ParseXmlAttribute(*rootElement, "name", "Missing");
						if (spawnEmitterName == "Missing")
						{
							ERROR_RECOVERABLE("Provide a valid Emitter xml file");
						}
						else
						{
							//emitter def has not been loaded yet
							if (emitterInstanceCPU->m_subEmitterDefIdx == -1)
							{
								emitterInstanceCPU->m_subEmitterDefIdx = CreateOrGetEmitterDefinitionFromFile(emitterToSpawnFile);
							}
						}
					}
				}
			}
		}
		else
		{
			const char* simSpace[] = { "Local", "World" };
			ImGui::ListBox("Simulation Space", (int*)&updateDef->m_worldSimulation, simSpace, IM_ARRAYSIZE(simSpace), 2);
		}
		UpdateChildEmitters(currentlyEditedEmitter, emitterDefIdx);
	}

	if (emitterInstanceCPU->m_spawnSubEmitters)
	{
		UpdateSubEmittersIMGUI(currentlyEditedEmitter, index, updateDef, renderDef, emitterInstanceGPU, emitterInstanceCPU, emitterDefIdx);
	}
	else
	{
		UpdateNormalParticleEmitterIMGUI(currentlyEditedEmitter, index, updateDef, renderDef, emitterInstanceGPU, emitterInstanceCPU, emitterDefIdx);
	}

	if (!emitterWindowOpen)
	{
		ParticleEffectDefinition* currentlyEditedEffectDefinition = m_currentlyEditedEffect->GetEffectDefinition();
		currentlyEditedEffectDefinition->m_particleEmitterDefs.erase(currentlyEditedEffectDefinition->m_particleEmitterDefs.begin() + index);
		delete currentlyEditedEmitter;
		index--;
	}
	else
	{
		currentlyEditedEmitter->GetEmitterUpdateDef()->isDirty = true;
		currentlyEditedEmitter->GetEmitterRenderDef()->isDirty = true;

		for (int i = 0; i < (int)FloatGraphType::NUM_FLOATGRAPHS; i++)
		{
			currentlyEditedEmitter->GetFloatGraphByType((FloatGraphType)i)->isDirty = 1;
		}
		for (int i = 0; i < (int)Float2GraphType::NUM_FLOAT2GRAPHS; i++)
		{
			currentlyEditedEmitter->GetFloat2GraphByType((Float2GraphType)i)->isDirty = 1;
		}
		for (int i = 0; i < (int)Float3GraphType::NUM_FLOAT3GRAPHS; i++)
		{
			currentlyEditedEmitter->GetFloat3GraphByType((Float3GraphType)i)->isDirty = 1;
		}
	}
}

void ParticleSystem::UpdateSubEmittersIMGUI(ParticleEmitter* currentlyEditedEmitter, int& index, EmitterUpdateDefinitionGPU* updateDef,
	EmitterRenderDefinitionGPU* renderDef, EmitterInstanceGPU* emitterInstanceGPU, EmitterInstanceCPU* emitterInstanceCPU, int emitterDefIdx)
{
	UNUSED(emitterInstanceGPU);
	UNUSED(index);
	UNUSED(currentlyEditedEmitter);

	//make emission shape box
	updateDef->m_emissionType = 1;
	if (ImGui::CollapsingHeader("Emission"))
	{
		ImGui::InputFloat("Emission Rate", &updateDef->m_emissionRate, 1.0f, 10.0f, "%.3f");
		ImGui::InputFloat2("Lifetime Range", updateDef->m_lifetime, "%.3f");
		renderDef->m_lifetime[0] = updateDef->m_lifetime[0];
		renderDef->m_lifetime[1] = updateDef->m_lifetime[1];

		const char* modes[] = { "Continuous", "Burst" };
		unsigned int prevEmissionMode = updateDef->m_emissionMode;
		ImGui::ListBox("Emission Modes", (int*)&updateDef->m_emissionMode, modes, IM_ARRAYSIZE(modes), 2);

		if (updateDef->m_emissionMode == (unsigned int)EmissionMode::CONTINUOUS)
		{
			bool emitForLimitedTime = updateDef->m_emitTime != -1;
			bool prevEmitForLimitedTime = emitForLimitedTime;
			ImGui::Checkbox("Emit for Limited Time", &emitForLimitedTime);

			if (!emitForLimitedTime)
			{
				updateDef->m_emitTime = -1.f;
			}
			else
			{
				if (prevEmitForLimitedTime != emitForLimitedTime)
				{
					updateDef->m_emitTime = 1.f;
				}
				float prevEmitTime = updateDef->m_emitTime;
				ImGui::InputFloat("Emit Time", &updateDef->m_emitTime);

				if (prevEmitTime != updateDef->m_emitTime)
				{
					emitterInstanceCPU->m_emitTimeLeft = updateDef->m_emitTime;
				}
			}
		}
		if (updateDef->m_emissionMode == (unsigned int)EmissionMode::BURST)
		{
			//if states change re-emit particles
			if (prevEmissionMode != updateDef->m_emissionMode)
			{
				emitterInstanceCPU->m_currentRepeatTime = 0.f;
			}
			unsigned int prevRepeat = updateDef->m_repeat;
			ImGui::Checkbox("Repeat", (bool*)&updateDef->m_repeat);
			//if states change re-emit particles
			if (prevRepeat != updateDef->m_repeat)
			{
				emitterInstanceCPU->m_currentRepeatTime = 0.f;
			}
			if (updateDef->m_repeat == 1)
			{
				float prevRepeatTime = updateDef->m_repeatTime;
				ImGui::InputFloat("Repeat Time", &updateDef->m_repeatTime);
				//if states change re-emit particles
				if (prevRepeatTime != updateDef->m_repeatTime)
				{
					emitterInstanceCPU->m_currentRepeatTime = 0.f;
				}
			}
			ImGui::InputInt("Number of Bursts", &m_loadedEmitterDefinitions[emitterDefIdx].m_numBursts);
			ImGui::InputFloat("Interval Between Bursts", &m_loadedEmitterDefinitions[emitterDefIdx].m_burstInterval);
		}

		ImGui::InputFloat3("Emission Box Dimensions", updateDef->m_boxDimensions, "%.3f");
		ImGui::InputFloat3("Min SubEmitter Orientation", m_loadedEmitterDefinitions[emitterDefIdx].m_minSubEmitterOrientation, "%.3f");
		ImGui::InputFloat3("Max SubEmitter Orientation", m_loadedEmitterDefinitions[emitterDefIdx].m_maxSubEmitterOrientation, "%.3f");
	}
	ImGui::End();
}

void ParticleSystem::UpdateNormalParticleEmitterIMGUI(ParticleEmitter* currentlyEditedEmitter, int& index, EmitterUpdateDefinitionGPU* updateDef, 
	EmitterRenderDefinitionGPU* renderDef, EmitterInstanceGPU* emitterInstanceGPU, EmitterInstanceCPU* emitterInstanceCPU, int emitterDefIdx)
{
	UNUSED(emitterInstanceGPU);
	UNUSED(emitterDefIdx);
	if (ImGui::CollapsingHeader("Emission"))
	{
		ImGui::InputFloat("Emission Rate", &updateDef->m_emissionRate, 1.0f, 10.0f, "%.3f");
		//ImGui::InputFloat("Start Delay", )
		ImGui::InputFloat2("Lifetime Range", updateDef->m_lifetime, "%.3f");
		renderDef->m_lifetime[0] = updateDef->m_lifetime[0];
		renderDef->m_lifetime[1] = updateDef->m_lifetime[1];

		const char* modes[] = { "Continuous", "Burst" };
		unsigned int prevEmissionMode = updateDef->m_emissionMode;
		ImGui::ListBox("Emission Modes", (int*)&updateDef->m_emissionMode, modes, IM_ARRAYSIZE(modes), 2);

		if (updateDef->m_emissionMode == (unsigned int)EmissionMode::CONTINUOUS)
		{
			bool emitForLimitedTime = updateDef->m_emitTime != -1.f;
			bool prevEmitForLimitedTime = emitForLimitedTime;
			ImGui::Checkbox("Emit for Limited Time", &emitForLimitedTime);

			if (!emitForLimitedTime)
			{
				updateDef->m_emitTime = -1.f;
			}
			else
			{
				if (prevEmitForLimitedTime != emitForLimitedTime)
				{
					updateDef->m_emitTime = 1.f;
					emitterInstanceCPU->m_emitTimeLeft = updateDef->m_emitTime;
				}
				float prevEmitTime = updateDef->m_emitTime;
				ImGui::InputFloat("Emit Time", &updateDef->m_emitTime);

				if (prevEmitTime != updateDef->m_emitTime)
				{
					emitterInstanceCPU->m_emitTimeLeft = updateDef->m_emitTime;
				}
			}
		}
		if (updateDef->m_emissionMode == (unsigned int)EmissionMode::BURST)
		{
			//if states change re-emit particles
			if (prevEmissionMode != updateDef->m_emissionMode)
			{
				emitterInstanceCPU->m_currentRepeatTime = 0.f;
			}
			unsigned int prevRepeat = updateDef->m_repeat;
			ImGui::Checkbox("Repeat", (bool*)&updateDef->m_repeat);
			//if states change re-emit particles
			if (prevRepeat != updateDef->m_repeat)
			{
				emitterInstanceCPU->m_currentRepeatTime = 0.f;
			}
			if (updateDef->m_repeat == 1)
			{
				float prevRepeatTime = updateDef->m_repeatTime;
				ImGui::InputFloat("Repeat Time", &updateDef->m_repeatTime);
				//if states change re-emit particles
				if (prevRepeatTime != updateDef->m_repeatTime)
				{
					emitterInstanceCPU->m_currentRepeatTime = 0.f;
				}
			}
			ImGui::InputInt("Number of Bursts", &m_loadedEmitterDefinitions[emitterDefIdx].m_numBursts);
			ImGui::InputFloat("Interval Between Bursts", &m_loadedEmitterDefinitions[emitterDefIdx].m_burstInterval);
		}


		const char* shapes[] = { "Sphere", "Box", "Mesh" };
		ImGui::ListBox("Emission Shape", (int*)&updateDef->m_emissionType, shapes, IM_ARRAYSIZE(shapes), 3);
		if (updateDef->m_emissionType == (unsigned int)EmissionType::SPHERE)
		{
			ImGui::InputFloat("Emission Radius", &updateDef->m_emissionRadius, .5f, .1f, "%.3f");
		}
		if (updateDef->m_emissionType == (unsigned int)EmissionType::BOX)
		{
			ImGui::InputFloat3("Emission Box Dimensions", updateDef->m_boxDimensions, "%.3f");
		}
		if (updateDef->m_emissionType == (unsigned int)EmissionType::MESH)
		{
			bool loadMesh = ImGui::Button("Load Mesh");
			if (loadMesh)
			{
				std::string fileToLoad = Window::GetTheWindowInstance()->GetFileFromWindowsExplorer();
				updateDef->m_meshEntryEmissionIndex = CreateOrGetMeshParticleEntry(fileToLoad);
				m_currentlyEditedEffect->GetEffectDefinition()->m_particleEmitterDefs[index].GetParticleEmitterDefinition().m_meshFilePath = fileToLoad;
			}
			ImGui::InputFloat3("Mesh Scale", updateDef->m_meshScale, "%.3f");
		}
		else
		{
			const char* items[] = { "Linear", "Smooth Start 2", "Smooth Start 3", "Smooth Start 4", "Smooth Start 5",
			"Smooth Start 6", "Smooth Stop 2", "Smooth Stop 3", "Smooth Stop 4", "Smooth Stop 5", "Smooth Stop 6",
			"Smooth Step 3", "Smooth Step 5", "Hesitate 3", "Hesitate 5"
			};

			ImGui::ListBox("Distribution From Center", (int*)&updateDef->m_emissionDistribution, items, IM_ARRAYSIZE(items), 8);
		}
	}

	if (ImGui::CollapsingHeader("Motion"))
	{
		//billboard
		
		if (updateDef->m_particleMeshIndex < 0)
		{
			FloatGraph* rotationGraph = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_ROTATION_1D);
			CustomInputFloat("Billboard Rotation", *rotationGraph, 0.f);
		}
		
		else
		{
			Float3Graph* rotationGraph = currentlyEditedEmitter->GetFloat3GraphByType(Float3GraphType::FLOAT3GRAPH_LIFETIME_ROTATION_3D);
			CustomInputFloat3("Particle Rotation", *rotationGraph, Vec3::ZERO);
		}
		

		ImGui::Checkbox("Set Lifetime Position", (bool*)&updateDef->m_setLifetimePosition);
		if (updateDef->m_setLifetimePosition == 1)
		{
			Float3Graph* positionGraph = currentlyEditedEmitter->GetFloat3GraphByType(Float3GraphType::FLOAT3GRAPH_LIFETIME_POSITION);
			CustomInputFloat3("Lifetime Position", *positionGraph, Vec3(0.f, 0.f, 0.f));
		}
		else
		{
			if (ImGui::TreeNode("Velocity"))
			{
				ImGui::Checkbox("Set Lifetime Velocity", (bool*)&updateDef->m_setLifetimeVelocity);
				if (updateDef->m_setLifetimeVelocity == 1)
				{
					const char* items[] = { "Linear", "Radial" };

					ImGui::ListBox("Velocity Mode", (int*)&updateDef->m_velocityMode, items, IM_ARRAYSIZE(items), 2);

					if (updateDef->m_velocityMode == 0)
					{
						Float3Graph* lifetimeVelocity = currentlyEditedEmitter->GetFloat3GraphByType(Float3GraphType::FLOAT3GRAPH_LIFETIME_VELOCITY);
						CustomInputFloat3("Lifetime Velocity", *lifetimeVelocity, Vec3(0.f, 0.f, 10.f));
					}
					else
					{
						FloatGraph* radialVelocity = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_RADIAL_VELOCITY);
						CustomInputFloat("Lifetime Velocity", *radialVelocity, 0.f);
					}

				}
				else
				{
					ImGui::InputFloat2("Start Velocity Range X", updateDef->m_velocityXRange, "%.3f");
					ImGui::InputFloat2("Start Velocity Range Y", updateDef->m_velocityYRange, "%.3f");
					ImGui::InputFloat2("Start Velocity Range Z", updateDef->m_velocityZRange, "%.3f");
					ImGui::Checkbox("Inherit Emitter Velocity", (bool*)&updateDef->m_inheritEmitterVelocity);
					if (updateDef->m_inheritEmitterVelocity == 1)
					{
						ImGui::InputFloat("Inherited Velocity Percentage", &updateDef->m_inheritVelocityPercentage);
					}
				}

				FloatGraph* maxSpeedFloatGraph = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_MAX_SPEED);
				CustomInputFloat("Max Speed", *maxSpeedFloatGraph, 10.f);

				ImGui::TreePop();
				ImGui::Checkbox("Orient to Velocity", (bool*)&updateDef->m_orientToVelocity);
				//billboard
				if (updateDef->m_particleMeshIndex < 0)
				{
					ImGui::Checkbox("Stretch Billboard", (bool*)&updateDef->m_stretchBillboard);
					if (updateDef->m_stretchBillboard == 1)
					{
						ImGui::InputFloat("Billboard Length Per Speed", &updateDef->m_lengthPerSpeed);
					}
				}
			}
			if (ImGui::TreeNode("Noise"))
			{
				ImGui::InputFloat("Perlin Noise Force", &updateDef->m_perlinNoiseForce, .5f, .1f, "%.3f");
				FloatGraph* curlNoiseForce = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_CURL_NOISE_FORCE);
				ImGui::Checkbox("Curl Noise Effect Position", (bool*)&updateDef->m_curlNoiseAffectPosition);
				CustomInputFloat("Curl Noise Strength", *curlNoiseForce, 0.f);
				ImGui::InputFloat("Curl Noise Scale", &updateDef->m_curlNoiseScale, .5f, .1f, "%.3f");
				ImGui::InputFloat("Curl Noise Sample Size", &updateDef->m_curlNoiseSampleSize, .5f, .1f, "%.3f");
				ImGui::InputInt("Curl Noise Sample Octaves", &updateDef->m_curlNoiseOctives, 1, 1);
				ImGui::InputFloat3("Curl Noise Pan", updateDef->m_curlNoisePan, "%.3f");
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Forces"))
			{
				FloatGraph* dragForce = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_DRAG_FORCE);
				CustomInputFloat("Drag Force", *dragForce, 0.f);

				Float3Graph* linearForce = currentlyEditedEmitter->GetFloat3GraphByType(Float3GraphType::FLOAT3GRAPH_LINEAR_FORCE);
				CustomInputFloat3("Linear Force", *linearForce, Vec3(0.f, 0.f, 10.f));

				if (ImGui::TreeNode("Point Force"))
				{
					FloatGraph* pointForce = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_POINT_FORCE);
					CustomInputFloat("Point Force Strength", *pointForce, 0.f);
					float pointForcePos[3] = { 0.f, 0.f, 0.f };
					ImGui::InputFloat3("Point Force Position", updateDef->m_pointForcePosition, "%.3f");
					ImGui::InputFloat("Point Force Radius", &updateDef->m_pointForceRadius, .5f, .1f, "%.3f");
					ImGui::InputFloat("Point Force Falloff Exponent", &updateDef->m_pointForceFalloffExponent, .5f, .1f, "%.3f");
					ImGui::Checkbox("Point Force Attract", (bool*)&updateDef->m_pointForceAttract);
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Vortex Force"))
				{
					FloatGraph* vortexForce = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_VORTEX_FORCE);
					CustomInputFloat("Vortex Force Strength", *vortexForce, 0.f);
					ImGui::InputFloat3("Vortex Axis", updateDef->m_vortexAxisDir, "%.3f");
					float vortexAxisOriginAsFloats[3] = { 0.f, 0.f, 0.f };
					updateDef->m_vortexAxisOrigin.GetFromFloats(vortexAxisOriginAsFloats);
					ImGui::InputFloat3("Vortex Position", vortexAxisOriginAsFloats, "%.3f");
					updateDef->m_vortexAxisOrigin.SetFromFloats(vortexAxisOriginAsFloats);
					ImGui::InputFloat("Vortex Radius", &updateDef->m_vortexForceRadius, .5f, .1f, "%.3f");
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Return Force"))
				{
					ImGui::InputFloat("Return Force Strength", &updateDef->m_returnToOriginForce, .5f, .1f, "%.3f");
					ImGui::InputFloat("Return Force Delay", &updateDef->m_returnToOriginDelay, .5f, .1f, "%.3f");
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			ImGui::Checkbox("Depth Buffer Collisions", (bool*)&updateDef->m_depthBufferCollisions);
			ImGui::Checkbox("Ignore World Forces", (bool*)&updateDef->m_ignoreWorldPhysics);
		}
	}


	bool spriteSheetPressed = false;
	if (ImGui::CollapsingHeader("Appearance"))
	{
		UpdateColorOverLifetime(updateDef);
		//billboard
		if (updateDef->m_particleMeshIndex < 0)
		{
			if (ImGui::TreeNode("Size"))
			{
				const char* stretchModes[] = { "Center", "Left", "Right", "Up", "Down" };
				ImGui::ListBox("Stretch Mode", (int*)&updateDef->m_stretchMode, stretchModes, IM_ARRAYSIZE(stretchModes), 5);

				Float2Graph* size = currentlyEditedEmitter->GetFloat2GraphByType(Float2GraphType::FLOAT2GRAPH_SIZE);
				CustomInputFloat2("Size", *size, Vec2(1.f, 1.f));
				ImGui::TreePop();
			}
		}
		else
		{
			Float3Graph* scale = currentlyEditedEmitter->GetFloat3GraphByType(Float3GraphType::FLOAT3GRAPH_LIFETIME_SCALE_3D);
			CustomInputFloat3("Scale", *scale, Vec3(1.f, 1.f, 1.f));
		}
		
		FloatGraph* emissive = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_EMISSIVE);
		CustomInputFloat("Emissive", *emissive, 0.f);

		FloatGraph* alphaObscurance = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_ALPHA_OBSCURANCE);
		CustomInputFloat("Alpha Obscurance", *alphaObscurance, 1.f);
		spriteSheetPressed = ImGui::Button("Spritesheet");
		ImGui::InputInt2("Sprite Sheet Dimensions", updateDef->m_spriteSheetdimensions);
		ImGui::InputInt("Sprite Sheet Start index", &updateDef->m_spriteStartIndex);
		ImGui::InputInt("Sprite Sheet End index", &updateDef->m_spriteEndIndex);
		const char* easingItems[] = { "Linear", "Smooth Start 2", "Smooth Start 3", "Smooth Start 4", "Smooth Start 5",
					"Smooth Start 6", "Smooth Stop 2", "Smooth Stop 3", "Smooth Stop 4", "Smooth Stop 5", "Smooth Stop 6",
					"Smooth Step 3", "Smooth Step 5", "Hesitate 3", "Hesitate 5", "Smooth Start 1.5", "Smooth Stop 1.5"
		};

		ImGui::ListBox("Easing Function", (int*)&updateDef->m_spriteEasingFunction, easingItems, IM_ARRAYSIZE(easingItems), 8);

		if (ImGui::TreeNode("Pan Texture"))
		{
			bool textureToPanSelected = false;
			textureToPanSelected = ImGui::Button("Texture To Pan");
			if (textureToPanSelected)
			{
				std::string textureFilePath = Window::GetTheWindowInstance()->GetFileFromWindowsExplorer();
				m_currentlyEditedEffect->GetEffectDefinition()->m_particleEmitterDefs[index].GetParticleEmitterDefinition().m_panTextureFilePath = textureFilePath;
				renderDef->m_panTextureUVBounds = GetImageBoundsInSpriteAtlas(textureFilePath);
			}

			FloatGraph* panTextureContribution = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_PAN_TEXTURE_CONTRIBUTION);
			CustomInputFloat("Pan Texture Contribution", *panTextureContribution, 0.f);
			ImGui::InputFloat2("Pan Texture Speed", renderDef->m_panTextureSpeed, "%.3f");
			ImGui::InputFloat2("Pan Texture Sample Scale", renderDef->m_panTextureSampleScale, "%.3f");
			ImGui::TreePop();
		}

		ParticleEmitterDefinition* emitterDef = currentlyEditedEmitter->GetEmitterDefininition();
		const char* meshModes[] = { "Billboard", "3D Mesh", "Partial Mesh"};
		ImGui::ListBox("Mesh Render Mode", (int*)&emitterDef->m_renderMode, meshModes, IM_ARRAYSIZE(meshModes), 3);
		if (emitterDef->m_renderMode > 0)
		{
			if (updateDef->m_particleMeshIndex == -1)
			{
				updateDef->m_particleMeshIndex = 0;
			}
			bool loadParticleMesh = false;
			loadParticleMesh = ImGui::Button("Load Mesh");
			if (loadParticleMesh)
			{
				std::string particleMeshToGet = Window::GetTheWindowInstance()->GetFileFromWindowsExplorer();
				emitterDef->m_particleMeshFilePath = particleMeshToGet;
				if (SplitStringOnDelimiter(particleMeshToGet, '.')[1] != "xml")
				{
					ERROR_RECOVERABLE("Particle Mesh must be an .xml file");
				}
				else
				{
					updateDef->m_particleMeshIndex = CreateOrGetMeshParticleEntry(particleMeshToGet);
				}
			}
			if (emitterDef->m_renderMode == 2)
			{
				if (updateDef->m_partialMeshTriangles == 0)
				{
					updateDef->m_partialMeshTriangles = 1;
				}
				ImGui::InputInt("Number of Triangles", &updateDef->m_partialMeshTriangles);
			}
		}
		else if (emitterDef->m_renderMode == 0)
		{
			updateDef->m_particleMeshIndex = -1;
		}
		ImGui::Checkbox("Soft Particles", (bool*)&renderDef->m_softParticles);
	}

	ImGui::End();

	if (spriteSheetPressed)
	{
		std::string filePath = Window::GetTheWindowInstance()->GetFileFromWindowsExplorer();
		if (filePath != "")
		{
			std::string fileExtension = SplitStringOnDelimiter(filePath, '.')[1];

			if (fileExtension != "png" && fileExtension != "tif")
			{
				ERROR_RECOVERABLE("Spritesheet must be a .png file");
			}
			else
			{
				AABB2 spriteSheetBounds = GetImageBoundsInSpriteAtlas(filePath);
				updateDef->m_atlasUVMins = spriteSheetBounds.m_mins;
				updateDef->m_atlasUVMaxs = spriteSheetBounds.m_maxs;
				m_currentlyEditedEffect->GetEffectDefinition()->m_particleEmitterDefs[index].GetParticleEmitterDefinition().m_spriteSheetFilePath = filePath;
			}
		}
	}
}

void ParticleSystem::UpdateColorOverLifetime(EmitterUpdateDefinitionGPU* emitterConfigGPU)
{
	if (ImGui::TreeNode("Color over lifetime"))
	{
		ImGui::SameLine();
		if (ImGui::SmallButton("Add Entry")) 
		{
			emitterConfigGPU->m_numColorsOverLifetime++;
		}

		for (unsigned int i = 0; i < emitterConfigGPU->m_numColorsOverLifetime; i++)
		{
			ImGui::PushID(i);
			if (ImGui::TreeNode("", "Entry %d", i))
			{
				ImGui::SameLine();
				if (ImGui::SmallButton("Delete Entry"))
				{
					if (emitterConfigGPU->m_numColorsOverLifetime > 1)
					{
						for (unsigned int aheadIdx = i; aheadIdx < emitterConfigGPU->m_numColorsOverLifetime - 1; aheadIdx++)
						{
							emitterConfigGPU->m_colorOverLifetime[aheadIdx] = emitterConfigGPU->m_colorOverLifetime[aheadIdx + 1];
						}
						emitterConfigGPU->m_numColorsOverLifetime--;
					}
					else
					{
						Rgba8 color = Rgba8::WHITE;
						color.GetAsFloats(emitterConfigGPU->m_colorOverLifetime[0].m_color);
						emitterConfigGPU->m_colorOverLifetime->m_time = 0.f;
					}
				}
				ImGui::InputFloat("Start Time", &emitterConfigGPU->m_colorOverLifetime[i].m_time, .5f, .1f, "%.3f");
				ImGui::ColorEdit4("Color", emitterConfigGPU->m_colorOverLifetime[i].m_color); // Edit 4 floats representing a color

				if (i == 0)
				{
					emitterConfigGPU->m_colorOverLifetime[i].m_time = 0.f;
				}
				else if (emitterConfigGPU->m_colorOverLifetime[i].m_time > 1.f)
				{
					emitterConfigGPU->m_colorOverLifetime[i].m_time = 1.f;
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
}

void ParticleSystem::UpdateChildEmitters(ParticleEmitter* currentlyEditedEmitter, int emitterDefIdx)
{
	bool isDirty = false;
	if (ImGui::TreeNode("Child Emitters"))
	{
		ImGui::SameLine();
		if (ImGui::SmallButton("Add Entry"))
		{
			isDirty = true;
			std::string emitterFileName = Window::GetTheWindowInstance()->GetFileFromWindowsExplorer();
			if (SplitStringOnDelimiter(emitterFileName, ".")[1] != "xml")
			{
				ERROR_RECOVERABLE("Pick a valid Emitter xml file");
			}
			int childDefIndex = CreateOrGetEmitterDefinitionFromFile(emitterFileName);
			ParticleEmitterDefinition const& childDef = m_loadedEmitterDefinitions[childDefIndex];
			ChildEmitter childEmitter;
			childEmitter.m_childEmitterIndex = childDefIndex;
			childEmitter.m_childEmitterFile = emitterFileName;
			childEmitter.m_childEmitterName = childDef.m_name;
			m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters.push_back(childEmitter);

		}

		for (unsigned int i = 0; i < m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters.size(); i++)
		{
			ImGui::PushID(i);
			if (ImGui::TreeNode("", m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters[i].m_childEmitterName.c_str(), i))
			{
				ImGui::SameLine();
				if (ImGui::SmallButton("Delete Entry"))
				{
					isDirty = true;
					if (m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters.size() >= 1)
					{
						m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters.erase(m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters.begin() + i);
						if (i == m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters.size())
						{
							ImGui::TreePop();
							ImGui::PopID();
							break;
						}
					}
				}
				Vec3 previousPos = m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters[i].m_localPosition;
				float localPosAsFloats[3];
				m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters[i].m_localPosition.GetFromFloats(localPosAsFloats);
				ImGui::InputFloat3("Position", localPosAsFloats, "%.2f");
				m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters[i].m_localPosition.SetFromFloats(localPosAsFloats);
				if (previousPos != m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters[i].m_localPosition)
				{
					isDirty = true;
				}

				EulerAngles previousOrient = m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters[i].m_localOrientation;
				float localOrientAsFloats[3];
				m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters[i].m_localOrientation.GetFromFloats(localOrientAsFloats);
				ImGui::InputFloat3("Orientation", localOrientAsFloats, "%.2f");
				m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters[i].m_localOrientation.SetFromFloats(localOrientAsFloats);
				if (previousOrient != m_loadedEmitterDefinitions[emitterDefIdx].m_childEmitters[i].m_localOrientation)
				{
					isDirty = true;
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
		if (isDirty)
		{
			currentlyEditedEmitter->ResetChildren(*currentlyEditedEmitter->GetEmitterDefininition());
		}
	}
}

void ParticleSystem::CustomInputFloat(char const* fieldName, FloatGraph& fieldGraph, float defaultConstant)
{
	if (fieldGraph.constantValue == -999.f)
	{
		fieldGraph.constantValue = defaultConstant;
		

		fieldGraph.minValue = defaultConstant;
		fieldGraph.maxValue = defaultConstant;

		fieldGraph.points[0].m_time = 0.f;

		fieldGraph.points[0].m_minValue = defaultConstant;
		fieldGraph.points[0].m_minValue = defaultConstant;

		fieldGraph.numPoints = 1;
	}

	if (ImGui::TreeNode(fieldName))
	{
		const char* items[] = { "Constant", "Range", "Graph" };
		ImGui::SameLine();
		ImGui::ListBox("Data Type", (int*)&fieldGraph.dataMode, items, IM_ARRAYSIZE(items), 3);
		if (fieldGraph.dataMode == 0)
		{
			ImGui::InputFloat(fieldName, &fieldGraph.constantValue);
		}
		else if (fieldGraph.dataMode == 1)
		{
			ImGui::InputFloat(Stringf("Min %s", fieldName).c_str(), &fieldGraph.minValue);
			ImGui::InputFloat(Stringf("Max %s", fieldName).c_str(), &fieldGraph.maxValue);
		}
		else
		{
			if (ImGui::SmallButton("Add Entry"))
			{
				int addedIndex = fieldGraph.numPoints;
				fieldGraph.numPoints++;
				fieldGraph.points[addedIndex].m_time = 1.f;
				fieldGraph.points[addedIndex].m_minValue = defaultConstant;
				fieldGraph.points[addedIndex].m_maxValue = defaultConstant;
			}
			for (unsigned int i = 0; i < fieldGraph.numPoints; i++)
			{
				// Use SetNextItemOpen() so set the default state of a node to be open. We could
				// also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
				//if (i == 0)
					//ImGui::SetNextItemOpen(true, ImGuiCond_Once);

				// Here we use PushID() to generate a unique base ID, and then the "" used as TreeNode id won't conflict.
				// An alternative to using 'PushID() + TreeNode("", ...)' to generate a unique ID is to use 'TreeNode((void*)(intptr_t)i, ...)',
				// aka generate a dummy pointer-sized value to be hashed. The demo below uses that technique. Both are fine.
				ImGui::PushID(i);
				if (ImGui::TreeNode("", "Entry %d", i))
				{
					ImGui::SameLine();
					if (ImGui::SmallButton("Delete Entry"))
					{
						if (fieldGraph.numPoints > 1)
						{
							for (unsigned int aheadIdx = i; aheadIdx < fieldGraph.numPoints - 1; aheadIdx++)
							{
								fieldGraph.points[aheadIdx] = fieldGraph.points[aheadIdx + 1];
							}
							fieldGraph.numPoints--;
						}
					}
					ImGui::InputFloat("Time", &fieldGraph.points[i].m_time, .5f, .1f, "%.3f");
					ImGui::InputFloat(Stringf("Min %s", fieldName).c_str(), &fieldGraph.points[i].m_minValue);
					ImGui::InputFloat(Stringf("Max %s", fieldName).c_str(), &fieldGraph.points[i].m_maxValue);


					if (ImGui::TreeNode("Easing function"))
					{
						const char* easingItems[] = { "Linear", "Smooth Start 2", "Smooth Start 3", "Smooth Start 4", "Smooth Start 5",
							"Smooth Start 6", "Smooth Stop 2", "Smooth Stop 3", "Smooth Stop 4", "Smooth Stop 5", "Smooth Stop 6",
							"Smooth Step 3", "Smooth Step 5", "Hesitate 3", "Hesitate 5", "Smooth Start 1.5", "Smooth Stop 1.5"
						};

						ImGui::ListBox("listbox", (int*)&fieldGraph.points[i].m_easingFunction, easingItems, IM_ARRAYSIZE(easingItems), 8);
						ImGui::TreePop();
					}

					if (i == 0)
					{
						fieldGraph.points[i].m_time = 0.f;
					}
					else if (fieldGraph.points[i].m_time > 1.f)
					{
						fieldGraph.points[i].m_time = 1.f;
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}
		ImGui::TreePop();
	}
}

void ParticleSystem::CustomInputFloat2(char const* fieldName, Float2Graph& fieldGraph, Vec2 defaultConstant)
{
	if (fieldGraph.constantValue[0] == -999.f && fieldGraph.constantValue[1] == -999.f)
	{
		fieldGraph.constantValue[0] = defaultConstant.x;
		fieldGraph.constantValue[1] = defaultConstant.y;

		fieldGraph.minValue[0] = defaultConstant.x;
		fieldGraph.minValue[1] = defaultConstant.y;

		fieldGraph.maxValue[0] = defaultConstant.x;
		fieldGraph.maxValue[1] = defaultConstant.y;

		fieldGraph.points[0].m_time = 0.f;

		fieldGraph.points[0].m_minValue[0] = defaultConstant.x;
		fieldGraph.points[0].m_minValue[1] = defaultConstant.y;

		fieldGraph.points[0].m_maxValue[0] = defaultConstant.x;
		fieldGraph.points[0].m_maxValue[1] = defaultConstant.y;
		fieldGraph.numPoints = 1;
	}
	std::string nodeName(fieldName);
	nodeName += " Over Lifetime";
	if (ImGui::TreeNode(nodeName.c_str()))
	{
		const char* items[] = { "Constant", "Range", "Graph" };
		ImGui::SameLine();
		ImGui::ListBox("Data Type", (int*)&fieldGraph.dataMode, items, IM_ARRAYSIZE(items), 3);
		if (fieldGraph.dataMode == 0)
		{
			ImGui::InputFloat2(fieldName, fieldGraph.constantValue);
		}
		else if (fieldGraph.dataMode == 1)
		{
			ImGui::InputFloat2(Stringf("Min %s", fieldName).c_str(), fieldGraph.minValue);
			ImGui::InputFloat2(Stringf("Max %s", fieldName).c_str(), fieldGraph.maxValue);
		}
		else
		{
			if (ImGui::SmallButton("Add Entry"))
			{
				int addedIndex = fieldGraph.numPoints;
				fieldGraph.numPoints++;
				fieldGraph.points[addedIndex].m_time = 1.f;
				fieldGraph.points[addedIndex].m_minValue[0] = defaultConstant.x;
				fieldGraph.points[addedIndex].m_minValue[1] = defaultConstant.y;
				fieldGraph.points[addedIndex].m_maxValue[0] = defaultConstant.x;
				fieldGraph.points[addedIndex].m_maxValue[1] = defaultConstant.y;
			}
			for (unsigned int i = 0; i < fieldGraph.numPoints; i++)
			{
				// Use SetNextItemOpen() so set the default state of a node to be open. We could
				// also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
				//if (i == 0)
					//ImGui::SetNextItemOpen(true, ImGuiCond_Once);

				// Here we use PushID() to generate a unique base ID, and then the "" used as TreeNode id won't conflict.
				// An alternative to using 'PushID() + TreeNode("", ...)' to generate a unique ID is to use 'TreeNode((void*)(intptr_t)i, ...)',
				// aka generate a dummy pointer-sized value to be hashed. The demo below uses that technique. Both are fine.
				ImGui::PushID(i);
				if (ImGui::TreeNode("", "Entry %d", i))
				{
					ImGui::SameLine();
					if (ImGui::SmallButton("Delete Entry"))
					{
						if (fieldGraph.numPoints > 1)
						{
							for (unsigned int aheadIdx = i; aheadIdx < fieldGraph.numPoints - 1; aheadIdx++)
							{
								fieldGraph.points[aheadIdx] = fieldGraph.points[aheadIdx + 1];
							}
							fieldGraph.numPoints--;
						}
					}
					ImGui::InputFloat("Time", &fieldGraph.points[i].m_time, .5f, .1f, "%.3f");
					ImGui::InputFloat2(Stringf("Min %s", fieldName).c_str(), fieldGraph.points[i].m_minValue, "%.2f");
					ImGui::InputFloat2(Stringf("Max %s", fieldName).c_str(), fieldGraph.points[i].m_maxValue, "%.2f");


					if (ImGui::TreeNode("Easing function"))
					{
						const char* easingItems[] = { "Linear", "Smooth Start 2", "Smooth Start 3", "Smooth Start 4", "Smooth Start 5",
							"Smooth Start 6", "Smooth Stop 2", "Smooth Stop 3", "Smooth Stop 4", "Smooth Stop 5", "Smooth Stop 6",
							"Smooth Step 3", "Smooth Step 5", "Hesitate 3", "Hesitate 5", "Smooth Start 1.5", "Smooth Stop 1.5"
						};

						ImGui::ListBox("listbox", (int*)&fieldGraph.points[i].m_easingFunction, easingItems, IM_ARRAYSIZE(easingItems), 8);
						ImGui::TreePop();
					}

					if (i == 0)
					{
						fieldGraph.points[i].m_time = 0.f;
					}
					else if (fieldGraph.points[i].m_time > 1.f)
					{
						fieldGraph.points[i].m_time = 1.f;
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}
		ImGui::TreePop();
	}
}

void ParticleSystem::CustomInputFloat3(char const* fieldName, Float3Graph& fieldGraph, Vec3 defaultConstant)
{
	if (fieldGraph.constantValue[0] == -999.f && fieldGraph.constantValue[1] == -999.f && fieldGraph.constantValue[2] == -999.f)
	{
		fieldGraph.constantValue[0] = defaultConstant.x;
		fieldGraph.constantValue[1] = defaultConstant.y;
		fieldGraph.constantValue[2] = defaultConstant.z;

		fieldGraph.minValue[0] = defaultConstant.x;
		fieldGraph.minValue[1] = defaultConstant.y;
		fieldGraph.minValue[2] = defaultConstant.z;

		fieldGraph.maxValue[0] = defaultConstant.x;
		fieldGraph.maxValue[1] = defaultConstant.y;
		fieldGraph.maxValue[2] = defaultConstant.z;

		fieldGraph.points[0].m_time = 0.f;

		fieldGraph.points[0].m_minValue[0] = defaultConstant.x;
		fieldGraph.points[0].m_minValue[1] = defaultConstant.y;
		fieldGraph.points[0].m_minValue[2] = defaultConstant.z;

		fieldGraph.points[0].m_maxValue[0] = defaultConstant.x;
		fieldGraph.points[0].m_maxValue[1] = defaultConstant.y;
		fieldGraph.points[0].m_maxValue[2] = defaultConstant.z;
		fieldGraph.numPoints = 1;
	}

	if (ImGui::TreeNode(fieldName))
	{
		const char* items[] = { "Constant", "Range", "Graph" };
		ImGui::SameLine();
		ImGui::ListBox("Data Type", (int*)&fieldGraph.dataMode, items, IM_ARRAYSIZE(items), 3);
		if (fieldGraph.dataMode == 0)
		{
			ImGui::InputFloat3(fieldName, fieldGraph.constantValue);
		}
		else if (fieldGraph.dataMode == 1)
		{
			ImGui::InputFloat3(Stringf("Min %s", fieldName).c_str(), fieldGraph.minValue);
			ImGui::InputFloat3(Stringf("Max %s", fieldName).c_str(), fieldGraph.maxValue);
		}
		else
		{
			if (ImGui::SmallButton("Add Entry"))
			{
				int addedIndex = fieldGraph.numPoints;
				fieldGraph.numPoints++;
				fieldGraph.points[addedIndex].m_time = 1.f;
				fieldGraph.points[addedIndex].m_minValue[0] = defaultConstant.x;
				fieldGraph.points[addedIndex].m_minValue[1] = defaultConstant.y;
				fieldGraph.points[addedIndex].m_minValue[2] = defaultConstant.z;

				fieldGraph.points[addedIndex].m_maxValue[0] = defaultConstant.x;
				fieldGraph.points[addedIndex].m_maxValue[1] = defaultConstant.y;
				fieldGraph.points[addedIndex].m_maxValue[2] = defaultConstant.z;
			}
			for (unsigned int i = 0; i < fieldGraph.numPoints; i++)
			{
				// Use SetNextItemOpen() so set the default state of a node to be open. We could
				// also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
				//if (i == 0)
					//ImGui::SetNextItemOpen(true, ImGuiCond_Once);

				// Here we use PushID() to generate a unique base ID, and then the "" used as TreeNode id won't conflict.
				// An alternative to using 'PushID() + TreeNode("", ...)' to generate a unique ID is to use 'TreeNode((void*)(intptr_t)i, ...)',
				// aka generate a dummy pointer-sized value to be hashed. The demo below uses that technique. Both are fine.
				ImGui::PushID(i);
				if (ImGui::TreeNode("", "Entry %d", i))
				{
					ImGui::SameLine();
					if (ImGui::SmallButton("Delete Entry"))
					{
						if (fieldGraph.numPoints > 1)
						{
							for (unsigned int aheadIdx = i; aheadIdx < fieldGraph.numPoints - 1; aheadIdx++)
							{
								fieldGraph.points[aheadIdx] = fieldGraph.points[aheadIdx + 1];
							}
							fieldGraph.numPoints--;
						}
					}
					ImGui::InputFloat("Time", &fieldGraph.points[i].m_time, .5f, .1f, "%.3f");
					ImGui::InputFloat3(Stringf("Min %s", fieldName).c_str(), fieldGraph.points[i].m_minValue, "%.2f");
					ImGui::InputFloat3(Stringf("Max %s", fieldName).c_str(), fieldGraph.points[i].m_maxValue, "%.2f");


					if (ImGui::TreeNode("Easing function"))
					{
						const char* easingItems[] = { "Linear", "Smooth Start 2", "Smooth Start 3", "Smooth Start 4", "Smooth Start 5",
							"Smooth Start 6", "Smooth Stop 2", "Smooth Stop 3", "Smooth Stop 4", "Smooth Stop 5", "Smooth Stop 6",
							"Smooth Step 3", "Smooth Step 5", "Hesitate 3", "Hesitate 5", "Smooth Start 1.5", "Smooth Stop 1.5"
						};

						ImGui::ListBox("listbox", (int*)&fieldGraph.points[i].m_easingFunction, easingItems, IM_ARRAYSIZE(easingItems), 8);
						ImGui::TreePop();
					}

					if (i == 0)
					{
						fieldGraph.points[i].m_time = 0.f;
					}
					else if (fieldGraph.points[i].m_time > 1.f)
					{
						fieldGraph.points[i].m_time = 1.f;
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}
		ImGui::TreePop();
	}
}



void ParticleSystem::UpdateLiveEmitters(float deltaSeconds)
{
	m_particleConstants.m_emittedParticles = 0;
	for (int i = 0; i < (int)m_liveEmitterInstancesGPU.size(); i++)
	{

		EmitterUpdateDefinitionGPU& currentUpdateDef = m_updateDefinitions[m_liveEmitterInstancesGPU[i].m_definitionIndex];
		ParticleEmitterDefinition const* currentEmitterDef = &m_loadedEmitterDefinitions[m_liveEmitterInstancesCPU[i].m_loadedEmitterDefIdx];
		if (m_liveEmitterInstancesCPU[i].m_isActive == 1)
		{
			if (m_liveEmitterInstancesCPU[i].m_delayTimeLeft > 0.f)
			{
				m_liveEmitterInstancesCPU[i].m_delayTimeLeft -= deltaSeconds;
				m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame = 0;
				m_liveEmitterInstancesGPU[i].m_emissionStartIdx = m_particleConstants.m_emittedParticles;
				continue;
			}

			//if an emitter is not active it means there are no longer any live particles under its control
			if (m_liveEmitterInstancesCPU[i].m_activeTimeLeft != -1.f && m_liveEmitterInstancesCPU[i].m_activeTimeLeft > 0.f)
			{
				m_liveEmitterInstancesCPU[i].m_activeTimeLeft -= deltaSeconds;
				if (m_liveEmitterInstancesCPU[i].m_activeTimeLeft <= 0.f)
				{
					m_liveEmitterInstancesCPU[i].m_isActive = 0;

					//this emitter was spawned from another emitter and must clean up the ParticleEmitter* 
					if (m_liveEmitterInstancesCPU[i].m_isSubEmitter)
					{
						delete m_liveEmitterInstancesCPU[i].m_emitter;
						m_liveEmitterInstancesCPU[i].m_emitter = nullptr;

						m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame = 0;
						m_liveEmitterInstancesGPU[i].m_emissionStartIdx = m_particleConstants.m_emittedParticles;
						continue;
					}
				}
			}

			//emitters should stop once emit time left reaches 0
			bool shouldEmit = true;
			if (m_liveEmitterInstancesCPU[i].m_emitTimeLeft != -1.f)
			{
				if (m_liveEmitterInstancesCPU[i].m_emitTimeLeft > 0.f)
				{
					m_liveEmitterInstancesCPU[i].m_emitTimeLeft -= deltaSeconds;
					float normalizedLiveTime = (m_liveEmitterInstancesCPU[i].m_emitterLifetime - m_liveEmitterInstancesCPU[i].m_emitTimeLeft) / m_liveEmitterInstancesCPU[i].m_emitterLifetime;
					Vec3 velocity = GetValueInFloat3Graph(m_emitterVelocityGraphs[m_liveEmitterInstancesGPU[i].m_definitionIndex], normalizedLiveTime, m_liveEmitterInstancesCPU[i].m_seed, true);
					if (velocity.x != -999.f && velocity.GetLengthSquared() > .001f)
					{
						m_liveEmitterInstancesCPU[i].m_emitter->Translate(velocity * deltaSeconds);
					}
				}
				if (m_liveEmitterInstancesCPU[i].m_emitTimeLeft <= 0.f)
				{
 					shouldEmit = false;
				}
			}
			m_liveEmitterInstancesGPU[i].m_emitterVelocity = (m_liveEmitterInstancesCPU[i].m_emitter->GetWorldPosition() - m_liveEmitterInstancesCPU[i].m_lastFramePos) / deltaSeconds;
			m_liveEmitterInstancesCPU[i].m_lastFramePos = m_liveEmitterInstancesCPU[i].m_emitter->GetWorldPosition();
			
			if (currentUpdateDef.m_emissionMode == (unsigned int)EmissionMode::CONTINUOUS)
			{
				if (shouldEmit)
				{
					float particlesToEmitAsFloat = deltaSeconds * currentUpdateDef.m_emissionRate;
					m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame = RoundDownToInt(particlesToEmitAsFloat);
					m_liveEmitterInstancesCPU[i].m_owedParticles += particlesToEmitAsFloat - (float)m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame;
					while (m_liveEmitterInstancesCPU[i].m_owedParticles >= 1.f)
					{
						m_liveEmitterInstancesCPU[i].m_owedParticles--;
						m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame++;
					}
				}
				else
				{
					m_liveEmitterInstancesCPU[i].m_owedParticles = 0.f;
					m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame = 0;
				}
			}
			else if (currentUpdateDef.m_emissionMode == (unsigned int)EmissionMode::BURST)
			{
				if (m_liveEmitterInstancesCPU[i].m_currentRepeatTime <= 0.f && m_liveEmitterInstancesCPU[i].m_currentRepeatTime != -1.f && shouldEmit)
				{
					//new burst up
					if (m_liveEmitterInstancesCPU[i].m_currentBurstInterval <= 0.f)
					{
						m_liveEmitterInstancesCPU[i].m_currentBurst++;
						m_liveEmitterInstancesCPU[i].m_currentBurstInterval = currentEmitterDef->m_burstInterval;
						m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame = (int)currentUpdateDef.m_emissionRate;

						//bursts done
						if (m_liveEmitterInstancesCPU[i].m_currentBurst == currentEmitterDef->m_numBursts)
						{
							m_liveEmitterInstancesCPU[i].m_currentBurst = 0;
							if (currentUpdateDef.m_repeat == 1)
							{
								m_liveEmitterInstancesCPU[i].m_currentRepeatTime = currentUpdateDef.m_repeatTime;
							}
							else
							{
								m_liveEmitterInstancesCPU[i].m_currentRepeatTime = -1.f;
							}
						}
					}
					else
					{
						m_liveEmitterInstancesCPU[i].m_currentBurstInterval -= deltaSeconds;
						m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame = 0;
					}
				}
				else
				{
					m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame = 0;
					if (m_liveEmitterInstancesCPU[i].m_currentRepeatTime != -1.f)
					{
						m_liveEmitterInstancesCPU[i].m_currentRepeatTime -= deltaSeconds;
					}
				}
			}

			m_liveEmitterInstancesGPU[i].m_emissionStartIdx = m_particleConstants.m_emittedParticles;
			m_liveEmitterInstancesGPU[i].m_totalParticlesEmitted += m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame;

			if (m_liveEmitterInstancesCPU[i].m_spawnSubEmitters)
			{
				if (m_liveEmitterInstancesCPU[i].m_subEmitterDefIdx != -1)
				{
					for (int c = 0; c < (int)m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame; c++)
					{
						ParticleEmitterInitializer initializer;
						initializer.m_emitterIndex = m_liveEmitterInstancesCPU[i].m_subEmitterDefIdx;

						initializer.m_position = m_liveEmitterInstancesCPU[i].m_emitter->GetWorldPosition();
						FloatRange xBoxRange = FloatRange(-.5f * currentUpdateDef.m_boxDimensions[0], .5f * currentUpdateDef.m_boxDimensions[0]);
						FloatRange yBoxRange = FloatRange(-.5f * currentUpdateDef.m_boxDimensions[1], .5f * currentUpdateDef.m_boxDimensions[1]);
						FloatRange zBoxRange = FloatRange(-.5f * currentUpdateDef.m_boxDimensions[2], .5f * currentUpdateDef.m_boxDimensions[2]);
						initializer.m_position.x += m_rng->RollRandomFloatInRange(xBoxRange);
						initializer.m_position.y += m_rng->RollRandomFloatInRange(yBoxRange);
						initializer.m_position.z += m_rng->RollRandomFloatInRange(zBoxRange);

						EulerAngles randomRotation;
						randomRotation.m_yaw = m_rng->RollRandomFloatInRange(currentEmitterDef->m_minSubEmitterOrientation[0], currentEmitterDef->m_maxSubEmitterOrientation[0]);
						randomRotation.m_pitch = m_rng->RollRandomFloatInRange(currentEmitterDef->m_minSubEmitterOrientation[1], currentEmitterDef->m_maxSubEmitterOrientation[1]);
						randomRotation.m_roll = m_rng->RollRandomFloatInRange(currentEmitterDef->m_minSubEmitterOrientation[2], currentEmitterDef->m_maxSubEmitterOrientation[2]);
						initializer.m_scale = m_liveEmitterInstancesGPU[i].m_localToWorldMatrix.GetUniformScale();
						Mat44 orientationMatrix;
						orientationMatrix.Append(randomRotation.GetAsMatrix_IFwd_JLeft_KUp());
						orientationMatrix.Append(m_liveEmitterInstancesCPU[i].m_emitter->GetWorldOrientationMatrix());
						initializer.m_orientationDegrees = orientationMatrix.GetEulerAngles();

						ParticleEmitter* addedEmitter = new ParticleEmitter(initializer, nullptr);
						EmitterInstanceCPU* addedInstanceCPU = addedEmitter->GetEmitterInstanceCPU();
						
						/*
						if (addedInstanceCPU->m_emitTimeLeft != -1.f || )
						{
							addedInstanceCPU->m_activeTimeLeft = addedEmitter->GetEmitterUpdateDef()->m_lifetime[1] + addedInstanceCPU->m_emitTimeLeft;

						}
						*/
						addedEmitter->Play(addedInstanceCPU->m_emitTimeLeft);
						addedInstanceCPU->m_currentRepeatTime = 0.f;
						addedInstanceCPU->m_isSubEmitter = true;
					}
				}
				m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame = 0;
			}
			else
			{
				m_particleConstants.m_emittedParticles += m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame;
			}
		}

		// is active false
		else
		{
			m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame = 0;
			m_liveEmitterInstancesGPU[i].m_emissionStartIdx = m_particleConstants.m_emittedParticles;
		}
	}

	//make sure the emission start index is correct because of sub emitters
	int startIndex = 0;
	for (int i = 0; i < (int)m_liveEmitterInstancesGPU.size(); i++)
	{
		m_liveEmitterInstancesGPU[i].m_emissionStartIdx = startIndex;
		startIndex += m_liveEmitterInstancesGPU[i].m_particlesToEmitThisFrame;
	}
}

void ParticleSystem::UpdateEmitterSRVData()
{
	
	if (m_particleConstants.m_emittedParticles == 0 && m_aliveCountFromlastFrame == 0)
	{
		return;
	}
	
	//update emitter instances
	if (m_emitterInstanceBuffer != nullptr)
	{
		delete m_emitterInstanceBuffer;
		m_emitterInstanceBuffer = nullptr;
	}
	if (m_emitterInstanceSRV != nullptr)
	{
		delete m_emitterInstanceSRV;
		m_emitterInstanceSRV = nullptr;
	}
	m_emitterInstanceBuffer = g_theRenderer->CreateStructuredBuffer((unsigned int)sizeof(EmitterInstanceGPU), 
		(unsigned int)m_liveEmitterInstancesGPU.size(), m_liveEmitterInstancesGPU.data());
	m_emitterInstanceSRV = g_theRenderer->CreateSRV(m_emitterInstanceBuffer);

	//update update defs
	if (m_prevUpdateDefEntries != (unsigned int)m_updateDefinitions.size())
	{
		delete m_updateDefBuffer;
		delete m_updateDefSRV;

		m_updateDefBuffer = g_theRenderer->CreateStructuredBuffer((unsigned int)sizeof(EmitterUpdateDefinitionGPU), 
			(unsigned int)m_updateDefinitions.size(), m_updateDefinitions.data(), true);
		m_updateDefSRV = g_theRenderer->CreateSRV(m_updateDefBuffer);

		for (int i = 0; i < (int)m_updateDefinitions.size(); i++)
		{
			m_updateDefinitions[i].isDirty = 0;
		}
		m_prevUpdateDefEntries = (unsigned int)m_updateDefinitions.size();
	}
	else
	{
		std::vector<int> entriesToReplace;
		for (int i = 0; i < (int)m_updateDefinitions.size(); i++)
		{
			if (m_updateDefinitions[i].isDirty == 1)
			{
				entriesToReplace.push_back(i);
				m_updateDefinitions[i].isDirty = 0;
			}
		}
		if (entriesToReplace.size() > 0)
		{
			D3D11_MAPPED_SUBRESOURCE pResource;
			g_theRenderer->MapBufferForWrite(m_updateDefBuffer, &pResource);
			EmitterUpdateDefinitionGPU* pUdateDefs = reinterpret_cast<EmitterUpdateDefinitionGPU*>(pResource.pData);
			//replace dirty entries
			for (int i = 0; i < (int)entriesToReplace.size(); i++)
			{
				int idxToReplace = entriesToReplace[i];
				//size_t offset = idxToReplace * sizeof(EmitterUpdateDefinitionGPU);
				size_t copySize = sizeof(EmitterUpdateDefinitionGPU);
				memcpy((void*)&pUdateDefs[idxToReplace], (void*)&(m_updateDefinitions[idxToReplace]), copySize);
			}
			g_theRenderer->UnmapBuffer(m_updateDefBuffer);
		}
	}

	//update render defs
	if (m_prevRenderDefEntries != (unsigned int)m_renderDefinitions.size())
	{
		delete m_renderDefBuffer;
		delete m_renderDefSRV;

		m_renderDefBuffer = g_theRenderer->CreateStructuredBuffer((unsigned int)sizeof(EmitterRenderDefinitionGPU),
			(unsigned int)m_renderDefinitions.size(), m_renderDefinitions.data(), true);
		m_renderDefSRV = g_theRenderer->CreateSRV(m_renderDefBuffer);

		for (int i = 0; i < (int)m_renderDefinitions.size(); i++)
		{
			m_renderDefinitions[i].isDirty = 0;
		}
		m_prevRenderDefEntries = (unsigned int)m_renderDefinitions.size();
	}
	else
	{
		std::vector<int> entriesToReplace;
		for (int i = 0; i < (int)m_renderDefinitions.size(); i++)
		{
			if (m_renderDefinitions[i].isDirty == 1)
			{
				entriesToReplace.push_back(i);
				m_renderDefinitions[i].isDirty = 0;
			}
		}
		if (entriesToReplace.size() > 0)
		{
			D3D11_MAPPED_SUBRESOURCE pResource;
			g_theRenderer->MapBufferForWrite(m_renderDefBuffer, &pResource);
			EmitterRenderDefinitionGPU* prenderDefs = reinterpret_cast<EmitterRenderDefinitionGPU*>(pResource.pData);
			//replace dirty entries
			for (int i = 0; i < (int)entriesToReplace.size(); i++)
			{
				int idxToReplace = entriesToReplace[i];
				memcpy((void*)&prenderDefs[idxToReplace], (void*)&m_renderDefinitions[idxToReplace], sizeof(EmitterRenderDefinitionGPU));
			}
			g_theRenderer->UnmapBuffer(m_renderDefBuffer);
		}
	}

	//update floatgraphs
	if (m_prevFloatGraphEntries != (unsigned int)m_floatGraphs.size())
	{
		delete m_floatGraphsBuffer;
		delete m_floatGraphsSRV;

		m_floatGraphsBuffer = g_theRenderer->CreateStructuredBuffer((unsigned int)sizeof(FloatGraph),
			(unsigned int)m_floatGraphs.size(), m_floatGraphs.data(), true);
		m_floatGraphsSRV = g_theRenderer->CreateSRV(m_floatGraphsBuffer);

		for (int i = 0; i < (int)m_floatGraphs.size(); i++)
		{
			m_floatGraphs[i].isDirty = 0;
		}
		m_prevFloatGraphEntries = (unsigned int)m_floatGraphs.size();
	}
	else
	{
		std::vector<int> entriesToReplace;
		for (int i = 0; i < (int)m_floatGraphs.size(); i++)
		{
			if (m_floatGraphs[i].isDirty == 1)
			{
				entriesToReplace.push_back(i);
				m_floatGraphs[i].isDirty = 0;
			}
		}
		if (entriesToReplace.size() > 0)
		{
			D3D11_MAPPED_SUBRESOURCE pResource;
			g_theRenderer->MapBufferForWrite(m_floatGraphsBuffer, &pResource);
			FloatGraph* pfloatGraphs = reinterpret_cast<FloatGraph*>(pResource.pData);
			//replace dirty entries
			for (int i = 0; i < (int)entriesToReplace.size(); i++)
			{
				int idxToReplace = entriesToReplace[i];
				memcpy((void*)&pfloatGraphs[idxToReplace], (void*)& m_floatGraphs[idxToReplace], sizeof(FloatGraph));
			}
			g_theRenderer->UnmapBuffer(m_floatGraphsBuffer);
		}
	}

	//update float2graphs
	if (m_prevFloat2GraphEntries != (unsigned int)m_float2Graphs.size())
	{
		delete m_float2GraphsBuffer;
		delete m_float2GraphsSRV;

		m_float2GraphsBuffer = g_theRenderer->CreateStructuredBuffer((unsigned int)sizeof(Float2Graph),
			(unsigned int)m_float2Graphs.size(), m_float2Graphs.data(), true);
		m_float2GraphsSRV = g_theRenderer->CreateSRV(m_float2GraphsBuffer);

		for (int i = 0; i < (int)m_float2Graphs.size(); i++)
		{
			m_float2Graphs[i].isDirty = 0;
		}
		m_prevFloat2GraphEntries = (unsigned int)m_float2Graphs.size();
	}
	else
	{
		std::vector<int> entriesToReplace;
		for (int i = 0; i < (int)m_float2Graphs.size(); i++)
		{
			if (m_float2Graphs[i].isDirty == 1)
			{
				entriesToReplace.push_back(i);
				m_float2Graphs[i].isDirty = false;
			}
		}
		if (entriesToReplace.size() > 0)
		{
			D3D11_MAPPED_SUBRESOURCE pResource;
			g_theRenderer->MapBufferForWrite(m_float2GraphsBuffer, &pResource);
			Float2Graph* pfloat2Graphs = reinterpret_cast<Float2Graph*>(pResource.pData);
			//replace dirty entries
			for (int i = 0; i < (int)entriesToReplace.size(); i++)
			{
				int idxToReplace = entriesToReplace[i];
				memcpy((void*)&pfloat2Graphs[idxToReplace], (void*)&m_float2Graphs[idxToReplace], sizeof(Float2Graph));
			}
			g_theRenderer->UnmapBuffer(m_float2GraphsBuffer);
		}
	}

	//update float3graphs
	if (m_prevFloat3GraphEntries != (unsigned int)m_float3Graphs.size())
	{
		delete m_float3GraphsBuffer;
		delete m_float3GraphsSRV;

		m_float3GraphsBuffer = g_theRenderer->CreateStructuredBuffer((unsigned int)sizeof(Float3Graph),
			(unsigned int)m_float3Graphs.size(), m_float3Graphs.data(), true);
		m_float3GraphsSRV = g_theRenderer->CreateSRV(m_float3GraphsBuffer);

		for (int i = 0; i < (int)m_float3Graphs.size(); i++)
		{
			m_float3Graphs[i].isDirty = 0;
		}
		m_prevFloat3GraphEntries = (unsigned int)m_float3Graphs.size();
	}
	else
	{
		std::vector<int> entriesToReplace;
		for (int i = 0; i < (int)m_float3Graphs.size(); i++)
		{
			if (m_float3Graphs[i].isDirty == 1)
			{
				entriesToReplace.push_back(i);
				m_float3Graphs[i].isDirty = false;
			}
		}
		if (entriesToReplace.size() > 0)
		{
			D3D11_MAPPED_SUBRESOURCE pResource;
			g_theRenderer->MapBufferForWrite(m_float3GraphsBuffer, &pResource);
			Float3Graph* pfloat3Graphs = reinterpret_cast<Float3Graph*>(pResource.pData);
			//replace dirty entries
			for (int i = 0; i < (int)entriesToReplace.size(); i++)
			{
				int idxToReplace = entriesToReplace[i];
				memcpy((void*)&pfloat3Graphs[idxToReplace], (void*)&m_float3Graphs[idxToReplace], sizeof(Float3Graph));
			}
			g_theRenderer->UnmapBuffer(m_float3GraphsBuffer);
		}
	}	
}

void ParticleSystem::CreatePhysicsObjectsSRV()
{
	if (m_livePhysicsObjects.size() != 0)
	{
		if (m_physicsObjectsBuffer != nullptr)
		{
			delete m_physicsObjectsBuffer;
		}
		if (m_physicsObjectsSRV != nullptr)
		{
			delete m_physicsObjectsSRV;
		}

		m_physicsObjectsBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(ParticlePhysicsObjectGPU),
			(unsigned int)m_livePhysicsObjects.size(), m_livePhysicsObjects.data());
		m_physicsObjectsSRV = g_theRenderer->CreateSRV(m_physicsObjectsBuffer);
	}

	if (m_livePhysicsAABB3s.size() != 0)
	{
		if (m_physicsAABB3Buffer != nullptr)
		{
			delete m_physicsAABB3Buffer;
		}
		if (m_physicsAABB3SRV != nullptr)
		{
			delete m_physicsAABB3SRV;
		}
		m_physicsAABB3Buffer = g_theRenderer->CreateStructuredBuffer(sizeof(ParticlePhysicsAABB3GPU),
			(unsigned int)m_livePhysicsAABB3s.size(), m_livePhysicsAABB3s.data());
		m_physicsAABB3SRV = g_theRenderer->CreateSRV(m_physicsAABB3Buffer);
	}
}

void ParticleSystem::BeginFrame()
{

}

void ParticleSystem::EndFrame()
{
	//swap alive lists
	SRV* tempAliveList1SRV = m_particleAliveList1SRV;
	UAV* tempAliveList1UAV = m_particleAliveList1UAV;
	StructuredBuffer* tempAliveList1Buffer = m_particleAliveList1Buffer;


	m_particleAliveList1Buffer = m_particleAliveList2Buffer;
	m_particleAliveList1UAV = m_particleAliveList2UAV;
	m_particleAliveList1SRV = m_particleAliveList2SRV;

	m_particleAliveList2Buffer = tempAliveList1Buffer;
	m_particleAliveList2UAV = tempAliveList1UAV;
	m_particleAliveList2SRV = tempAliveList1SRV;

	ComputeShader* resetCounterShader = g_theRenderer->CreateOrGetComputeShaderFromFile(GetShaderFilePath("ResetCounter.hlsl").c_str());
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
}

void ParticleSystem::EmitParticles(float deltaSeconds)
{
	m_particleConstants.m_timeElapsed += deltaSeconds;
	m_particleConstants.m_deltaSeconds = deltaSeconds;
	m_particleConstants.m_frameCount = m_frameCount;
	m_particleConstants.m_frameCount++;
	m_particleConstants.m_numEmitterConfigs = (unsigned int)m_liveEmitterInstancesGPU.size();
	m_particleConstants.m_numPhysicsObjects = (unsigned int)m_livePhysicsObjects.size();
	m_particleConstants.m_numPhysicsAABB3s = (unsigned int)m_livePhysicsAABB3s.size();
	m_particleConstants.m_windowDimensionsX = Window::GetTheWindowInstance()->GetClientDimensions().x;
	m_particleConstants.m_windowDimensionsY = Window::GetTheWindowInstance()->GetClientDimensions().y;
	m_particleConstants.m_nearPlane = m_config.m_playerCamera->GetPerspectiveNear();
	m_particleConstants.m_farPlane = m_config.m_playerCamera->GetPerspectiveFar();
	m_particleConstants.m_inverseViewProjMatrix = m_config.m_playerCamera->GetInverseViewProjectionMatrix();
	m_particleConstants.m_cameraIBasis = m_config.m_playerCamera->GetCameraForward();

	g_theRenderer->SetCameraConstants(*m_config.m_playerCamera);
	if (m_config.m_playerCamera != nullptr)
	{
		m_particleConstants.m_playerPosition = m_config.m_playerCamera->GetPosition();
	}
	g_theRenderer->CopyCPUToGPU((void*)&m_particleConstants, m_particleConstantsCBO);
	g_theRenderer->BindConstantBuffer(k_particleConstantsSlot, m_particleConstantsCBO);
	if (m_particleConstants.m_emittedParticles == 0)
	{
		return;
	}

	ComputeShader* emitShader = g_theRenderer->CreateOrGetComputeShaderFromFile(GetShaderFilePath("ParticleEmit.hlsl").c_str());
	if (!emitShader)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get emit shader");
		return;
	}

	std::vector<SRV*> srvs;
	srvs.push_back(m_emitterInstanceSRV);
	srvs.push_back(m_updateDefSRV);
	if (m_meshParticleEntryStructuredBuffer != nullptr)
	{
		srvs.push_back(m_meshParticleVertsSRV);
		srvs.push_back(m_meshParticleEntrySRV);
	}
	std::vector<UAV*> uavs;
	uavs.push_back(m_particlesUAV);
	uavs.push_back(m_particleAliveList1UAV);
	uavs.push_back(m_particleAliveList2UAV);
	uavs.push_back(m_deadListUAV);
	uavs.push_back(m_counterUAV);
	uavs.push_back(m_particleDrawListUAV);

	unsigned int threadGroupX = (unsigned int)ceil((double)m_particleConstants.m_emittedParticles / 64.f);
	if (threadGroupX > D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION)
	{
		threadGroupX = D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
		//ERROR_RECOVERABLE("Thread group exceeded max size for emitting particles");
	}

	g_theRenderer->RunComputeShader(emitShader, srvs, uavs, threadGroupX, 1, 1);
}

void ParticleSystem::UpdateParticles(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	int particlesToUpdate = m_particleConstants.m_emittedParticles + m_aliveCountFromlastFrame;
	if (particlesToUpdate == 0)
	{
		return;
	}
	ComputeShader* simulateShader = g_theRenderer->CreateOrGetComputeShaderFromFile(GetShaderFilePath("ParticleSimulate.hlsl").c_str());
	if (!simulateShader)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get simulate shader");
		return;
	}
	std::vector<SRV*> srvs;
	srvs.push_back(m_particleAliveList1SRV);
	srvs.push_back(m_emitterInstanceSRV);
	srvs.push_back(m_updateDefSRV);
	srvs.push_back(m_physicsObjectsSRV);
	srvs.push_back(m_floatGraphsSRV);
	srvs.push_back(m_float2GraphsSRV);
	srvs.push_back(m_float3GraphsSRV);
	srvs.push_back(m_physicsAABB3SRV);

	std::vector<UAV*> uavs;
	uavs.push_back(m_particlesUAV);
	uavs.push_back(m_particleAliveList2UAV);
	uavs.push_back(m_deadListUAV);
	uavs.push_back(m_counterUAV);
	uavs.push_back(m_particleDistanceUAV);
	uavs.push_back(m_particleDrawListUAV);
	uavs.push_back(m_meshParticleDistanceUAV);
	uavs.push_back(m_meshParticleDrawListUAV);


	unsigned int threadGroupX = (unsigned int)ceil((double)particlesToUpdate / 64.f);
	if (threadGroupX > D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION)
	{
		threadGroupX = D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
	}
	
	g_theRenderer->SetComputeShaderSRVsAndUAVs(srvs, uavs);
	g_theRenderer->BindPrevFrameDepthTextureToTRegister(8);
	g_theRenderer->RunComputeShader(simulateShader, threadGroupX, 1, 1);
	
	//g_theRenderer->RunComputeShader(simulateShader, srvs, uavs, threadGroupX, 1, 1);

	CounterBuffer* counterBuffer = (CounterBuffer*)g_theRenderer->ReadFromBuffer(m_counterBuffer);
	g_theRenderer->ClearSRVSAndUAVS(9, 8);

	//this is how many particles need to be drawn
	m_aliveCountFromlastFrame = counterBuffer->aliveCountAfterSim;
	m_deadCountFromLastFrame = counterBuffer->deadCount;
	m_drawCount = counterBuffer->drawCount;
	m_meshParticleDrawCount = counterBuffer->drawMeshCount;
	m_culledCountLastFrame = m_aliveCountFromlastFrame - (m_drawCount + m_meshParticleDrawCount);

	if (counterBuffer->drawCount > 1)
	{
		SortParticlesGPU(counterBuffer->drawCount, m_particleDistanceSRV, m_particleDrawListUAV);
	}
	if (m_meshParticleDrawCount > 0)
	{
		//everything done in here should be pretty slow...

		//fist create the buffer and views based on drawMeshCount
		if (m_meshOffsetListBuffer != nullptr)
		{
			delete m_meshOffsetListBuffer;
			delete m_meshOffsetListSRV;
			delete m_meshOffsetListUAV;
		}
		m_meshOffsetListBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(unsigned int), m_meshParticleDrawCount, nullptr, true, true);
		m_meshOffsetListSRV = g_theRenderer->CreateSRV(m_meshOffsetListBuffer);
		m_meshOffsetListUAV = g_theRenderer->CreateUAV(m_meshOffsetListBuffer);

		SortParticlesGPU(m_meshParticleDrawCount, m_meshParticleDistanceSRV, m_meshParticleDrawListUAV);
		srvs.clear();
		uavs.clear();
		srvs.push_back(m_particlesSRV);
		srvs.push_back(m_emitterInstanceSRV);
		srvs.push_back(m_updateDefSRV);
		srvs.push_back(m_meshParticleDrawListSRV);
		srvs.push_back(m_counterSRV);
		srvs.push_back(m_meshParticleEntrySRV);
		uavs.push_back(m_meshOffsetListUAV);

		unsigned int threadGroupXPopulateVertexCounts = (unsigned int)ceil((double)m_meshParticleDrawCount / 64.f);
		g_theRenderer->RunComputeShader(g_theRenderer->CreateOrGetComputeShaderFromFile(GetShaderFilePath("PopulateVertexCounts.hlsl").c_str()), srvs, uavs, threadGroupXPopulateVertexCounts, 1, 1);
		unsigned int* meshOffsetList = (unsigned int*)g_theRenderer->MapBufferForReadWrite(m_meshOffsetListBuffer);
		m_totalMeshVertCount = 0;
		for (unsigned int i = 0; i < m_meshParticleDrawCount; i++)
		{
			unsigned int vertCount = meshOffsetList[i];
			meshOffsetList[i] = m_totalMeshVertCount;
			m_totalMeshVertCount += vertCount;
		}
		g_theRenderer->UnmapBuffer(m_meshOffsetListBuffer);
	}
	//g_theRenderer->SetUpRenderTargets();
}

void ParticleSystem::SortParticlesGPU(int maxCount, SRV* comparisonList, UAV* indexList)
{
	//nothing to sort
	if (maxCount < 2)
	{
		return;
	}
	SortConstants sort;
	sort.numElements = maxCount;
	g_theRenderer->CopyCPUToGPU((void*)&sort, m_sortConstantsCBO);
	g_theRenderer->BindConstantBuffer(k_sortConstantsSlot, m_sortConstantsCBO);

	//initialize sorting arguments
	ComputeShader* kickoffSortShader = g_theRenderer->CreateOrGetComputeShaderFromFile(GetShaderFilePath("KickoffSort.hlsl").c_str());
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
	uavs.push_back(indirectArgsUAV);

	g_theRenderer->RunComputeShader(kickoffSortShader, srvs, uavs, 1, 1, 1);

	srvs.clear();
	uavs.clear();
	srvs.push_back(comparisonList);
	uavs.push_back(indexList);

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
	ComputeShader* sortShader = g_theRenderer->CreateOrGetComputeShaderFromFile(GetShaderFilePath("AMD_SortCS.hlsl").c_str());
	if (!sortShader)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get sort shader");
		return;
	}
	g_theRenderer->RunComputeShader(sortShader, srvs, uavs, sortIndirectArgs);

	ComputeShader* sortStepShader = g_theRenderer->CreateOrGetComputeShaderFromFile(GetShaderFilePath("AMD_SortStepCS.hlsl").c_str());
	if (!sortStepShader)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get sort step shader");
		return;
	}

	ComputeShader* sortInnerShader = g_theRenderer->CreateOrGetComputeShaderFromFile(GetShaderFilePath("AMD_SortInnerCS.hlsl").c_str());
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
			sort.numElements = maxCount;
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

void ParticleSystem::Render() const
{
	if (m_meshParticleDrawCount > 0)
	{
		ComputeShader* addVertsShader = g_theRenderer->CreateOrGetComputeShaderFromFile(GetShaderFilePath("PopulateVBOMeshes.hlsl").c_str());
		if (!addVertsShader)
		{
			g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get populate vbo shader");
			return;
		}

		std::vector<SRV*> srvs;
		std::vector<UAV*> uavs;
		uavs.push_back(m_meshVBOUAV);

		srvs.push_back(m_emitterInstanceSRV);
		srvs.push_back(m_updateDefSRV);
		srvs.push_back(m_particlesSRV);
		srvs.push_back(m_meshParticleDrawListSRV);
		srvs.push_back(m_counterSRV);
		srvs.push_back(m_meshOffsetListSRV);
		srvs.push_back(m_meshParticleEntrySRV);
		srvs.push_back(m_meshParticleVertsSRV);
		srvs.push_back(m_floatGraphsSRV);
		srvs.push_back(m_float2GraphsSRV);
		srvs.push_back(m_float3GraphsSRV);

		unsigned int threadGroupX = (unsigned int)ceil((double)m_meshParticleDrawCount / 64.f);
		if (threadGroupX > D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION)
		{
			threadGroupX = D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
			//ERROR_RECOVERABLE("Thread group exceeded max size for updating particles");
		}
		g_theRenderer->RunComputeShader(addVertsShader, srvs, uavs, threadGroupX, 1, 1);

		std::vector<SRV*> pssrvs;
		pssrvs.push_back(m_renderDefSRV);
		pssrvs.push_back(m_floatGraphsSRV);
		pssrvs.push_back(m_float2GraphsSRV);
		pssrvs.push_back(m_float3GraphsSRV);
		g_theRenderer->SetPSSRVs(pssrvs, 1);

		g_theRenderer->BindTexture(m_spriteAtlasTexture);
		g_theRenderer->BindShader(g_theRenderer->CreateOrGetShaderFromFile(GetShaderFilePath("ParticleMesh").c_str(), VertexType::VERTEX_TYPE_MESH_PARTICLE));
		g_theRenderer->SetBlendMode(BlendMode::PREMULTIPLIED_ALPHA);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->SetPrimaryRenderTargetType(PrimaryRenderTargetType::HDR);
		g_theRenderer->DrawVertexBuffer(m_meshVBO, m_totalMeshVertCount, 0, VertexType::VERTEX_TYPE_MESH_PARTICLE);
	}
	if (m_drawCount != 0)
	{
		ComputeShader* addVertsShader = g_theRenderer->CreateOrGetComputeShaderFromFile(GetShaderFilePath("PopulateVBO.hlsl").c_str());
		if (!addVertsShader)
		{
			g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Failed to get populate vbo shader");
			return;
		}

		std::vector<SRV*> srvs;
		std::vector<UAV*> uavs;
		uavs.push_back(m_vboUAV);

		srvs.push_back(m_emitterInstanceSRV);
		srvs.push_back(m_updateDefSRV);
		srvs.push_back(m_particlesSRV);
		srvs.push_back(m_particleDrawListSRV);
		srvs.push_back(m_counterSRV);
		srvs.push_back(m_floatGraphsSRV);
		srvs.push_back(m_float2GraphsSRV);
		srvs.push_back(m_float3GraphsSRV);

		unsigned int threadGroupX = (unsigned int)ceil((double)m_drawCount / 64.f);
		if (threadGroupX > D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION)
		{
			threadGroupX = D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
			//ERROR_RECOVERABLE("Thread group exceeded max size for updating particles");
		}
		g_theRenderer->RunComputeShader(addVertsShader, srvs, uavs, threadGroupX, 1, 1);

		std::vector<SRV*> pssrvs;
		pssrvs.push_back(m_renderDefSRV);
		pssrvs.push_back(m_floatGraphsSRV);
		pssrvs.push_back(m_float2GraphsSRV);
		pssrvs.push_back(m_float3GraphsSRV);
		g_theRenderer->SetPSSRVs(pssrvs, 1);
		g_theRenderer->BindPrevFrameDepthTextureToTRegister(8);

		g_theRenderer->BindTexture(m_spriteAtlasTexture);
		g_theRenderer->BindShader(g_theRenderer->CreateOrGetShaderFromFile(GetShaderFilePath("Emissive").c_str(), VertexType::VERTEX_TYPE_PARTICLE));
		g_theRenderer->SetBlendMode(BlendMode::PREMULTIPLIED_ALPHA);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED_NO_WRITE);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->SetPrimaryRenderTargetType(PrimaryRenderTargetType::HDR);
		g_theRenderer->DrawVertexBuffer(m_vbo, m_drawCount * 6, 0, VertexType::VERTEX_TYPE_PARTICLE);
	}

	if (m_showEditor && m_currentlyEditedEffect != nullptr && m_debugDraw)
	{
		m_currentlyEditedEffect->DebugDrawParticleEffect();
	}

	g_theRenderer->ClearSRVSAndUAVS(9, 1);
}

void ParticleSystem::Shutdown()
{
	for (int i = 0; i < m_particleEffectsOwnedBySystem.size(); i++)
	{
		if (m_particleEffectsOwnedBySystem[i] != nullptr)
		{
			delete m_particleEffectsOwnedBySystem[i];
			m_particleEffectsOwnedBySystem[i] = nullptr;
		}
	}
	delete m_sortConstantsCBO;
	delete m_particleConstantsCBO;
	delete m_particlesUAV;
	delete m_particlesSRV;
	delete m_particlesBuffer;
	delete m_particleAliveList1UAV;
	delete m_particleAliveList1SRV;
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
	delete m_meshVBO;
	delete m_meshVBOUAV;

	delete m_particleDistanceBuffer;
	delete m_particleDistanceUAV;
	delete m_particleDistanceSRV;
	delete m_particleDrawListBuffer;
	delete m_particleDrawListSRV;
	delete m_particleDrawListUAV;

	delete m_meshParticleDistanceBuffer;
	delete m_meshParticleDistanceUAV;
	delete m_meshParticleDistanceSRV;
	delete m_meshParticleDrawListBuffer;
	delete m_meshParticleDrawListSRV;
	delete m_meshParticleDrawListUAV;
	delete m_meshOffsetListBuffer;
	delete m_meshOffsetListSRV;
	delete m_meshOffsetListUAV;

	delete m_updateDefBuffer;
	delete m_updateDefSRV;
	delete m_physicsObjectsBuffer;
	delete m_physicsObjectsSRV;
	delete m_physicsAABB3Buffer;
	delete m_physicsAABB3SRV;

	delete m_renderDefBuffer;
	delete m_renderDefSRV;

	delete m_emitterInstanceBuffer;
	delete m_emitterInstanceSRV;
	delete m_floatGraphsSRV;
	delete m_float2GraphsSRV;
	delete m_float3GraphsSRV;
	delete m_floatGraphsBuffer;
	delete m_float2GraphsBuffer;
	delete m_float3GraphsBuffer;

	delete m_meshParticleEntryStructuredBuffer;
	delete m_meshParticleEntrySRV;

	delete m_meshParticleVertsStructuredBuffer;
	delete m_meshParticleVertsSRV;

	delete m_currentlyEditedEffect;
	delete m_rng;
}

void ParticleSystem::ToggleParticleEditor(bool enableEditor)
{
	m_showEditor = enableEditor;
	if (m_showEditor)
	{
		if (m_currentlyEditedEffect == nullptr)
		{
			ParticleEffectDefinition config;
			config.m_name = "New Effect";

			ParticleEmitterInitializer defToAdd;
			int defIndex = EmplaceDefinitionDataForEmitter();
			defToAdd.m_emitterIndex = defIndex;
			config.m_particleEmitterDefs.push_back(defToAdd);

			int effectDefIdx = (int)m_loadedEffectDefinitions.size();
			config.m_effectDefinitionIndex = effectDefIdx;
			m_loadedEffectDefinitions.push_back(config);

			m_currentlyEditedEffect = new ParticleEffect(config, effectDefIdx);
		}
		else
		{
			m_currentlyEditedEffect->Play();
		}
	}
	else if (!m_showEditor && m_currentlyEditedEffect != nullptr)
	{
		m_currentlyEditedEffect->Stop();
	}
	if (m_config.m_useImGUI)
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		if (m_showEditor)
		{
			io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
			io.WantCaptureKeyboard = true;
		}
		else
		{
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
			io.WantCaptureKeyboard = false;
		}
	}
}

void ParticleSystem::ToggleParticleEditor()
{
	ToggleParticleEditor(!m_showEditor);
}

bool ParticleSystem::IsShowingEditor() const
{
	return m_showEditor;
}

ParticleEffect* ParticleSystem::AddParticleEffectByFileName(std::string effectName, Vec3 const& position, Mat44 const& orientation, bool playImmediately, float playDuration)
{
	ParticleEffectDefinition const& config = CreateOrGetEffectDefinitionFromFile(effectName);
	ParticleEffect* effect = new ParticleEffect(config, config.m_effectDefinitionIndex);
	effect->m_useWorldTransform = true;
	Mat44 transform;
	transform.AppendTranslation3D(position);
	transform.Append(orientation);
	effect->SetWorldTransform(transform);
	effect->Stop();
	if (playImmediately)
	{
		effect->Play(playDuration);
	}
	return effect;
}

ParticleEffect* ParticleSystem::AddParticleEffectByFileName(std::string effectName, Vec3 const& position, EulerAngles const& orientation, bool playImmediately, float playDuration, bool useWorldTransform)
{
	ParticleEffectDefinition const& config = CreateOrGetEffectDefinitionFromFile(effectName);
	ParticleEffect* effect = new ParticleEffect(config, config.m_effectDefinitionIndex);
	effect->m_useWorldTransform = useWorldTransform;
	effect->SetPosition(position);
	effect->SetOrientationDegrees(orientation);
	effect->Stop();

	if (playImmediately)
	{
		effect->Play(playDuration);
	}
	return effect;
}

void ParticleSystem::PlayParticleEffectByFileName(std::string effectName, Vec3 const& position, Mat44 const& orientation, float duraiton)
{
	ParticleEffectDefinition const& config = CreateOrGetEffectDefinitionFromFile(effectName);
	ParticleEffect* effect = new ParticleEffect(config, config.m_effectDefinitionIndex);
	effect->m_useWorldTransform = true;
	Mat44 transform;
	transform.AppendTranslation3D(position);
	transform.Append(orientation);
	effect->SetWorldTransform(transform);
	effect->Stop();
	effect->Play(duraiton);

	for (int i = 0; i < (int)m_particleEffectsOwnedBySystem.size(); i++)
	{
		if (m_particleEffectsOwnedBySystem[i] == nullptr)
		{
			m_particleEffectsOwnedBySystem[i] = effect;
			return;
		}
	}
	m_particleEffectsOwnedBySystem.push_back(effect);
}

void ParticleSystem::PlayParticleEffectByFileName(std::string effectName, Vec3 const& position, EulerAngles const& orientation, float duration)
{
	ParticleEffectDefinition const& config = CreateOrGetEffectDefinitionFromFile(effectName);
	ParticleEffect* effect = new ParticleEffect(config, config.m_effectDefinitionIndex);
	effect->SetPosition(position);
	effect->SetOrientationDegrees(orientation);
	effect->Stop();
	effect->Play(duration);

	for (int i = 0; i < (int)m_particleEffectsOwnedBySystem.size(); i++)
	{
		if (m_particleEffectsOwnedBySystem[i] == nullptr)
		{
			m_particleEffectsOwnedBySystem[i] = effect;
			return;
		}
	}
	m_particleEffectsOwnedBySystem.push_back(effect);
}

ParticlePhysicsObject* ParticleSystem::AddParticlePhysicsObject(Vec3 position, float radius, float forceMagnitude, float fallofExp, bool attract)
{
	for (int i = 0; i < (int)m_livePhysicsObjects.size(); i++)
	{
		if (m_livePhysicsObjects[i].m_isActive == 0)
		{
			m_livePhysicsObjects[i].m_position = position;
			m_livePhysicsObjects[i].m_falloffExponent = fallofExp;
			m_livePhysicsObjects[i].m_forceMagnitude = forceMagnitude;
			m_livePhysicsObjects[i].m_isActive = 1;
			m_livePhysicsObjects[i].m_radius = radius;
			m_livePhysicsObjects[i].m_attract = attract;
			ParticlePhysicsObject* physicsObjCPU = new ParticlePhysicsObject(i);
			return physicsObjCPU;
		}
	}

	int index = (int)m_livePhysicsObjects.size();
	m_livePhysicsObjects.emplace_back();
	m_livePhysicsObjects[index].m_position = position;
	m_livePhysicsObjects[index].m_radius = radius;
	m_livePhysicsObjects[index].m_forceMagnitude = forceMagnitude;
	m_livePhysicsObjects[index].m_falloffExponent = fallofExp;
	m_livePhysicsObjects[index].m_attract = attract;
	m_livePhysicsObjects[index].m_isActive = 1;

	ParticlePhysicsObject* physicsObjCPU = new ParticlePhysicsObject(index);
	return physicsObjCPU;
}

ParticlePhysicsAABB3* ParticleSystem::AddParticlePhysicsAABB3(Vec3 const& mins, Vec3 const& maxs)
{
	for (int i = 0; i < (int)m_livePhysicsAABB3s.size(); i++)
	{
		if (m_livePhysicsAABB3s[i].m_isActive == 0)
		{
			m_livePhysicsAABB3s[i].m_mins = mins;
			m_livePhysicsAABB3s[i].m_maxs = maxs;
			m_livePhysicsAABB3s[i].m_isActive = 1;
			ParticlePhysicsAABB3* physicsAABB3CPU = new ParticlePhysicsAABB3(m_livePhysicsAABB3s[i]);
			return physicsAABB3CPU;
		}
	}

	int index = (int)m_livePhysicsAABB3s.size();
	m_livePhysicsAABB3s.emplace_back();
	m_livePhysicsAABB3s[index].m_mins = mins;
	m_livePhysicsAABB3s[index].m_maxs = maxs;
	m_livePhysicsAABB3s[index].m_isActive = 1;

	ParticlePhysicsAABB3* physicsAABB3CPU = new ParticlePhysicsAABB3(m_livePhysicsAABB3s[index]);
	return physicsAABB3CPU;
}

//add particle emitter instance GPU to the system
int ParticleSystem::AddParticleEmitterToSystem(ParticleEmitterDefinition const& definition, ParticleEmitter* emitter)
{
	EmitterInstanceGPU emitterInstanceGPU;
	EmitterInstanceCPU emitterInstanceCPU;
	emitterInstanceCPU.m_lastFramePos = emitter->GetWorldPosition();
	emitterInstanceCPU.m_emitterLifetime = definition.m_emitterLifetime;
	emitterInstanceCPU.m_delayTimeLeft = definition.m_emitterStartDelay;
	emitterInstanceCPU.m_emitTimeLeft = emitterInstanceCPU.m_emitterLifetime;
	emitterInstanceGPU.m_definitionIndex = definition.m_loadedDefinitionIndex;
	emitterInstanceCPU.m_emitter = emitter;
	emitterInstanceCPU.m_isActive = 1;
	emitterInstanceCPU.m_startPosition = emitter->GetLocalPosition();
	emitterInstanceCPU.m_spawnSubEmitters = definition.m_emitEmitters;
	emitterInstanceCPU.m_currentRepeatTime = g_theParticleSystem->m_updateDefinitions[definition.m_loadedDefinitionIndex].m_repeat == 1 ? 0.f : -1.f;
	emitterInstanceCPU.m_subEmitterDefIdx = definition.m_subEmitterDefinitionIndex;
	emitterInstanceCPU.m_currentBurst = 0;
	emitterInstanceCPU.m_currentBurstInterval = 0.f;

	emitterInstanceCPU.m_loadedEmitterDefIdx = definition.m_loadedDefinitionIndex;
	emitterInstanceCPU.m_seed = m_currentSeed;
	m_currentSeed++;
	//add emitter instance
	for (int i = 0; i < (int)m_liveEmitterInstancesCPU.size(); i++)
	{
		//vacant spot that is not currently sending kill signal to particles 
		if (m_liveEmitterInstancesCPU[i].m_emitter == nullptr && m_liveEmitterInstancesGPU[i].m_killParticles == 0 && m_liveEmitterInstancesCPU[i].m_isActive == false)
		{
			m_liveEmitterInstancesCPU[i] = emitterInstanceCPU;
			m_liveEmitterInstancesGPU[i] = emitterInstanceGPU;
			return i;
		}
	}

	int index = (int)m_liveEmitterInstancesGPU.size();
	m_liveEmitterInstancesGPU.push_back(emitterInstanceGPU);
	m_liveEmitterInstancesCPU.push_back(emitterInstanceCPU);
	return index;
}

void ParticleSystem::LoadEffectByFileName(std::string fileName)
{
	CreateOrGetEffectDefinitionFromFile(fileName);
}

ParticleEffectDefinition ParticleSystem::CreateOrGetEffectDefinitionFromFile(std::string const& fileName)
{
	Strings splitFilePath = SplitStringOnDelimiter(fileName, "/", false);
	std::string effectName = splitFilePath[splitFilePath.size() - 1];
	effectName = SplitStringOnDelimiter(effectName, '.')[0];
	for (int i = 0; i < (int)m_loadedEffectDefinitions.size(); i++)
	{
		if (m_loadedEffectDefinitions[i].m_name == effectName)
		{
			return m_loadedEffectDefinitions[i];
		}
	}
	return CreateEffectDefinitionFromFile(fileName);
}

ParticleEffectDefinition ParticleSystem::CreateEffectDefinitionFromFile(std::string const& fileName)
{
	XmlDocument effectConfigDocument;
	GUARANTEE_OR_DIE(effectConfigDocument.LoadFile(fileName.c_str()) == 0, Stringf("Failed to load effect config file %s", fileName.c_str()));
	XmlElement* rootElement = effectConfigDocument.FirstChildElement("ParticleEffect");
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root element for effect config %s", fileName.c_str()));

	ParticleEffectDefinition outputEffectDefinition;
	outputEffectDefinition.m_name = ParseXmlAttribute(*rootElement, "name", "Missing");
	outputEffectDefinition.m_scale = ParseXmlAttribute(*rootElement, "scale", 1.f);
	XmlElement* emittersRoot = rootElement->FirstChildElement("Emitters");

	std::vector<int> emitterDefsIdxsForEffect;
	for (XmlElement const* currEmitter = emittersRoot->FirstChildElement("Emitter"); currEmitter != nullptr; currEmitter = currEmitter->NextSiblingElement())
	{
		bool configExists = false;
		for (int i = 0; i < (int)m_loadedEmitterDefinitions.size(); i++)
		{
			std::string emitterName = ParseXmlAttribute(*currEmitter, "name", "Missing");
			if (m_loadedEmitterDefinitions[i].m_name == emitterName)
			{
				emitterDefsIdxsForEffect.push_back(i);
				configExists = true;
				break;
			}
		}
		if (!configExists)
		{
			//also adds the emitter definition to the loaded emitter definitions
			int currEmitterDefIdx = ParticleEmitterDefinition::CreateEmitterDefinitionFromXML(currEmitter);
			emitterDefsIdxsForEffect.push_back(currEmitterDefIdx);
		}
	}
	XmlElement* emitterInstancesRoot = rootElement->FirstChildElement("EmitterInstances");
	for (XmlElement const* currEmitterInstance = emitterInstancesRoot->FirstChildElement("EmitterInstance");
		currEmitterInstance != nullptr; currEmitterInstance = currEmitterInstance->NextSiblingElement("EmitterInstance"))
	{
		ParticleEmitterInitializer emitterInstance;
		emitterInstance.m_position = ParseXmlAttribute(*currEmitterInstance, "position", Vec3(0.f, 0.f, 0.f));
		emitterInstance.m_orientationDegrees = ParseXmlAttribute(*currEmitterInstance, "orientationDegrees", EulerAngles(0.f, 0.f, 0.f));

		std::string emitter = ParseXmlAttribute(*currEmitterInstance, "emitter", "Missing");
		for (int i = 0; i < (int)emitterDefsIdxsForEffect.size(); i++)
		{
			if (emitter == g_theParticleSystem->m_loadedEmitterDefinitions[emitterDefsIdxsForEffect[i]].m_name)
			{

				emitterInstance.m_emitterIndex = emitterDefsIdxsForEffect[i];
				outputEffectDefinition.m_particleEmitterDefs.push_back(emitterInstance);
				break;
			}
		}
	}
	int effectDefIndex = (int)m_loadedEffectDefinitions.size();
	outputEffectDefinition.m_effectDefinitionIndex = effectDefIndex;
	m_loadedEffectDefinitions.push_back(outputEffectDefinition);
	return outputEffectDefinition;
}

int ParticleSystem::CreateOrGetEmitterDefinitionFromFile(std::string const& fileName)
{
	Strings splitFilePath = SplitStringOnDelimiter(fileName, '/');
	std::string emitterName = splitFilePath[(int)splitFilePath.size() - 1];
	emitterName = SplitStringOnDelimiter(emitterName, '.')[0];
	TrimString(emitterName, "Emitter");
	for (int i = 0; i < (int)m_loadedEmitterDefinitions.size(); i++)
	{
		if (m_loadedEmitterDefinitions[i].m_name == emitterName)
		{
			return i;
		}
	}
	return CreateEmitterDefinitionFromFile(fileName);
}

int ParticleSystem::CreateEmitterDefinitionFromFile(std::string const& fileName)
{
	XmlDocument effectConfigDocument;
	GUARANTEE_OR_DIE(effectConfigDocument.LoadFile(fileName.c_str()) == 0, Stringf("Failed to load emmiter def file %s", fileName.c_str()));
	XmlElement* rootElement = effectConfigDocument.FirstChildElement("Emitter");
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root element for emmiter def file %s", fileName.c_str()));
	return ParticleEmitterDefinition::CreateEmitterDefinitionFromXML(rootElement);
}

void ParticleSystem::SaveCurrentlyEditedEffectAsXML()
{
	ParticleEffectDefinition* currentlyEditedEffectDefinition = m_currentlyEditedEffect->GetEffectDefinition();
	std::string fileName = "Data/Saves/ParticleEffects/" + currentlyEditedEffectDefinition->m_name + ".xml";
	std::string xml;
	std::vector<std::string> emitterXML;
	xml += Stringf("<ParticleEffect name=\"%s\" scale=\"%.3f\">\n", currentlyEditedEffectDefinition->m_name.c_str(), m_currentlyEditedEffect->GetScale());
	xml += Stringf("<Emitters>\n");
	for (int i = 0; i < (int)currentlyEditedEffectDefinition->m_particleEmitterDefs.size(); i++)
	{
		emitterXML.push_back(currentlyEditedEffectDefinition->m_particleEmitterDefs[i].GetParticleEmitterDefinition().GetAsXML());
		xml += emitterXML[i];
	}
	xml += Stringf("</Emitters>\n");
	xml += Stringf("\t<EmitterInstances>\n");
	for (int i = 0; i < (int)currentlyEditedEffectDefinition->m_particleEmitterDefs.size(); i++)
	{
		ParticleEmitterInitializer currentEmitData = currentlyEditedEffectDefinition->m_particleEmitterDefs[i];
		Vec3 position = currentEmitData.m_position;
		EulerAngles orientation = currentEmitData.m_orientationDegrees;
		xml += Stringf("\t\t<EmitterInstance emitter=\"%s\" position=\"%.2f,%.2f,%.2f\" orientationDegrees=\"%.2f,%.2f,%.2f\"/>\n",
			currentEmitData.GetParticleEmitterDefinition().m_name.c_str(), position.x, position.y, position.z,
			orientation.m_yaw, orientation.m_pitch, orientation.m_roll);
	}
	xml += Stringf("\t</EmitterInstances>\n");
	xml += Stringf("</ParticleEffect>\n");
	WriteStringToFile(xml, fileName);

	for (int i = 0; i < (int)currentlyEditedEffectDefinition->m_particleEmitterDefs.size(); i++)
	{
		std::string emitterFileName = "Data/Saves/ParticleEmitters/" + 
			currentlyEditedEffectDefinition->m_particleEmitterDefs[i].GetParticleEmitterDefinition().m_name + "Emitter.xml";
		WriteStringToFile(emitterXML[i], emitterFileName);
	}
}

AABB2 ParticleSystem::GetImageBoundsInSpriteAtlas(std::string imageFilePath)
{
	if (imageFilePath == "")
	{
		return AABB2(Vec2::ZERO, Vec2::ZERO);
	}

	//check if image is already a part of sprite atlas
	for (int i = 0; i < (int)m_spriteSheetEntries.size(); i++)
	{
		if ( FilePathCompare(m_spriteSheetEntries[i].m_imageFilePath, imageFilePath) )
		{
			return m_spriteSheetEntries[i].m_uvBounds;
		}
	}
	//create image
	Image imageToAdd = Image(imageFilePath.c_str());
	AABB2 imageBoundsUV = AABB2(Vec2(0.f, 0.f), 
		Vec2((float)imageToAdd.GetDimensions().x / (float)m_spriteAtlasImage.GetDimensions().x, (float)imageToAdd.GetDimensions().y / (float)m_spriteAtlasImage.GetDimensions().y));
	if (imageBoundsUV.GetDimensions().x > 1.f || imageBoundsUV.GetDimensions().y > 1.f)
	{
		ERROR_RECOVERABLE("Spritesheet cannot fit in sprite atlas");
		return AABB2(Vec2::ZERO, Vec2::ZERO);
	}

	if (m_spriteSheetEntries.size() == 0)
	{
		CopyImageToSpriteAtlas(imageToAdd, imageBoundsUV);
		return imageBoundsUV;
	}

	//check if you can fit after every top right
	for (int i = 0; i < (int)m_spriteSheetEntries.size(); i++)
	{
		Vec2 imageDimensions = imageBoundsUV.GetDimensions();
		imageBoundsUV.m_mins.x = m_spriteSheetEntries[i].m_uvBounds.m_maxs.x;
		imageBoundsUV.m_mins.y = m_spriteSheetEntries[i].m_uvBounds.m_mins.y;
		imageBoundsUV.m_maxs = imageBoundsUV.m_mins + imageDimensions;
		if (DoImageBoundsFitInSpriteAtlas(imageBoundsUV))
		{
			CopyImageToSpriteAtlas(imageToAdd, imageBoundsUV);
			return imageBoundsUV;
		}
	}

	//check if you can fit after every bottom left
	for (int i = 0; i < (int)m_spriteSheetEntries.size(); i++)
	{
		Vec2 imageDimensions = imageBoundsUV.GetDimensions();
		imageBoundsUV.m_mins.x = m_spriteSheetEntries[i].m_uvBounds.m_mins.x;
		imageBoundsUV.m_mins.y = m_spriteSheetEntries[i].m_uvBounds.m_maxs.y;
		imageBoundsUV.m_maxs = imageBoundsUV.m_mins + imageDimensions;
		if (DoImageBoundsFitInSpriteAtlas(imageBoundsUV))
		{
			CopyImageToSpriteAtlas(imageToAdd, imageBoundsUV);
			return imageBoundsUV;
		}
	}
	ERROR_RECOVERABLE("Spritesheet cannot fit in sprite atlas");
	return AABB2(Vec2::ZERO, Vec2::ZERO);
}

void ParticleSystem::KillAllEmitters()
{
	for (int i = 0; i < m_liveEmitterInstancesGPU.size(); i++)
	{
		if (m_liveEmitterInstancesCPU[i].m_emitter != nullptr)
		{
			m_liveEmitterInstancesCPU[i].m_emitter->Stop();
		}
		m_liveEmitterInstancesCPU[i].m_isActive = 0;
		m_liveEmitterInstancesGPU[i].m_killParticles = 1;
	}
	if (m_currentlyEditedEffect != nullptr)
	{
		delete m_currentlyEditedEffect;
		m_currentlyEditedEffect = nullptr;
	}
}

bool ParticleSystem::DoImageBoundsFitInSpriteAtlas(AABB2 const& imageBoundsUV)
{
	AABB2 spriteAtlasBoundsUV = AABB2::ZERO_TO_ONE;
	if (!spriteAtlasBoundsUV.IsAABB2Inside(imageBoundsUV))
	{
		return false;
	}
	for (int i = 0; i < (int)m_spriteSheetEntries.size(); i++)
	{
		if (DoAABBsOverlap2D(imageBoundsUV, m_spriteSheetEntries[i].m_uvBounds))
		{
			return false;
		}
	}
	return true;
}

void ParticleSystem::CopyImageToSpriteAtlas(Image const& imageToAdd, AABB2 const& imageBoundsUV)
{
	SpriteSheetEntry spriteSheetToAdd;
	spriteSheetToAdd.m_imageFilePath = imageToAdd.GetImageFilePath();
	spriteSheetToAdd.m_uvBounds = imageBoundsUV;
	m_spriteSheetEntries.push_back(spriteSheetToAdd);
	IntVec2 imageToAddDimensions = imageToAdd.GetDimensions();
	for (int y = 0; y < imageToAddDimensions.y; y++)
	{
		for (int x = 0; x < imageToAddDimensions.x; x++)
		{
			Vec2 texelUVs;
			texelUVs.x = ((float)x / (float)imageToAddDimensions.x) * imageBoundsUV.GetDimensions().x + imageBoundsUV.m_mins.x;
			texelUVs.y = ((float)y / (float)imageToAddDimensions.y) * imageBoundsUV.GetDimensions().y + imageBoundsUV.m_mins.y;

			m_spriteAtlasImage.SetTexelColor(texelUVs, imageToAdd.GetTexelColor(IntVec2(x, y)));
		}
	}
	m_spriteAtlasTexture = g_theRenderer->CreateTextureFromImage(m_spriteAtlasImage, false, false, true);
}


int ParticleSystem::CreateOrGetMeshParticleEntry(std::string fileName)
{
	for (int i = 0; i < (int)m_loadedMeshParticleEntries.size(); i++)
	{
		if (FilePathCompare(m_loadedMeshParticleEntries[i].m_name, fileName))
		{
			return m_loadedMeshParticleEntries[i].m_meshEntryIndex;
		}
	}

	std::vector<Vertex_PCUTBN> meshPCUTBNVerts;
	std::vector<unsigned int> meshIndexes;
	bool hasNormals = false;
	bool hasUVs = false;
	ObjLoader::Load(g_theRenderer, fileName, meshPCUTBNVerts, meshIndexes, hasNormals, hasUVs);

	m_meshParticleVerts.reserve(m_meshParticleVerts.size() + meshIndexes.size());
	unsigned int newMeshEntryOffset = (unsigned int)m_meshParticleVerts.size();
	unsigned int newMeshEntrySize = (unsigned int)meshIndexes.size();
	int newMeshEntryIndex = (int)m_meshParticleEntriesGPU.size();
	m_loadedMeshParticleEntries.emplace_back(LoadedMeshEntry{ fileName, newMeshEntryIndex });
	m_meshParticleEntriesGPU.emplace_back(MeshEntry{ newMeshEntrySize, newMeshEntryOffset });

	for (int i = 0; i < (int)meshIndexes.size(); i++)
	{
		Vertex_PCUTBN meshVertPCUTBN = meshPCUTBNVerts[meshIndexes[i]];
		MeshParticleVert vertToAdd;
		vertToAdd.m_position = meshVertPCUTBN.m_position;
		if (hasUVs)
		{
			vertToAdd.m_uv = meshVertPCUTBN.m_uvTexCoords;
		}
		else
		{
			vertToAdd.m_uv = Vec2(.5f, .5f);
		}
		if (hasNormals)
		{
			vertToAdd.m_normal = meshVertPCUTBN.m_normal;
		}
		else
		{
			vertToAdd.m_normal = Vec3::ZERO;
		}
		m_meshParticleVerts.emplace_back(vertToAdd);
	}

	if (m_meshParticleEntrySRV != nullptr)
	{
		delete m_meshParticleEntrySRV;
		delete m_meshParticleEntryStructuredBuffer;
	}
	if (m_meshParticleVertsSRV != nullptr)
	{
		delete m_meshParticleVertsSRV;
		delete m_meshParticleVertsStructuredBuffer;
	}

	m_meshParticleEntryStructuredBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(MeshEntry), (unsigned int)m_meshParticleEntriesGPU.size(), (void*)m_meshParticleEntriesGPU.data());
	m_meshParticleEntrySRV = g_theRenderer->CreateSRV(m_meshParticleEntryStructuredBuffer);

	m_meshParticleVertsStructuredBuffer = g_theRenderer->CreateStructuredBuffer(sizeof(MeshParticleVert), (unsigned int)m_meshParticleVerts.size(), m_meshParticleVerts.data());
	m_meshParticleVertsSRV = g_theRenderer->CreateSRV(m_meshParticleVertsStructuredBuffer);

	return newMeshEntryIndex;
}


void ParticleSystem::InitializeDefaultEffectValues()
{
	for (int i = 0; i < (int)m_currentlyEditedEffect->m_emitters.size(); i++)
	{
		ParticleEmitter* currentlyEditedEmitter = m_currentlyEditedEffect->m_emitters[i];

		FloatGraph* maxSpeedFloatGraph = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_MAX_SPEED);
		SetDefaultValueForFloatGraph(maxSpeedFloatGraph, 10.f);
		FloatGraph* dragForce = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_DRAG_FORCE);
		SetDefaultValueForFloatGraph(dragForce, 0.f);
		FloatGraph* curlNoiseForce = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_CURL_NOISE_FORCE);
		SetDefaultValueForFloatGraph(curlNoiseForce, 0.f);
		FloatGraph* pointForce = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_POINT_FORCE);
		SetDefaultValueForFloatGraph(pointForce, 0.f);
		FloatGraph* vortexForce = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_VORTEX_FORCE);
		SetDefaultValueForFloatGraph(vortexForce, 0.f);
		FloatGraph* emissive = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_EMISSIVE);
		SetDefaultValueForFloatGraph(emissive, 0.f);
		FloatGraph* alphaObscurance = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_ALPHA_OBSCURANCE);
		SetDefaultValueForFloatGraph(alphaObscurance, 1.f);
		FloatGraph* panTextureContribution = currentlyEditedEmitter->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_PAN_TEXTURE_CONTRIBUTION);
		SetDefaultValueForFloatGraph(panTextureContribution, 0.f);

		Float2Graph* size = currentlyEditedEmitter->GetFloat2GraphByType(Float2GraphType::FLOAT2GRAPH_SIZE);
		SetDefaultValueForFloat2Graph(size, Vec2(1.f, 1.f));

		Float3Graph* linearForce = currentlyEditedEmitter->GetFloat3GraphByType(Float3GraphType::FLOAT3GRAPH_LINEAR_FORCE);
		SetDefaultValueForFloat3Graph(linearForce, Vec3(0.f, 0.f, 10.f));
		Float3Graph* lifetimeVelocity = currentlyEditedEmitter->GetFloat3GraphByType(Float3GraphType::FLOAT3GRAPH_LIFETIME_VELOCITY);
		SetDefaultValueForFloat3Graph(lifetimeVelocity, Vec3(0.f, 0.f, 10.f));
	}
}

void ParticleSystem::SetDefaultValueForFloatGraph(FloatGraph* floatGraph, float defaultValue)
{
	floatGraph->constantValue = defaultValue;
	floatGraph->minValue = defaultValue;
	floatGraph->maxValue = defaultValue;
	floatGraph->numPoints = 1;
	floatGraph->points[0].m_time = 0.f;
	floatGraph->points[0].m_minValue = defaultValue;
	floatGraph->points[0].m_maxValue = defaultValue;
}

void ParticleSystem::SetDefaultValueForFloat2Graph(Float2Graph* floatGraph, Vec2 defaultValue)
{
	floatGraph->constantValue[0] = defaultValue.x;
	floatGraph->constantValue[1] = defaultValue.y;

	floatGraph->minValue[0] = defaultValue.x;
	floatGraph->minValue[1] = defaultValue.y;

	floatGraph->maxValue[0] = defaultValue.x;
	floatGraph->maxValue[1] = defaultValue.y;

	floatGraph->numPoints = 1;
	floatGraph->points[0].m_time = 0.f;

	floatGraph->points[0].m_minValue[0] = defaultValue.x;
	floatGraph->points[0].m_minValue[1] = defaultValue.y;
	floatGraph->points[0].m_maxValue[0] = defaultValue.x;
	floatGraph->points[0].m_maxValue[1] = defaultValue.y;
}

void ParticleSystem::SetDefaultValueForFloat3Graph(Float3Graph* floatGraph, Vec3 defaultValue)
{
	floatGraph->constantValue[0] = defaultValue.x;
	floatGraph->constantValue[1] = defaultValue.y;
	floatGraph->constantValue[2] = defaultValue.z;

	floatGraph->minValue[0] = defaultValue.x;
	floatGraph->minValue[1] = defaultValue.y;
	floatGraph->minValue[2] = defaultValue.z;

	floatGraph->maxValue[0] = defaultValue.x;
	floatGraph->maxValue[1] = defaultValue.y;
	floatGraph->maxValue[2] = defaultValue.z;

	floatGraph->numPoints = 1;
	floatGraph->points[0].m_time = 0.f;

	floatGraph->points[0].m_minValue[0] = defaultValue.x;
	floatGraph->points[0].m_minValue[1] = defaultValue.y;
	floatGraph->points[0].m_minValue[2] = defaultValue.z;


	floatGraph->points[0].m_maxValue[0] = defaultValue.x;
	floatGraph->points[0].m_maxValue[1] = defaultValue.y;
	floatGraph->points[0].m_maxValue[2] = defaultValue.z;
}

int ParticleSystem::EmplaceDefinitionDataForEmitter()
{
	int defIndex = (int)m_updateDefinitions.size();
	m_loadedEmitterDefinitions.emplace_back();
	m_loadedEmitterDefinitions[defIndex].m_loadedDefinitionIndex = defIndex;
	m_updateDefinitions.emplace_back();
	m_renderDefinitions.emplace_back();
	m_emitterVelocityGraphs.emplace_back();

	m_floatGraphs.resize(m_updateDefinitions.size() * (size_t)FloatGraphType::NUM_FLOATGRAPHS);
	m_float2Graphs.resize(m_updateDefinitions.size() * (size_t)Float2GraphType::NUM_FLOAT2GRAPHS);
	m_float3Graphs.resize(m_updateDefinitions.size() * (size_t)Float3GraphType::NUM_FLOAT3GRAPHS);

	m_loadedEmitterDefinitions[defIndex].InitializeDefaultFloatGraphValues();
	m_loadedEmitterDefinitions[defIndex].InitializeDefaultFloat2GraphValues();
	m_loadedEmitterDefinitions[defIndex].InitializeDefaultFloat3GraphValues();

	return defIndex;
}

Vec3 ParticleSystem::GetValueInFloat3Graph(Float3Graph& graph, float normalizedTime, unsigned int seed, bool seperateAxis)
{
	if (graph.dataMode == 0)
	{
		return Vec3(graph.constantValue);
	}

	if (graph.dataMode == 1)
	{
		return GetFloat3NoiseInRange(graph.minValue, graph.maxValue, 0, seed, seperateAxis);
	}

	Vec3 outputValue = GetFloat3NoiseInRange(Vec3(graph.points[0].m_minValue), Vec3(graph.points[0].m_maxValue), 0, seed, seperateAxis);
	for (int i = 0; i < (int)graph.numPoints; i++)
	{
		if (i == (int)graph.numPoints - 1)
		{
			outputValue = GetFloat3NoiseInRange(Vec3(graph.points[i].m_minValue), Vec3(graph.points[i].m_maxValue), 0, seed, seperateAxis);
			return outputValue;
		}
		float startTime = graph.points[i].m_time;
		float endTime = graph.points[i + 1].m_time;
		if (startTime > normalizedTime || endTime < normalizedTime)
		{
			continue;
		}

		float fractionInRange = (normalizedTime - startTime) / (endTime - startTime);
		fractionInRange = GetValueFromEasingFunction((EasingFunction)graph.points[i].m_easingFunction, fractionInRange);
		Vec3 minValue = Vec3::Lerp(Vec3(graph.points[i].m_minValue), Vec3(graph.points[i + 1].m_minValue), fractionInRange);
		Vec3 maxValue = Vec3::Lerp(Vec3(graph.points[i].m_maxValue), Vec3(graph.points[i + 1].m_maxValue), fractionInRange);
		outputValue = GetFloat3NoiseInRange(minValue, maxValue, 0, seed, seperateAxis);
		return outputValue;
	}
	return outputValue;
}

Vec3 ParticleSystem::GetFloat3NoiseInRange(Vec3 min, Vec3 max, int index, unsigned int seed, bool seperateAxis)
{
	if (seperateAxis)
	{
		float normalizedX = Get1dNoiseZeroToOne(index, seed);
		float normalizedY = Get1dNoiseZeroToOne(index, seed + 1);
		float normalizedZ = Get1dNoiseZeroToOne(index, seed + 2);
		Vec3 output;
		output.x = Lerp(min.x, max.x, normalizedX);
		output.y = Lerp(min.y, max.y, normalizedY);
		output.z = Lerp(min.z, max.z, normalizedZ);

		return output;
	}
	else
	{
		float normalizedTime = Get1dNoiseZeroToOne(index, seed);
		return Vec3::Lerp(min, max, normalizedTime);
	}
}

MeshParticleVert::MeshParticleVert(Vec3 const& position, Vec2 const& uv, Vec3 const& normal)
	: m_position(position)
	, m_uv(uv)
	, m_normal(normal)
{
}
