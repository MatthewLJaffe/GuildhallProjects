#include "PerformanceScene.hpp"

void PerformanceScene::Update(float deltaSeconds)
{
	const float flyingFiresPerSecond = 235.f;
	m_firesThisFrame += deltaSeconds * flyingFiresPerSecond;

	int roundedFiresThisFrame = (int)m_firesThisFrame;
	m_firesThisFrame -= roundedFiresThisFrame;
	FloatRange xSpawnRange(-60.f, 60.f);
	FloatRange ySpawnRange(-60.f, 60.f);
	FloatRange zSpawnRange(0.f, 80.f);

	for (int i = 0; i < roundedFiresThisFrame; i++)
	{
		bool entryAdded = false;
		for (int fe = 0; fe < (int)m_flyingFires.size(); fe++)
		{
			if (m_flyingFires[fe].m_effectInstance->IsActive() == 0)
			{
				delete m_flyingFires[fe].m_effectInstance;
				m_flyingFires[fe].m_effectInstance = nullptr;

				FlyingFireEntry fireEntry;
				fireEntry.m_orientaiton = EulerAngles(g_randGen->RollRandomFloatInRange(0.f, 360.f), g_randGen->RollRandomFloatInRange(0.f, 360.f), g_randGen->RollRandomFloatInRange(0.f, 360.f));
				fireEntry.m_speed = 20.f;
				Vec3 randomPos = Vec3(g_randGen->RollRandomFloatInRange(xSpawnRange), g_randGen->RollRandomFloatInRange(ySpawnRange), g_randGen->RollRandomFloatInRange(zSpawnRange));
				fireEntry.m_effectInstance = g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/FireV1.xml", randomPos, fireEntry.m_orientaiton, false);
				fireEntry.m_effectInstance->Play(5.f);
				m_flyingFires[fe] = fireEntry;
				entryAdded = true;
				break;
			}
		}

		if (!entryAdded)
		{
			FlyingFireEntry fireEntry;
			fireEntry.m_orientaiton = EulerAngles(g_randGen->RollRandomFloatInRange(0.f, 360.f), g_randGen->RollRandomFloatInRange(0.f, 360.f), g_randGen->RollRandomFloatInRange(0.f, 360.f));
			fireEntry.m_speed = 20.f;
			Vec3 randomPos = Vec3(g_randGen->RollRandomFloatInRange(xSpawnRange), g_randGen->RollRandomFloatInRange(ySpawnRange), g_randGen->RollRandomFloatInRange(zSpawnRange));
			fireEntry.m_effectInstance = g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/FireV1.xml", randomPos, fireEntry.m_orientaiton, false);
			fireEntry.m_effectInstance->Play(5.f);
			m_flyingFires.push_back(fireEntry);
		}

	}


	for (int i = 0; i < m_flyingFires.size(); i++)
	{
		m_flyingFires[i].m_effectInstance->SetPosition(m_flyingFires[i].m_effectInstance->GetPosition() +
			(m_flyingFires[i].m_orientaiton.GetKUp() * -m_flyingFires[i].m_speed * deltaSeconds));
	}

}

void PerformanceScene::StartUp()
{
}

void PerformanceScene::SwitchOn()
{
}

void PerformanceScene::SwitchOff()
{
	Scene::SwitchOff();
	for (int i = 0; i < m_flyingFires.size(); i++)
	{
		if (m_flyingFires[i].m_effectInstance != nullptr)
		{
			delete m_flyingFires[i].m_effectInstance;
			m_flyingFires[i].m_effectInstance = nullptr;
		}
	}
	m_flyingFires.clear();
}
