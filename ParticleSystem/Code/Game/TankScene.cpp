#include "Game/TankScene.hpp"
#include "Game/Model.hpp"
#include "Game/TankProjectile.hpp"

void TankScene::StartUp()
{
	float startX = -37.5f;
	for (int i = 0; i < 5; i++)
	{
		float xOffset = startX + (float)i * 15.f;
		Model* blueTank = new Model(g_theGame, Vec3(xOffset, -15.f, 0.f));
		blueTank->m_orientationDegrees = EulerAngles(90.f, 0.f, 0.f);
		blueTank->m_color = Rgba8(10, 115, 153);
		blueTank->Load("Data/Models/Tank1.xml");
		m_blueTanks.push_back(blueTank);
		m_sceneEntities.push_back(blueTank);
	}

	for (int i = 0; i < 5; i++)
	{
		float xOffset = startX + (float)i * 15.f;

		Model* redTank = new Model(g_theGame, Vec3(xOffset, 15.f, 0.f));
		redTank->m_orientationDegrees = EulerAngles(-90.f, 0.f, 0.f);
		redTank->Load("Data/Models/Tank1.xml");
		redTank->m_color = Rgba8(156, 22, 22);
		m_redTanks.push_back(redTank);
		m_sceneEntities.push_back(redTank);
	}

	m_blueFireTimer = Timer(5.5f, g_theApp->m_clock);
	m_redFireTimer = Timer(5.5f, g_theApp->m_clock);
	m_blueRespawnTimer = Timer(3.5f, g_theApp->m_clock);
	m_redRespawnTimer = Timer(3.5f, g_theApp->m_clock);
}

void TankScene::SwitchOn()
{
	m_blueFireTimer.Start();
	m_blueFireTimer.SetTimePosition(5.f);
	for (int i = 0; i < m_blueTanks.size(); i++)
	{
		m_blueTanks[i]->m_isActive = true;
	}
	for (int i = 0; i < m_redTanks.size(); i++)
	{
		m_redTanks[i]->m_isActive = true;
	}
}

void TankScene::SwitchOff()
{
	Scene::SwitchOff();
	m_blueFireTimer.Stop();
	m_redFireTimer.Stop();
	m_blueRespawnTimer.Stop();
	m_redRespawnTimer.Stop();

	for (int i = 0; i < (int)m_sceneEntities.size(); i++)
	{
		TankProjectile* projectile = dynamic_cast<TankProjectile*>(m_sceneEntities[i]);
		if (projectile != nullptr)
		{
			delete m_sceneEntities[i];
			m_sceneEntities[i] = nullptr;
		}
	}
}

void TankScene::Update(float deltaSeconds)
{
	Scene::Update(deltaSeconds);
	if (m_blueFireTimer.HasPeriodElapsed())
	{
		m_redRespawnTimer.Start();
		m_blueFireTimer.Stop();
		m_redFireTimer.Start();

		Vec3 startLocation(-37.5f, -10.f, 1.f);

		for (int i = 0; i < 5; i++)
		{
			float xOffset = startLocation.x + i * (float)15.f;
			TankProjectile* tankProjectile = new TankProjectile(g_theGame, Vec3(xOffset, startLocation.y, startLocation.z), true);
			tankProjectile->m_velocity = Vec3(0.f, 25.f, 0.f);
			tankProjectile->m_travelDistance = 25.f;
			tankProjectile->m_liveTime = 3.f;
			tankProjectile->m_tankToDestroy = m_redTanks[i];
			bool added = false;
			for (int e = 0; e < (int)m_sceneEntities.size(); e++)
			{
				if (m_sceneEntities[e] == nullptr)
				{
					m_sceneEntities[e] = tankProjectile;
					added = true;
					break;
				}
			}
			if (!added)
			{
				m_sceneEntities.push_back(tankProjectile);
			}
		}
	}
	if (m_redFireTimer.HasPeriodElapsed())
	{
		m_blueRespawnTimer.Start();
		m_redFireTimer.Stop();
		m_blueFireTimer.Start();

		Vec3 startLocation(-37.5f, 10.f, 1.f);

		for (int i = 0; i < 5; i++)
		{
			float xOffset = startLocation.x + i * (float)15.f;
			TankProjectile* tankProjectile = new TankProjectile(g_theGame, Vec3(xOffset, startLocation.y, startLocation.z), false);
			tankProjectile->m_velocity = Vec3(0.f, -25.f, 0.f);
			tankProjectile->m_travelDistance = 25.f;
			tankProjectile->m_liveTime = 3.f;
			tankProjectile->m_tankToDestroy = m_blueTanks[i];
			bool added = false;
			for (int e = 0; e < (int)m_sceneEntities.size(); e++)
			{
				if (m_sceneEntities[e] == nullptr)
				{
					m_sceneEntities[e] = tankProjectile;
					added = true;
					break;
				}
			}
			if (!added)
			{
				m_sceneEntities.push_back(tankProjectile);
			}
		}
	}
	if (m_blueRespawnTimer.HasPeriodElapsed())
	{
		m_blueRespawnTimer.Stop();
		for (int i = 0; i < m_blueTanks.size(); i++)
		{
			m_blueTanks[i]->m_isActive = true;
		}
	}
	if (m_redRespawnTimer.HasPeriodElapsed())
	{
		m_redRespawnTimer.Stop();
		for (int i = 0; i < m_redTanks.size(); i++)
		{
			m_redTanks[i]->m_isActive = true;
		}
	}
}
