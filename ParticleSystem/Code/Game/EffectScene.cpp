#include "Game/EffectScene.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"

void EffectScene::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	
	if (m_earthShatterTimer.HasPeriodElapsed() || m_earthShatterTimer.IsStopped())
	{
		m_earthShatterTimer.Start();
		m_sceneParticleEffects[0]->Play(1.5f);
	}
	
	if (m_laserBeamTimer.HasPeriodElapsed() || m_earthShatterTimer.IsStopped())
	{
		m_laserBeamTimer.m_period = 3.f;
		m_laserBeamTimer.Start();
		m_sceneParticleEffects[1]->Play(3.f);
	}
}

void EffectScene::StartUp()
{
	m_laserBeamTimer = Timer(3.f, g_theApp->m_clock);
	m_earthShatterTimer = Timer(6.f, g_theApp->m_clock);
}

void EffectScene::SwitchOn()
{
	m_sceneParticleEffects.push_back(g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/EarthShatter.xml", Vec3(0.f, -10.f, 0.f), EulerAngles(), false, 1.5f));
	m_sceneParticleEffects.push_back(g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/LaserBeam.xml", Vec3(0.f, 10.f, 3.f), EulerAngles(), false, 3.f));
	
	m_earthShatterTimer.Start();
	m_laserBeamTimer.m_period = .1f;
	m_laserBeamTimer.Start();
	m_sceneParticleEffects[0]->Play(1.5f);
}

void EffectScene::SwitchOff()
{
	Scene::SwitchOff();

	m_earthShatterTimer.Stop();
	m_laserBeamTimer.Stop();
}
