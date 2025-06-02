#include "Game/Boss3.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"
#include "Engine/Core/Clock.hpp"

Boss3::Boss3(GameState* gameState, EntityType entityType, Vec2 const& startPos, EntityConfig const& config)
	: Entity(gameState, entityType, startPos, config)
{
	m_boss3Sheet = g_theRenderer->CreateOrGetSpriteSheetFromFile("Data/Images/Boss3Sheet.png", IntVec2(14, 14));
	m_shakeTimer = Timer(.96f, g_theApp->GetGameClock());
	m_armsShakeTimer = Timer(1.5f, g_theApp->GetGameClock());
	m_wiggleTimer = Timer(.2f, g_theApp->GetGameClock());
	m_wiggleFastTimer = Timer(.1f, g_theApp->GetGameClock());
	m_changeFaceTimer = Timer(.1f, g_theApp->GetGameClock());
	m_timeBetweenSpeedUpArms = Timer(5.f, g_theApp->GetGameClock());
}

void Boss3::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	UpdateAnimation();
	UpdateWiggle();
	UpdateWiggleFast();
	UpdateShake();
	UpdateArmsShake();

	if (m_timesToChangeFace > 0 && m_changeFaceTimer.HasPeriodElapsed())
	{
		m_timesToChangeFace--;
		m_changeFaceTimer.Start();
		if (m_timesToChangeFace == 0)
		{
			if (m_endFaceOnSaw)
			{
				m_uvs = m_boss3Sheet->GetSpriteDef(3).GetUVs();
				m_color = Rgba8(236, 1, 106);
			}
			else
			{
				m_uvs = m_boss3Sheet->GetSpriteDef(0).GetUVs();
				m_color = Rgba8(103, 210, 243);
			}
		}
		else
		{
			IntVec2 faceCoords = IntVec2(g_randGen->RollRandomIntInRange(0, 4), g_randGen->RollRandomIntInRange(0, 1));
			m_uvs = m_boss3Sheet->GetSpriteDef(m_boss3Sheet->GetIndexFromCoords(faceCoords)).GetUVs();
			if (g_randGen->RollRandomFloatZeroToOne() > .3f)
			{
				m_color = Rgba8(236, 1, 106);
			}
			else
			{
				m_color = Rgba8(103, 210, 243);
			}
		}
	}

	if (m_speedUpArms)
	{
		HandleSpeedUpArms();
	}
}

bool Boss3::OverlapsPlayer(Player* player)
{
	if (m_hidden)
	{
		return false;
	}
	return Entity::OverlapsPlayer(player);
}

void Boss3::Render()
{
	if (m_hidden)
	{
		return;
	}
	Entity::Render();
} 

void Boss3::Wiggle()
{
	m_wiggleTimer.Start();
	PlayAnimation("Wiggle");
	m_uvs = m_boss3Sheet->GetSpriteDef(g_randGen->RollRandomIntInRange(1, 4)).GetUVs();
	SpawnTearBullets();
}

void Boss3::Shake(float shakeTime, bool changeFaces, float shakeScale)
{
	m_shakeScale = shakeScale;
	m_shakeTimer.m_period = shakeTime;
	m_shakeTimer.Start();
	if (changeFaces)
	{
		m_timesToChangeFace = RoundDownToInt(shakeTime / m_changeFaceTimer.m_period);
		m_changeFaceTimer.Start();
	}
}

void Boss3::ArmShake(bool endFaceOnSaw, float armShakeDuration)
{
	m_endFaceOnSaw = endFaceOnSaw;
	m_armsShakeTimer.m_period = armShakeDuration;
	m_armsShakeTimer.Start();
	m_timesToChangeFace = RoundDownToInt(m_armsShakeTimer.m_period / m_changeFaceTimer.m_period) - 4;
	m_changeFaceTimer.Start();
}

void Boss3::WiggleFast()
{
	m_wiggleTimer.Start();
	PlayAnimation("WiggleFast");
	m_uvs = m_boss3Sheet->GetSpriteDef(g_randGen->RollRandomIntInRange(1, 4)).GetUVs();
	SpawnTearBullets();
}

void Boss3::UpdateWiggle()
{
	if (!m_wiggleTimer.IsStopped() && !m_wiggleTimer.HasPeriodElapsed())
	{
		float rotationNoise = Compute1dPerlinNoise(g_theApp->GetGameClock()->GetTotalSeconds() * m_wiggleScale, 1000000.f, 4);
		float noiseRotation = RangeMap(rotationNoise, -1.f, 1.f, -m_wiggleDeltaDegrees, m_wiggleDeltaDegrees);
		if (m_wiggleTimer.GetElapsedFraction() < .5f)
		{
			m_orientationDegrees = Lerp(0.f, noiseRotation, SmoothStep3(m_wiggleTimer.GetElapsedFraction() * 2.f));
		}
		else
		{
			m_orientationDegrees = Lerp(noiseRotation, 0.f, SmoothStep3((m_wiggleTimer.GetElapsedFraction() - .5f) * 2.f));
		}
	}
	else if (m_wiggleTimer.HasPeriodElapsed())
	{
		m_wiggleTimer.Stop();
		m_orientationDegrees = 0.f;
	}
}

void Boss3::UpdateWiggleFast()
{
	if (!m_wiggleFastTimer.IsStopped() && !m_wiggleFastTimer.HasPeriodElapsed())
	{
		float rotationNoise = Compute1dPerlinNoise(g_theApp->GetGameClock()->GetTotalSeconds() * m_wiggleScale, 1000000.f, 4);
		float noiseRotation = RangeMap(rotationNoise, -1.f, 1.f, -m_wiggleDeltaDegrees, m_wiggleDeltaDegrees);
		if (m_wiggleFastTimer.GetElapsedFraction() < .5f)
		{
			m_orientationDegrees = Lerp(0.f, noiseRotation, SmoothStep3(m_wiggleFastTimer.GetElapsedFraction() * 2.f));
		}
		else
		{
			m_orientationDegrees = Lerp(noiseRotation, 0.f, SmoothStep3((m_wiggleFastTimer.GetElapsedFraction() - .5f) * 2.f));
		}
	}
	else if (m_wiggleFastTimer.HasPeriodElapsed())
	{
		m_wiggleFastTimer.Stop();
		m_orientationDegrees = 0.f;
	}
}

void Boss3::UpdateShake()
{
	if (!m_shakeTimer.IsStopped() && !m_shakeTimer.HasPeriodElapsed())
	{
		float rotationNoise = Compute1dPerlinNoise(g_theApp->GetGameClock()->GetTotalSeconds() * m_wiggleScale, m_shakeScale, 4);
		float noiseRotation = RangeMap(rotationNoise, -1.f, 1.f, -20.f, 20.f);
		if (m_shakeTimer.GetElapsedFraction() < .5f)
		{
			m_orientationDegrees = Lerp(0.f, noiseRotation, SmoothStep3(m_shakeTimer.GetElapsedFraction() * 2.f));
		}
		else
		{
			m_orientationDegrees = Lerp(noiseRotation, 0.f, SmoothStep3((m_shakeTimer.GetElapsedFraction() - .5f) * 2.f));
		} 
	}
	else if (m_shakeTimer.HasPeriodElapsed())
	{
		m_shakeTimer.Stop();
		m_orientationDegrees = 0.f;
	}
}

void Boss3::UpdateArmsShake()
{
	float shakeSlowdownTime = .8f;
	if (!m_armsShakeTimer.IsStopped() && !m_armsShakeTimer.HasPeriodElapsed())
	{
		float rotationNoise = Compute1dPerlinNoise(g_theApp->GetGameClock()->GetTotalSeconds() * m_wiggleScale, m_shakeScale, 4);
		float noiseRotation = RangeMap(rotationNoise, -1.f, 1.f, -15.f, 15.f);
		if (m_armsShakeTimer.GetElapsedFraction() < shakeSlowdownTime)
		{
			m_orientationDegrees = noiseRotation;
		}
		else
		{
			m_orientationDegrees = Lerp(noiseRotation, 0.f, GetFractionWithinRange(m_shakeTimer.GetElapsedFraction(), shakeSlowdownTime, 1.f));
		}
	}
	else if (m_armsShakeTimer.HasPeriodElapsed())
	{
		m_armsShakeTimer.Stop();
		m_orientationDegrees = 0.f;
	}
}

void Boss3::SetRandomSprite()
{
	if (g_randGen->RollRandomIntInRange(0, 3) == 0)
	{
		m_color = Rgba8(103, 210, 243);
	}
	else
	{
		m_color = Rgba8(236, 1, 106);
	}
	m_uvs = m_boss3Sheet->GetSpriteDef(g_randGen->RollRandomIntInRange(0, 4)).GetUVs();
}

void Boss3::SpawnTearBullets()
{
	EntityConfig tearBulletConfig = EntityConfig::GetEntityConfigByName("TearBullet");
	tearBulletConfig.m_startVelocity = Vec2::MakeFromPolarDegrees(g_randGen->RollRandomFloatInRange(120.f, 180.f)) * g_randGen->RollRandomFloatInRange(32.f, 64.f);
	Vec2 leftEyePos = GetPosition() + Vec2(-14.f, 9.f);
	m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, leftEyePos, tearBulletConfig));

	tearBulletConfig.m_startVelocity = Vec2::MakeFromPolarDegrees(g_randGen->RollRandomFloatInRange(0.f, 60.f)) * g_randGen->RollRandomFloatInRange(32.f, 64.f);
	Vec2 rightEyePos = GetPosition() + Vec2(14.f, 9.f);
	m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, rightEyePos, tearBulletConfig));
}

void Boss3::AddArm(float rotationSpeed)
{
	float startOrientation = 0.f;
	if (m_arms.size() > 0.f)
	{
		startOrientation = m_arms[(int)m_arms.size() - 1]->m_orientationDegrees + 60.f;
	}

	ProjectileConfig projectileConfig;

	projectileConfig.m_startOrientaiton = startOrientation;
	projectileConfig.m_rotationSpeed = rotationSpeed;
	Vec2 boxDimensions(15.f, .2f);
	projectileConfig.m_liveTime = 999.f;
	projectileConfig.m_becomeHazardTime = 1.25f;
	projectileConfig.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(16.f * boxDimensions.x, 16.f * boxDimensions.y));
	projectileConfig.m_normalizedPivot = Vec2(0.f, .5f);
	projectileConfig.m_animation = "SpawnBoss3Box";
	Vec2 boxPosition = GetPosition();
	BoxHazard* arm1 = new BoxHazard(m_gameState, EntityType::ENEMY_PROJECTILE, boxPosition, projectileConfig);
	arm1->m_sortOrder = 8;
	m_arms.push_back(arm1);
	m_gameState->AddEntity(arm1);

	projectileConfig.m_startOrientaiton += 180.f;
	BoxHazard* arm2 = new BoxHazard(m_gameState, EntityType::ENEMY_PROJECTILE, boxPosition, projectileConfig);
	arm2->m_sortOrder = 8;
	m_arms.push_back(arm2);
	m_gameState->AddEntity(arm2);
}

void Boss3::RemoveArms()
{
	for (int i = 0; i < (int)m_arms.size(); i++)
	{
		m_gameState->RemoveEntity(m_arms[i]);
		m_arms[i] = nullptr;
	}
	m_arms.clear();
}

void Boss3::StartSpeedUpArms()
{
	m_arms[m_armSpeedUpIdx]->m_config.m_rotationSpeed *= 2.f;
	m_arms[m_armSpeedUpIdx]->PlayAnimation("ScaleUpALittle");
	m_arms[m_armSpeedUpIdx + 1]->m_config.m_rotationSpeed *= 2.f;
	m_arms[m_armSpeedUpIdx + 1]->PlayAnimation("ScaleUpALittle");
	m_armSpeedUpIdx += 2;
	m_speedUpArms = true;
	m_timeBetweenSpeedUpArms.Start();
}

void Boss3::HandleSpeedUpArms()
{
	if (!m_timeBetweenSpeedUpArms.IsStopped() && m_timeBetweenSpeedUpArms.HasPeriodElapsed())
	{
		float armDiff = GetShortestAngularDispDegrees(m_arms[m_armSpeedUpIdx]->m_orientationDegrees, m_arms[m_armSpeedUpIdx - 1]->m_orientationDegrees);
		if (armDiff > 55.f && armDiff < 65.f)
		{
			m_arms[m_armSpeedUpIdx]->m_config.m_rotationSpeed *= 2.f;
			m_arms[m_armSpeedUpIdx]->PlayAnimation("ScaleUpALittle");
			m_arms[m_armSpeedUpIdx + 1]->m_config.m_rotationSpeed *= 2.f;
			m_arms[m_armSpeedUpIdx + 1]->PlayAnimation("ScaleUpALittle");
			m_armSpeedUpIdx += 2;
			if (m_armSpeedUpIdx == m_arms.size())
			{
				m_speedUpArms = false;
			}
			else
			{
				m_timeBetweenSpeedUpArms.Start();
			}
		}
	}
}
  