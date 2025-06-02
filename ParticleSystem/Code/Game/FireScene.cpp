#include "Game/FireScene.hpp"
#include "Game/Prop.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"

void FireScene::Update(float deltaSeconds)
{
	m_currentSceneTime += deltaSeconds;
	float t = RangeMap(SinDegrees(m_currentSceneTime * m_fireMoveSpeed), -1.f, 1.f, 0.f, 1.f);
	float yPos = Lerp(m_yFireRange.m_min, m_yFireRange.m_max, t);
	if (m_wildFire != nullptr)
	{
		m_wildFire->SetPosition(Vec3(0.f, yPos, 0.f));
	}
}

void FireScene::StartUp()
{
	Prop* testWall = new Prop(g_theGame, Vec3(7.f, -10.f, 0.f));
	testWall->m_physicsBounds = AABB3(-1.f, -5.f, 0.f, 1.f, 5.f, 5.f);
	AddVertsForAABB3D(testWall->m_vertexes, AABB3(-1.f, -5.f, 0.f, 1.f, 5.f, 5.f));
	m_sceneEntities.push_back(testWall);

	//m_testWallCollider = g_theParticleSystem->AddParticlePhysicsAABB3(Vec3(10.f, 0.f, 0.f) + Vec3(-2.5f, -5.f, 0.f), Vec3(10.f, 0.f, 0.f) + Vec3(2.5f, 5.f, 5.f));
}

void FireScene::SwitchOn()
{
	//m_sceneParticleEffects.push_back(g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/FireOld.xml", Vec3(0, 10.f, 0.f), EulerAngles(0.f, 0.f, 0.f)));
	//m_sceneParticleEffects.push_back(g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/FireV1.xml", Vec3(0, 5.f, 0.f), EulerAngles(0.f, 0.f, 0.f)));
	m_sceneParticleEffects.push_back(g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/MediumFire.xml", Vec3(0, 5.f, 0.f), EulerAngles(0.f, 0.f, 0.f)));
	m_sceneParticleEffects.push_back(g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/LargeFire.xml", Vec3(0, -2.5f, 0.f), EulerAngles(180.f, 0.f, 0.f)));
	m_sceneParticleEffects.push_back(g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/FlamethrowerEffect.xml", Vec3(-2, -10.f, 3.f), EulerAngles(267.f, 0.f, 0.f)));

	m_sceneParticleEffects.push_back(g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/Explosion.xml", Vec3(10.f, 0.f, 10.f), EulerAngles(0.f, 0.f, 0.f)));
	m_sceneParticleEffects.push_back(g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/EnergyExplosion.xml", Vec3(10.f, -20.f, 10.f), EulerAngles(0.f, 0.f, 0.f)));

	m_wildFire = g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/WildFire.xml", Vec3(0, -20.f, 0.f), EulerAngles(0.f, 0.f, 0.f));
	m_sceneParticleEffects.push_back(m_wildFire);
}
