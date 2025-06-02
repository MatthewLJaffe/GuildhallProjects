#include "Game/BigEffectScene.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"

void BigEffectScene::Update(float deltaSeconds)
{
	Scene::Update(deltaSeconds);
	/*
	UNUSED(deltaSeconds);
	if (m_beatTimer.HasPeriodElapsed())
	{
		m_noiseOn = !m_noiseOn;
		m_beatTimer.Start();
		EmitterUpdateDefinitionGPU* updateDef = m_sceneParticleEffects[0]->GetEffectDefinition()->GetUpdateDefByIndex(0);
		FloatGraph* forceGraph = m_sceneParticleEffects[0]->m_emitters[0]->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_CURL_NOISE_FORCE);
		m_sceneParticleEffects[0]->m_emitters[0]->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_CURL_NOISE_FORCE)->dataMode = 0;
		if (m_noiseOn)
		{
			forceGraph->constantValue = 120.f;
			updateDef->m_curlNoisePan[0] = 0.f;
			updateDef->m_curlNoisePan[1] = 0.f;
			updateDef->m_curlNoisePan[2] = 0.f;
		}
		else
		{
			forceGraph->constantValue = 0.f;
			updateDef->m_curlNoisePan[0] = 20.f;
			updateDef->m_curlNoisePan[1] = 20.f;
			updateDef->m_curlNoisePan[2] = 20.f;
		}
		updateDef->isDirty = 1;
		forceGraph->isDirty = 1;
	}
	*/
	UNUSED(deltaSeconds);
	DebugAddWorldBillboardText("RMB to shoot at head", Vec3(0.f, 0.f, 25.f), 1.f, Vec2(.5f, .5f), 0.f);
}

void BigEffectScene::StartUp()
{
	//m_beatTimer = Timer(60.f / 128.f, g_theApp->m_clock);
	//m_music = g_theAudio->CreateOrGetSound("Data/Audio/NewGame.mp3");
}

void BigEffectScene::SwitchOn()
{
	/*
	m_sceneParticleEffects.push_back(g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/GlowyBlueEffect.xml", Vec3(0.f, 0.f, 0.f), EulerAngles(0.f, 0.f, 0.f)));
	m_musicPlayback = g_theAudio->StartSound(m_music);
	m_beatTimer.Start();
	*/
	m_sceneParticleEffects.push_back(g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/Head.xml", Vec3(0.f, 0.f, 5.f), EulerAngles(0.f, 0.f, 0.f)));

	
}

void BigEffectScene::SwitchOff()
{
	Scene::SwitchOff();
	//m_beatTimer.Stop();
	//g_theAudio->StopSound(m_musicPlayback);
}
