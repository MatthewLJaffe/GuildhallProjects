#include "Game/Boss1.hpp"
#include "Game/BoxHazard.hpp"
#include "Game/Player.hpp"

Boss1::Boss1(GameState* gameState, EntityType entityType, Vec2 const& startPos)
	: Entity(gameState, entityType, startPos)
{
	m_spriteBounds = AABB2(Vec2::ZERO, Vec2(32.f, 32.f));
	m_color = Rgba8(236, 1, 106);
	m_rotationTimer = Timer(1.f, g_theApp->GetGameClock());
	m_hornAttackTimer = Timer(1.f, g_theApp->GetGameClock());
	m_rythmicRotationTimer = Timer(1.f, g_theApp->GetGameClock());
	m_randomMoveTimer = Timer(1.f, g_theApp->GetGameClock());
	m_moveTowardsPlayerTimer = Timer(1.f, g_theApp->GetGameClock());
	m_randomMoveTimer.Stop();
	m_moveTowardsPlayerTimer.Stop();

	m_spikeyBallConfig.m_animation = "ProjectileSpawn";
	m_spikeyBallConfig.m_collisionRadius = 4.f;
	m_spikeyBallConfig.m_liveTime = 10.f;
	m_spikeyBallConfig.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(9.f, 9.f));
	m_spikeyBallConfig.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/SpikeyBall.png");
	m_isHazard = true;
	m_radius = 16.f;
}

void Boss1::Update(float deltaSeconds)
{
	UpdateAnimation();
	UNUSED(deltaSeconds);
	if (m_attackInSpiral)
	{
		UpdateSpiralAttack();
	}
	if (m_rotationsLeft > 0)
	{
		UpdateRythmicRotation(deltaSeconds);
	}
	if (m_hornAttack)
	{
		UpdateAttack();
	}
	if (!m_randomMoveTimer.HasPeriodElapsed() && !m_randomMoveTimer.IsStopped())
	{
		UpdateRandomMove(deltaSeconds);
	}
	if (!m_moveTowardsPlayerTimer.HasPeriodElapsed() && !m_moveTowardsPlayerTimer.IsStopped())
	{
		MoveTowardsPlayer(deltaSeconds);
	}
	if (m_arms.size() > 0)
	{
		for (int i = 0; i < (int)m_arms.size(); i++)
		{
			m_arms[i]->SetPosition(GetPosition());
		}
	}
}

void Boss1::AttackInSpiral(float attackOrientation, float rotationDuration, float timePerAttack, int numAttacks)
{
	m_attackInSpiral = true;
	m_rotationTimer.m_period = rotationDuration;
	m_startRotation = m_orientationDegrees;
	m_goalRotation = attackOrientation;
	m_hornAttackTimer.m_period = timePerAttack;
	m_numHornAttacks = numAttacks;
	m_rotationTimer.Start();

}

void Boss1::UpdateSpiralAttack()
{
	if (!m_rotationTimer.HasPeriodElapsed())
	{
		m_orientationDegrees = Lerp(m_startRotation, m_goalRotation, SmoothStep3(m_rotationTimer.GetElapsedFraction()));
	}
	else
	{
		if (m_hornAttackTimer.IsStopped() || m_hornAttackTimer.HasPeriodElapsed())
		{
			PlayAnimation("Boss1Attack");
			m_orientationDegrees = m_goalRotation;
			m_numHornAttacks--;
			Vec2 leftProjectileDisp = Vec2::MakeFromPolarDegrees(m_orientationDegrees + 60.f) * 16.f;
			Vec2 rightProjectileDisp = Vec2::MakeFromPolarDegrees(m_orientationDegrees + 120.f) * 16.f;
			EnemyProjectile* hornProjectileLeft = new EnemyProjectile(m_gameState, EntityType::ENEMY_PROJECTILE, m_position + leftProjectileDisp, m_spikeyBallConfig);
			hornProjectileLeft->m_velocity = leftProjectileDisp.GetNormalized() * 120.f;
			EnemyProjectile* hornProjectileRight = new EnemyProjectile(m_gameState, EntityType::ENEMY_PROJECTILE, m_position + rightProjectileDisp, m_spikeyBallConfig);
			hornProjectileRight->m_velocity = rightProjectileDisp.GetNormalized() * 120.f;

			m_gameState->AddEntity(hornProjectileLeft);
			m_gameState->AddEntity(hornProjectileRight);
			if (m_numHornAttacks <= 0)
			{
				m_hornAttackTimer.Stop();
				m_attackInSpiral = false;
				return;
			}
			else
			{
				m_hornAttackTimer.Start();
			}
		}
	}
}

void Boss1::UpdateAttack()
{
	if (m_hornAttackTimer.IsStopped() || m_hornAttackTimer.HasPeriodElapsed())
	{
		PlayAnimation("Boss1Attack");
		m_numHornAttacks--;
		float startTheta = m_orientationDegrees + 60.f;
		float endTheta = m_orientationDegrees + 120.f;
		for (int i = 0; i < m_projectilesPerAttack; i++)
		{
			float angleFraction = (float)i / (float)(m_projectilesPerAttack - 1);
			float attackOrientation = Lerp(startTheta, endTheta, angleFraction);
			Vec2 projectileDisp = Vec2::MakeFromPolarDegrees(attackOrientation) * 16.f;
			EnemyProjectile* projectile = new EnemyProjectile(m_gameState, EntityType::ENEMY_PROJECTILE, m_position + projectileDisp, m_spikeyBallConfig);
			projectile->m_velocity = projectileDisp.GetNormalized() * 120.f;
			m_gameState->AddEntity(projectile);
		}

		if (m_numHornAttacks <= 0)
		{
			m_hornAttackTimer.Stop();
			m_hornAttack = false;
			return;
		}
		else
		{
			m_hornAttackTimer.Start();
		}
	}
}

void Boss1::Attack(int numberOfProjectilesInAttack, int numberOfAttacks, float timePerAttack)
{
	m_hornAttackTimer.m_period = timePerAttack;
	m_hornAttackTimer.Stop();
	m_projectilesPerAttack = numberOfProjectilesInAttack;
	m_numHornAttacks = numberOfAttacks;
	m_hornAttack = true;
}

void Boss1::UpdateRythmicRotation(float deltaSeconds)
{
	//reset timer
	if (m_rythmicRotationTimer.HasPeriodElapsed())
	{
		m_rotationsLeft--;
		m_rotationsComplete++;
		if (m_rotationsLeft == 0)
		{
			return;
		}
		m_rythmicRotationTimer.Start();
		if (m_rotationsComplete % 4 == 3)
		{
			m_startRotation = m_orientationDegrees;
			m_goalRotation = m_startRotation + g_randGen->RollRandomFloatInRange(m_fastTurnAngleRange.x, m_fastTurnAngleRange.y);
		}
	}
	//every 4th rotation do a fast lerp
	if (m_rotationsComplete % 4 == 3)
	{
		m_orientationDegrees = Lerp(m_startRotation, m_goalRotation, SmoothStep3(SmoothStep3(m_rythmicRotationTimer.GetElapsedFraction())));
	}
	else
	{
		m_orientationDegrees += deltaSeconds * m_slowTurnRotationSpeed;
	}
}

void Boss1::StartRythmicRotation(int numberOfBeats, float secondsPerBeat)
{
	m_rythmicRotationTimer.m_period = secondsPerBeat;
	m_rotationsComplete = 0;
	m_rotationsLeft = numberOfBeats;
	m_rythmicRotationTimer.Start();
}

void Boss1::StartRandomMovement(float moveTime)
{
	m_moveTarget.x = g_randGen->RollRandomFloatInRange(GetWorldScreenDimensions().x * .2f, GetWorldScreenDimensions().x * .8f);
	m_moveTarget.y = g_randGen->RollRandomFloatInRange(GetWorldScreenDimensions().y * .2f, GetWorldScreenDimensions().y * .8f);
	m_randomMoveTimer.m_period = moveTime;
	m_randomMoveTimer.Start();
}

void Boss1::StartChasePlayer(float moveTime)
{
	m_moveTowardsPlayerTimer.m_period = moveTime;
	m_moveTowardsPlayerTimer.Start();
}

void Boss1::MoveTowardsPlayer(float deltaSeconds)
{
	Vec2 playerPos = m_gameState->GetPlayer()->GetPosition();
	m_acceleration = playerPos - m_position;
	m_acceleration.SetLength(48.f);

	m_velocity += m_acceleration * deltaSeconds;
	if (m_velocity.GetLength() > 48.f)
	{
		m_velocity.SetLength(48.f);
	}

	m_position += m_velocity * deltaSeconds;
}

void Boss1::UpdateRandomMove(float deltaSeconds)
{
	float acc = 60.f;
	m_acceleration = (m_moveTarget - m_position).GetNormalized() * acc;
	m_velocity += m_acceleration * deltaSeconds;
	if (m_velocity.GetLength() > m_moveSpeed)
	{
		m_velocity.SetLength(m_moveSpeed);
	}
	m_position += m_velocity * deltaSeconds;
	if (GetDistance2D(m_moveTarget, m_position) < 100.f)
	{
		m_moveTarget.x = g_randGen->RollRandomFloatInRange(GetWorldScreenDimensions().x * .2f, GetWorldScreenDimensions().x * .8f);
		m_moveTarget.y = g_randGen->RollRandomFloatInRange(GetWorldScreenDimensions().y * .2f, GetWorldScreenDimensions().y * .8f);
	}
}

void Boss1::AddArm()
{
	float armOrientation = 0.f;
	if (m_arms.size() > 0)
	{
		Vec2 armDirection = Vec2::ZERO;
		for (int i = 0; i < (int)m_arms.size(); i++)
		{
			armDirection += Vec2::MakeFromPolarDegrees(m_arms[i]->m_orientationDegrees);
		}
		armDirection *= 1.f / (float)m_arms.size();
		if (armDirection.GetLength() < .01f)
		{
			armOrientation = m_arms[0]->m_orientationDegrees + 90.f;
		}
		else
		{
			armOrientation = (-armDirection).GetOrientationDegrees();
		}
	}



	ProjectileConfig projectileConfig;

	projectileConfig.m_startOrientaiton = armOrientation;
	projectileConfig.m_rotationSpeed = 40.f;
	Vec2 boxDimensions(15.f, .2f);
	projectileConfig.m_liveTime = 999.f;
	projectileConfig.m_becomeHazardTime = .5f;
	projectileConfig.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(16.f * boxDimensions.x, 16.f * boxDimensions.y));
	projectileConfig.m_normalizedPivot = Vec2(0.f, .5f);
	projectileConfig.m_animation = "SpawnBox";
	Vec2 boxPosition = GetPosition();
	BoxHazard* arm = new BoxHazard(m_gameState, EntityType::ENEMY_PROJECTILE, boxPosition, projectileConfig);
	m_arms.push_back(arm);
	m_gameState->AddEntity(arm);
}

void Boss1::KillBoss()
{
	PlayAnimation("Boss1Die");
	for (int i = 0; i < m_arms.size(); i++)
	{
		m_arms[i]->DestroyEntity();
	}
	m_arms.clear();
}
