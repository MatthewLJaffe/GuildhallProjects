#include "Game/Tasks.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameState.hpp"
#include "Game/Boss1.hpp"
#include "Game/GameStateLevel.hpp"
#include "Game/Particle.hpp"
#include "Game/AttractProjectile.hpp"
#include "Game/BoxHazard.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/BobbingProjectile.hpp"
#include "Game/Scythe.hpp"
#include "Game/Wisp.hpp"
#include "Game/Boss3.hpp"
#include "Game/BigGear.hpp"
#include "Game/Sawblade.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"

Task::Task(GameStateLevel* gameState, float taskDurationInBeats)
{
	m_numBeats = taskDurationInBeats;
	m_gameState = gameState;
}

void Task::Tick()
{
}

void Task::Begin()
{

}

void Task::End()
{
}

void Task::DebugSkip()
{
}

T_PlayAnimation::T_PlayAnimation(GameStateLevel* gameState, float taskDuration, Entity* entityToPlayAnimOn, std::string animName)
	: Task(gameState, taskDuration)
	, m_animName(animName)
	, m_entityToPlayAnimOn(entityToPlayAnimOn)
{
}

void T_PlayAnimation::Begin()
{
	m_entityToPlayAnimOn->PlayAnimation(m_animName);
}

void T_PlayAnimation::End()
{
	m_entityToPlayAnimOn->SetTimeOffsetInAnimation(m_numBeats * m_gameState->m_beatTime);
}

void T_PlayAnimation::DebugSkip()
{
	m_entityToPlayAnimOn->PlayAnimation(m_animName);
	m_entityToPlayAnimOn->SetTimeOffsetInAnimation(m_numBeats * m_gameState->m_beatTime);
}

T_B1_A1_StillSpiral::T_B1_A1_StillSpiral(GameStateLevel* gameState, float taskDuration)
	: Task(gameState, taskDuration)
{

}

void T_B1_A1_StillSpiral::Begin()
{
	m_gameState->GetBoss1()->AttackInSpiral(0.f, 0.f, m_gameState->m_beatTime, 2);
}

void T_B1_A1_StillSpiral::Tick()
{
	if (m_gameState->GetCurrentTaskBeats() > 3.f && !m_firedSecondVolley)
	{
		m_firedSecondVolley = true;
		m_gameState->GetBoss1()->AttackInSpiral(225.f, m_gameState->m_beatTime, m_gameState->m_beatTime, 3);
	}
	if (m_gameState->GetCurrentTaskBeats() > 7.f && !m_firedThirdVolley)
	{
		m_firedThirdVolley = true;
		m_gameState->GetBoss1()->AttackInSpiral(135.f, .1f, .1f, 3);
	}
	if (m_gameState->GetCurrentTaskBeats() > 9.f && !m_firedFourthVolley)
	{
		m_firedFourthVolley = true;
		m_gameState->GetBoss1()->AttackInSpiral(135.f, 0.f, m_gameState->m_beatTime, 2);
	}
	if (m_gameState->GetCurrentTaskBeats() > 11.f && !m_firedFifthVolley)
	{
		m_firedFifthVolley = true;
		m_gameState->GetBoss1()->AttackInSpiral(0.f, m_gameState->m_beatTime, m_gameState->m_beatTime, 3);
	}
}

T_B1_A1_SecondSpiral::T_B1_A1_SecondSpiral(GameStateLevel* gameState, float numBeats, int rythmicRotationBeats)
	: Task(gameState, numBeats)
	, m_rytmicRotationBeats(rythmicRotationBeats)
{

}

void T_B1_A1_SecondSpiral::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		if (currentBeatNumber == 30)
		{
			m_gameState->GetBoss1()->StartRandomMovement(m_gameState->m_beatTime * 19.f);
		}
		if (currentBeatNumber % 4 == 1)
		{
			m_gameState->GetBoss1()->Attack(5, 3, .1f);
		}
		else
		{
			m_gameState->GetBoss1()->Attack(2, 1, 0.f);
		}
	}
}

void T_B1_A1_SecondSpiral::Begin()
{
	m_gameState->GetBoss1()->StartRythmicRotation(m_rytmicRotationBeats, m_gameState->m_beatTime);
}

T_B1_A2::T_B1_A2(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
	m_panToBossTime = Timer(2.5f, g_theApp->GetGameClock());
	m_enlargeTimer = Timer(10.f, g_theApp->GetGameClock());
	m_attractSpawnTimer = Timer(.2f, g_theApp->GetGameClock());
}


void T_B1_A2::Begin()
{
	m_panToBossTime.Start();
	m_enlargeTimer.Start();
	m_attractSpawnTimer.Start();
	m_gameState->GetBoss1()->PlayAnimation("Boss1Wiggle");
}


void T_B1_A2::Tick()
{
	if (!m_panToBossTime.HasPeriodElapsed())
	{
		g_theGame->SetWorldCameraPos(Vec2::Lerp(GetWorldScreenDimensions() * .5f, m_gameState->GetBoss1()->GetPosition(), SmoothStep5(m_panToBossTime.GetElapsedFraction())));
	}
	if (!m_enlargeTimer.HasPeriodElapsed())
	{
		m_gameState->GetBoss1()->m_spriteBounds.m_maxs = Vec2::Lerp(m_startSpriteDimensions, m_finalSpriteDimensions, m_enlargeTimer.GetElapsedFraction());
		if (m_attractSpawnTimer.HasPeriodElapsed())
		{
			AttractProjectileConfig config;
			config.m_animation = "AttractProjectile";
			config.m_liveTime = 2.5f;
			config.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/SpikeyBall.png");
			config.m_targetPos = m_gameState->GetBoss1()->GetPosition();
			config.m_stillPercentage = .25f;
			AABB2 cameraBounds(g_theGame->m_worldCamera.GetOrthoBottomLeft(), g_theGame->m_worldCamera.GetOrthoTopRight());
			Vec2 spawnPos = cameraBounds.GetRandomPointOutsideBox(32.f, g_randGen);
			m_gameState->AddEntity(new AttractProjectile(m_gameState, EntityType::ENEMY_PROJECTILE, spawnPos, config));
			m_attractSpawnTimer.Start();
		}
	}
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		if (currentBeatNumber % 5 == 0)
		{
			m_gameState->SpawnExplodingProjectile();
		}
		if (currentBeatNumber == 78)
		{
			m_gameState->GetBoss1()->m_spriteBounds.m_maxs = m_startSpriteDimensions;
			m_gameState->GetBoss1()->PlayAnimation("Boss1Shrink");
		}
		if (currentBeatNumber == 80)
		{
			ParticleConfig particleConfig;
			particleConfig.m_animation = "Boss1BeatDrop";
			particleConfig.m_liveTime = .3f;
			particleConfig.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Ring.png");
			particleConfig.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(32.f, 32.f));
			m_gameState->AddEntity(new Particle(m_gameState, EntityType::PLAYER_VFX, m_gameState->GetBoss1()->GetPosition(), particleConfig));
		}
	}
}

T_B1_RotatingHazards::T_B1_RotatingHazards(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
	m_panToBossTimer = Timer(2.5f, g_theApp->GetGameClock());
}

void T_B1_RotatingHazards::Begin()
{
	for (float theta = 0.f; theta <= 360.f; theta += 90)
	{
		//m_gameState->AddEntity(new BoxHazard(m_gameState, EntityType::ENEMY_PROJECTILE, boxPosition, projectileConfig));
	}
	//m_gameState->SpawnConeHazard();
}

void T_B1_RotatingHazards::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		if (currentBeatNumber < 108)
		{
			if (g_randGen->RollRandomFloatZeroToOne() > .5f)
			{
				m_gameState->SpawnRandomDiscHazard();
			}
			else
			{
				m_gameState->SpawnDiscHazardOnPlayer();
			}
			if (currentBeatNumber % 2 == 0)
			{
				m_gameState->SpawnRandomBarHazard();
			}
		}
		else
		{
			if (currentBeatNumber > 110 && currentBeatNumber % 10 == 0)
			{
				m_gameState->SpawnConeHazard();
			}
			if (currentBeatNumber == 112)
			{
				m_gameState->SpawnConeHazard();
			}
			if (currentBeatNumber == 113)
			{
				m_gameState->GetBoss1()->PlayAnimation("Boss1Thump");
			}
		}
		if (currentBeatNumber == 129)
		{
			m_gameState->GetBoss1()->AddArm();
		}
		if (currentBeatNumber == 145)
		{
			m_gameState->GetBoss1()->AddArm();
			m_gameState->GetBoss1()->StartChasePlayer(m_gameState->m_beatTime * 16.f);
		}
		if (currentBeatNumber == 161)
		{
			m_panToBossTimer.Start();
			m_cameraStartPos = g_theGame->m_worldCamera.GetCameraPos();
			m_gameState->GetBoss1()->AddArm();
		}
		if (currentBeatNumber == 171)
		{
			m_gameState->GetBoss1()->AddArm();
		}
	}

	if (!m_panToBossTimer.IsStopped() && !m_panToBossTimer.HasPeriodElapsed())
	{
		g_theGame->SetWorldCameraPos(Vec2::Lerp(m_cameraStartPos, m_gameState->GetBoss1()->GetPosition(), SmoothStep5(m_panToBossTimer.GetElapsedFraction())));
	}
}

void T_B1_Die::Begin()
{
	m_gameState->GetBoss1()->KillBoss();
}

void T_B1_Die::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		if (currentBeatNumber == 262)
		{
			m_gameState->SpawnBossExplosion();
		}
		if (currentBeatNumber == 266)
		{
			m_gameState->GetBoss1()->m_isHazard = false;
			m_gameState->GetBoss1()->PlayAnimation("Boss1Fade");
		}
	}
}

T_B1_Die::T_B1_Die(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
}

T_B2_Intro::T_B2_Intro(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
	m_sprialBulletTimer = Timer(gameState->m_beatTime * .125f, g_theApp->GetGameClock());
}


void T_B2_Intro::Begin()
{
	m_sprialBulletTimer.Start();
}

void T_B2_Intro::End()
{
	m_gameState->DestroyWisps();
}

void T_B2_Intro::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		if (currentBeatNumber % 2 == 0 && currentBeatNumber <= 10)
		{
			m_gameState->SpawnWispOffscreen();
		}
	}

	m_spriralTheta += g_theApp->GetGameClock()->GetDeltaSeconds() * m_sprialRotationSpeed;
	if (m_sprialBulletTimer.HasPeriodElapsed() && currentBeatNumber < 22)
	{
		m_sprialBulletTimer.Start();
		EntityConfig circleBulletConfig = EntityConfig::GetEntityConfigByName("CircleBullet");
		circleBulletConfig.m_startVelocity = Vec2::MakeFromPolarDegrees(m_spriralTheta, 48.f);
		circleBulletConfig.m_startAcceleration = Vec2::MakeFromPolarDegrees(m_spriralTheta, 32.f);
		Vec2 startPos = m_gameState->GetBoss2()->GetPosition();
		m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, startPos, circleBulletConfig));
	}


}

T_B2_BossSpawn::T_B2_BossSpawn(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{

}

void T_B2_BossSpawn::Begin()
{
	m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, m_gameState->GetBoss2()->GetPosition(), EntityConfig::GetEntityConfigByName("MediumCircleParticle")));
}

void T_B2_BossSpawn::DebugSkip()
{
	m_gameState->GetBoss2()->Spawn();
}


void T_B2_BossSpawn::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		int beatTasks = RoundDownToInt(m_gameState->GetCurrentTaskBeats());
		if (beatTasks == 2)
		{
			m_gameState->GetBoss2()->Spawn();
			EntityConfig bulletConfig = EntityConfig::GetEntityConfigByName("MediumCircleBullet");
			m_gameState->SpawnProjectilesInCircle(m_gameState->GetBoss2()->GetPosition(), 15, bulletConfig, 48.f, 48.f);
			g_theGame->ShakeScreen(m_gameState->m_beatTime * .25f, 2.f);
		}
		if (beatTasks > 3 && beatTasks < 30 && beatTasks % 3 == 0)
		{
			g_theGame->ShakeScreen(m_gameState->m_beatTime * .25f, 1.5f);
			if (g_randGen->RollRandomFloatZeroToOne() > .5f)
			{
				EntityConfig bulletConfig = EntityConfig::GetEntityConfigByName("CircleBullet");
				m_gameState->SpawnProjectilesInCircle(m_gameState->GetBoss2()->GetPosition(), 30, bulletConfig);
			}
			else
			{
				EntityConfig bulletConfig = EntityConfig::GetEntityConfigByName("MediumCircleBullet");
				m_gameState->SpawnProjectilesInCircle(m_gameState->GetBoss2()->GetPosition(), 15, bulletConfig, 48.f, 48.f);
			}

		}
		if (beatTasks > 2 && beatTasks < 30 && beatTasks % 5 == 0)
		{
			EntityConfig bulletConfig = EntityConfig::GetEntityConfigByName("CircleBullet");
			m_gameState->SpawnProjectilesInSpiral(m_gameState->GetBoss2()->GetPosition(), 30, m_gameState->m_beatTime / 15.f, bulletConfig,32.f, 32.f, g_randGen->RollRandomFloatZeroToOne() > .5f);
		}
	}
}

T_B2_TeleportingAttacks::T_B2_TeleportingAttacks(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
	for (int i = 0; i < 4; i++)
	{
		m_teleportLocations.push_back(m_gameState->RollRandomPosOnScreen(Vec2(.1f, .1f)));
	}
	m_teleportLocations.push_back(GetWorldScreenDimensions() * .5f);
	for (int i = 0; i < 4; i++)
	{
		m_teleportLocations.push_back(m_gameState->RollRandomPosOnScreen(Vec2(.1f, .1f)));
	}
	m_teleportLocations.push_back(GetWorldScreenDimensions() * .5f);
	for (int i = 0; i < 3; i++)
	{
		m_teleportLocations.push_back(m_gameState->RollRandomPosOnScreen(Vec2(.1f, .1f)));
	}
	m_teleportLocations.push_back(GetWorldScreenDimensions() * .5f);
}

void T_B2_TeleportingAttacks::Begin()
{

}

void T_B2_TeleportingAttacks::SpawnBoopBoopBeep()
{
	EntityConfig config = EntityConfig::GetEntityConfigByName("DottedLineCircle");
	Vec2 circlePos = m_gameState->RollRandomPosOnScreen(Vec2(.25f, .25f));
	m_gameState->SpawnEntityAtPos(config, EntityType::PLAYER_VFX, circlePos);
	EntityConfig delayedCircleBullet = EntityConfig::GetEntityConfigByName("CircleBullet");
	delayedCircleBullet.m_sortOrder = 1;
	delayedCircleBullet.m_hideTime = m_gameState->m_beatTime * 2.f;
	m_gameState->SpawnProjectilesInCircle(circlePos, 20, delayedCircleBullet, 64.f, 32.f);
}

void T_B2_TeleportingAttacks::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		int currentTaskBeats = RoundDownToInt(m_gameState->GetCurrentTaskBeats());
		if (currentTaskBeats == 3 || currentTaskBeats == 5 || currentTaskBeats == 19 || currentTaskBeats == 21)
		{
			SpawnBoopBoopBeep();
		}
		int circleSpawnBeat1 = 5;
		int teleportStartBeat1 = 8;
		int circleSpawnBeat2 = 13;
		int teleportStartBeat2 = 16;
		int circleSpawnBeat3 = 21;
		int teleportStartBeat3 = 24;


		if (currentTaskBeats >= 0 && currentTaskBeats < 4)
		{
			m_gameState->GetBoss2()->Teleport(GetWorldScreenDimensions() * .5f);
		}
		if (currentTaskBeats >= circleSpawnBeat1 && currentTaskBeats < 5 + circleSpawnBeat1 || 
			currentTaskBeats >= circleSpawnBeat2 && currentTaskBeats < 4 + circleSpawnBeat2 || 
			currentTaskBeats >= circleSpawnBeat3 && currentTaskBeats < 5 + circleSpawnBeat3)
		{
			EntityConfig config = EntityConfig::GetEntityConfigByName("TeleportHazardIndicator");
			m_gameState->SpawnEntityAtPos(config, EntityType::ENEMY_PROJECTILE, m_teleportLocations[m_currentTeleportCircleIdx]);
			m_currentTeleportCircleIdx++;
		}
		if (currentTaskBeats >= teleportStartBeat1 && currentTaskBeats < 5 + teleportStartBeat1 || 
			currentTaskBeats >= teleportStartBeat2 && currentTaskBeats < 4 + teleportStartBeat2 || 
			currentTaskBeats >= teleportStartBeat3 && currentTaskBeats < 5 + teleportStartBeat3)
		{
			m_gameState->GetBoss2()->Teleport(m_teleportLocations[m_currentTeleportIdx]);
			m_currentTeleportIdx++;
			if (m_currentTeleportIdx == 5 || m_currentTeleportIdx == (int)m_teleportLocations.size())
			{
				EntityConfig config = EntityConfig::GetEntityConfigByName("CircleBullet");
				m_gameState->SpawnProjectilesInSpiral(m_gameState->GetBoss2()->GetPosition(), 90, m_gameState->m_beatTime / 30.f, config, 32.f, 32.f, true, 3.f);
			}
		}
	}
}

T_B2_DevilAttacks::T_B2_DevilAttacks(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
}

void T_B2_DevilAttacks::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		int currentTaskBeats = RoundDownToInt(m_gameState->GetCurrentTaskBeats());
		if (currentTaskBeats % 4 == 0 && currentTaskBeats > 0 && currentTaskBeats < 24)
		{
			DoDevilAttackAtPos(m_nextRandomAttackPos);
		}
		if (currentTaskBeats % 4 == 1 && currentTaskBeats < 20)
		{
			m_nextRandomAttackPos = g_theGame->GetRandomPosInWorldScreen(Vec2(.25f, .25f));
			EntityConfig config = EntityConfig::GetEntityConfigByName("TeleportHazardIndicator");
			m_gameState->SpawnEntityAtPos(config, EntityType::PLAYER_VFX, m_nextRandomAttackPos);
		}
		if (currentTaskBeats == 32)
		{
			m_gameState->BeatDropKillAllScythes();
			g_theGame->ShakeScreen(.25f, 2.f);
		}
	}
}

void T_B2_DevilAttacks::Begin()
{
	m_gameState->GetBoss2()->PlayDevilAnimation();
	DoDevilAttackAtPos(GetWorldScreenDimensions() * .5f);
}

void T_B2_DevilAttacks::DoDevilAttackAtPos(Vec2 const& pos)
{
	m_gameState->GetBoss2()->Teleport(pos, true);
	EntityConfig config = EntityConfig::GetEntityConfigByName("Scythe");
	m_gameState->SpawnScytheAtPos(config, EntityType::SCYTHE, m_gameState->GetBoss2()->GetPosition() + Vec2::RIGHT * 36);
}

T_B2_CrazyAttack::T_B2_CrazyAttack(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
}

void T_B2_CrazyAttack::Begin()
{
	EntityConfig config = EntityConfig::GetEntityConfigByName("TeleportHazardIndicator");
	m_gameState->SpawnEntityAtPos(config, EntityType::PLAYER_VFX, GetWorldScreenDimensions() * .5f);
	m_gameState->GetBoss2()->m_children[1]->PlayAnimation("Boss2FaceMove");
}

void T_B2_CrazyAttack::End()
{
	m_gameState->DestroyBobbingProjectiles();
}

void T_B2_CrazyAttack::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		int currentTaskBeats = RoundDownToInt(m_gameState->GetCurrentTaskBeats());
		if (currentTaskBeats == 3)
		{
			m_gameState->GetBoss2()->Teleport(GetWorldScreenDimensions() * .5f);
			EntityConfig projectileConfig = EntityConfig::GetEntityConfigByName("MediumCircleBullet");
			m_gameState->SpawnProjectilesInCircle(GetWorldScreenDimensions() * .5f, 15, projectileConfig);
			SpawnBobbingProjectiles();
			g_theGame->ShakeScreen(.25f, 2.f);
		}
		if (currentTaskBeats > 3 && currentTaskBeats < 27)
		{
			EntityConfig projectileConfig = EntityConfig::GetEntityConfigByName("CircleBullet");
			m_gameState->SpawnProjectilesInCircle(GetWorldScreenDimensions() * .5f, 12, projectileConfig, 32.f, 32.f, g_randGen->RollRandomFloatInRange(0.f, 180.f));
		}
		if (currentTaskBeats == 16)
		{
			SpawnArms();
		}
	}
}



void T_B2_CrazyAttack::SpawnBobbingProjectiles()
{
	AABB2 cameraBounds(g_theGame->m_worldCamera.GetOrthoBottomLeft(), g_theGame->m_worldCamera.GetOrthoTopRight());
	int numBobbingProjectiles = 64;
	float timeDurationBetweenProjectiles = m_gameState->m_beatTime * 8 / (float)numBobbingProjectiles;
	for (int i = 0; i < numBobbingProjectiles; i++)
	{
		Vec2 topBobbingPos;
		topBobbingPos.x = RangeMap((float)i / (float)(numBobbingProjectiles - 1), 0.f, 1.f, cameraBounds.m_mins.x, cameraBounds.m_maxs.x);
		topBobbingPos.y = cameraBounds.m_maxs.y - 12;
		EntityConfig config = EntityConfig::GetEntityConfigByName("CircleBullet");
		config.m_hideTime = (float)i * timeDurationBetweenProjectiles;
		m_gameState->AddEntity(new BobbingProjectile(m_gameState, EntityType::BOBBING_PROJECTILE, topBobbingPos, config));

		Vec2 bottomBobbingPos;
		bottomBobbingPos.x = RangeMap((float)i / (float)(numBobbingProjectiles - 1), 0.f, 1.f, cameraBounds.m_maxs.x, cameraBounds.m_mins.x);
		bottomBobbingPos.y = cameraBounds.m_mins.y + 12;
		m_gameState->AddEntity(new BobbingProjectile(m_gameState, EntityType::BOBBING_PROJECTILE, bottomBobbingPos, config));
	}
}

void T_B2_CrazyAttack::SpawnArms()
{
	float thetaStep = 360.f / 5.f;
	for (int i = 0; i < 5; i++)
	{

		ProjectileConfig projectileConfig;

		projectileConfig.m_startOrientaiton = (float)i * thetaStep;
		projectileConfig.m_rotationSpeed = 20.f;
		Vec2 boxDimensions(15.f, .2f);
		projectileConfig.m_liveTime = m_gameState->m_beatTime * 12.f;
		projectileConfig.m_becomeHazardTime = .5f;
		projectileConfig.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(16.f * boxDimensions.x, 16.f * boxDimensions.y));
		projectileConfig.m_normalizedPivot = Vec2(0.f, .5f);
		projectileConfig.m_animation = "SpawnBox";
		Vec2 boxPosition = m_gameState->GetBoss2()->GetPosition();
		BoxHazard* arm = new BoxHazard(m_gameState, EntityType::ENEMY_PROJECTILE, boxPosition, projectileConfig);
		arm->m_sortOrder = 8;
		m_gameState->AddEntity(arm);
	}
}

T2_B2_End::T2_B2_End(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
}

void T2_B2_End::Begin()
{
	m_gameState->GetBoss2()->PlayDevilAnimation();
	m_gameState->GetBoss2()->PlayAnimation("TeleportInBig");
	EntityConfig bulletConfig = EntityConfig::GetEntityConfigByName("MediumCircleBullet");
	bulletConfig.m_sortOrder = 5;
	m_gameState->SpawnProjectilesInSpiral(m_gameState->GetBoss2()->GetPosition(), 80, m_gameState->m_beatTime / 20.f, bulletConfig, 32.f, 48.f, true, 4);
	g_theGame->ShakeScreen(m_gameState->m_beatTime  * 6.f, 1.f);
}

void T2_B2_End::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		int currentTaskBeats = RoundDownToInt(m_gameState->GetCurrentTaskBeats());
		if (currentTaskBeats == 8)
		{
			m_gameState->GetBoss2()->Teleport(GetWorldScreenDimensions() * .5f, true);
			EntityConfig config = EntityConfig::GetEntityConfigByName("Scythe");
			m_gameState->SpawnScytheAtPos(config, EntityType::SCYTHE, m_gameState->GetBoss2()->GetPosition() + Vec2::RIGHT * 36);
		}
		if (currentTaskBeats == 12)
		{
			SpawnDevilClones();
		}
		if (currentTaskBeats > 12 && currentTaskBeats < 24)
		{
			m_gameState->SpawnRandomBarHazard(true);
		}
		if (currentTaskBeats >= 22 && currentTaskBeats < 44 && currentTaskBeats % 4 == 2)
		{
			m_nextTeleportPos = g_theGame->GetRandomPosInWorldScreen(Vec2(.2f, .2f));
			EntityConfig config = EntityConfig::GetEntityConfigByName("TeleportHazardIndicator");
			m_gameState->SpawnEntityAtPos(config, EntityType::PLAYER_VFX, m_nextTeleportPos);
		}

		if (currentTaskBeats >= 24 && currentTaskBeats < 46 && currentTaskBeats % 4 == 0)
		{
			SpawnDevilSpiralAtPos(m_nextTeleportPos);
		}
		if (currentTaskBeats == 45)
		{
			EntityConfig config = EntityConfig::GetEntityConfigByName("TeleportHazardIndicator");
			config.m_spriteBounds.m_maxs *= 1.5f;
			m_gameState->SpawnEntityAtPos(config, EntityType::PLAYER_VFX, GetWorldScreenDimensions() * .5f);
		}
		if (currentTaskBeats == 48)
		{
			g_theGame->ShakeScreen(.25f, 2.f);
			m_gameState->BeatDropKillAllScythes();
			m_gameState->GetBoss2()->Teleport(GetWorldScreenDimensions() * .5f, false);
			m_gameState->GetBoss2()->PlayAnimation("TeleportInBig");
			EntityConfig bulletConfig = EntityConfig::GetEntityConfigByName("MediumCircleBullet");
			bulletConfig.m_sortOrder = 5;
			m_gameState->SpawnProjectilesInSpiral(GetWorldScreenDimensions() * .5f, 40, m_gameState->m_beatTime / 20.f, bulletConfig, 32.f, 48.f, true, 2);
		}
		if (currentTaskBeats == 51)
		{
			m_gameState->GetBoss2()->PlayAnimation("FadeOut");
			m_gameState->GetBoss2()->m_isHazard = false;
			for (int i = 0; i < 4; i++)
			{
				EntityConfig wispConfig = EntityConfig::GetEntityConfigByName("Wisp");
				Wisp* wisp = new Wisp(m_gameState, EntityType::WISP, GetWorldScreenDimensions() * .5f + g_randGen->RollRandomNormalizedVec2()*32.f, wispConfig);
				wisp->m_accelerationLength = 14.f;
				wisp->m_targetPos = g_theGame->GetRandomPosOffScreen(64.f);
				m_gameState->AddEntity(wisp);
			}
		}
	}
}

void T2_B2_End::SpawnDevilClones()
{
	Vec2 screenDimensions = GetWorldScreenDimensions();
	for (int i = 0; i < 6; i++)
	{
		Vec2 spawnPos = Vec2(GetWorldScreenDimensions().x - 96 * i - 32.f, screenDimensions.y * .8f);
		EntityConfig scytheConfig = EntityConfig::GetEntityConfigByName("Scythe");
		scytheConfig.m_hideTime = (float)i * m_gameState->m_beatTime * .5f;
		scytheConfig.m_liveTime = 5.f;
		Scythe* scythe = new Scythe(m_gameState, EntityType::SCYTHE, spawnPos, scytheConfig);
		scythe->m_goDown = true;
		scythe->m_delayTimer.m_period = (5.f - (float)i) * m_gameState->m_beatTime * .5f;
		scythe->m_maxSpeed = 256.f;
		m_gameState->AddEntity(scythe);

		spawnPos.x -= 48.f;
		EntityConfig devilConfig = EntityConfig::GetEntityConfigByName("DevilClone");
		devilConfig.m_hideTime = (float)i * m_gameState->m_beatTime * .5f;
		devilConfig.m_liveTime = 1.f;
		Entity* devil = new Entity(m_gameState, EntityType::PLAYER_VFX, spawnPos, devilConfig);
		devil->AddChildEntity(new Entity(m_gameState, EntityType::PLAYER_VFX, Vec2(0, 6), EntityConfig::GetEntityConfigByName("Boss2Hair")));
		devil->AddChildEntity(new Entity(m_gameState, EntityType::PLAYER_VFX, Vec2(0, 0), EntityConfig::GetEntityConfigByName("Boss2Face")));
		devil->m_children[1]->PlayAnimation("DevilFace");
		m_gameState->AddEntity(devil);
	}
}

void T2_B2_End::SpawnDevilSpiralAtPos(Vec2 pos)
{
	m_gameState->GetBoss2()->Teleport(pos, true);
	EntityConfig config = EntityConfig::GetEntityConfigByName("Scythe");
	m_gameState->SpawnScytheAtPos(config, EntityType::SCYTHE, m_gameState->GetBoss2()->GetPosition() + Vec2::RIGHT * 36);
	EntityConfig bulletConfig = EntityConfig::GetEntityConfigByName("CircleBullet");
	m_gameState->SpawnProjectilesInSpiral(m_gameState->GetBoss2()->GetPosition(), 20, m_gameState->m_beatTime / 20.f, bulletConfig, 32.f, 48.f, true, 1);
}

T_B3_Intro::T_B3_Intro(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
	m_twoThridBeatTimer.m_period = gameState->m_beatTime * .667f;
	m_thirdBeatTimer.m_period = gameState->m_beatTime * .333f;
	m_quarterBeatTimer.m_period = gameState->m_beatTime * .25f;
}

void T_B3_Intro::Begin()
{
}

void T_B3_Intro::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{

		int currentTaskBeats = RoundDownToInt(m_gameState->GetCurrentTaskBeats());		
		if (currentTaskBeats >= 5 &&  currentTaskBeats <= 13 && currentTaskBeats % 2 == 1)
		{
			m_gameState->GetBoss3()->Wiggle();
		}
		if (currentTaskBeats > 13 && currentTaskBeats < 19)
		{
			m_gameState->GetBoss3()->Wiggle();
		}
	}
}

T_B3_CryFast::T_B3_CryFast(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
	m_twoThridBeatTimer.m_period = gameState->m_beatTime * .6667f;
}

void T_B3_CryFast::Begin()
{
	m_gameState->GetBoss3()->Wiggle();
	m_twoThridBeatTimer.Start();
}

void T_B3_CryFast::Tick()
{
	if (m_twoThridBeatTimer.HasPeriodElapsed())
	{
		m_twoThridBeatTimer.Start();
		m_gameState->GetBoss3()->Wiggle();
	}

}

T_B3_CryFaster::T_B3_CryFaster(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
	m_thirdBeatTimer.m_period = gameState->m_beatTime * .333f;
}

void T_B3_CryFaster::Begin()
{
	m_thirdBeatTimer.Start();
	m_gameState->GetBoss3()->Wiggle();
	m_gameState->GetBoss3()->AddArm();
}

void T_B3_CryFaster::Tick()
{
	if (m_thirdBeatTimer.HasPeriodElapsed())
	{
		m_thirdBeatTimer.Start();
		m_gameState->GetBoss3()->WiggleFast();
	}
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		int currentTaskBeats = RoundDownToInt(m_gameState->GetCurrentTaskBeats());
		if (currentTaskBeats == 6 || currentTaskBeats == 12)
		{
			m_gameState->GetBoss3()->AddArm();
		}
		if (currentTaskBeats == 30)
		{
			m_gameState->GetBoss3()->StartSpeedUpArms();
		}
	}
}

T_B3_CryReallyFast::T_B3_CryReallyFast(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
	m_quarterBeatTimer.m_period = gameState->m_beatTime * .25f;
}

void T_B3_CryReallyFast::Begin()
{
	m_gameState->GetBoss3()->WiggleFast();
	m_quarterBeatTimer.Start();
	m_gameState->GetBoss3()->m_wiggleScale *= 1.5f;
	m_gameState->GetBoss3()->m_wiggleDeltaDegrees = 15.f;
}

void T_B3_CryReallyFast::Tick()
{
	if (m_quarterBeatTimer.HasPeriodElapsed())
	{
		m_quarterBeatTimer.Start();
		m_gameState->GetBoss3()->WiggleFast();
	}
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		int currentTaskBeats = RoundDownToInt(m_gameState->GetCurrentTaskBeats());

		if (currentTaskBeats >= 24 && currentTaskBeats < 50 && currentTaskBeats % 2 == 0)
		{
			Vec2 riseUpBulletSpawnPos = m_gameState->GetRandomPosBelowScreen();
			EntityConfig riseUpBulletConfig = EntityConfig::GetEntityConfigByName("SpikeBulletRiseUp");
			riseUpBulletConfig.m_startVelocity = Vec2::UP * g_randGen->RollRandomFloatInRange(12.f, 32.f);
			riseUpBulletConfig.m_startAcceleration = Vec2::UP * g_randGen->RollRandomFloatInRange(48.f, 72.f);
			m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, riseUpBulletSpawnPos, riseUpBulletConfig));
		}
		if (currentTaskBeats == 50)
		{
			m_gameState->GetBoss3()->RemoveArms();
		}
	}
}


T_B3_GearSpinAttack::T_B3_GearSpinAttack(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
}

void T_B3_GearSpinAttack::Begin()
{
	m_gameState->GetBoss3()->PlayAnimation("RotateAway");
}

void T_B3_GearSpinAttack::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		int currentTaskBeats = RoundDownToInt(m_gameState->GetCurrentTaskBeats());
		if (currentTaskBeats > 0 && currentTaskBeats <= 3)
		{
			g_theGame->ShakeScreen(.2f, 1.25f);
		}
		if (currentTaskBeats == 3)
		{
			EntityConfig circleParticleConfig = EntityConfig::GetEntityConfigByName("BigCircleParticle");
			m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, m_gameState->GetBoss3()->GetPosition(), circleParticleConfig));
		}
		if (currentTaskBeats == 4)
		{
			m_gameState->GetBoss3()->m_hidden = true;
			EntityConfig gearConfig = EntityConfig::GetEntityConfigByName("BigGear");
			m_gameState->AddEntity(new BigGear(m_gameState, EntityType::ENEMY_PROJECTILE, m_gameState->GetBoss3()->GetPosition(), gearConfig));
		}
	}
}

T_B3_SawAttack::T_B3_SawAttack(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
}

void T_B3_SawAttack::Begin()
{
	m_gameState->GetBoss3()->PlayAnimation("Boss3ShootSaws");
	EntityConfig sawBladeConfig = EntityConfig::GetEntityConfigByName("SawBlade");
	m_gameState->AddEntity(new SawBlade(m_gameState, EntityType::ENEMY_PROJECTILE, m_gameState->GetBoss3()->GetPosition() + Vec2::LEFT * 4.f, sawBladeConfig, Vec2::LEFT));
	m_gameState->AddEntity(new SawBlade(m_gameState, EntityType::ENEMY_PROJECTILE, m_gameState->GetBoss3()->GetPosition() + Vec2::RIGHT * 4.f, sawBladeConfig, Vec2::RIGHT));
	m_gameState->AddEntity(new SawBlade(m_gameState, EntityType::ENEMY_PROJECTILE, m_gameState->GetBoss3()->GetPosition() + Vec2::UP * 4.f, sawBladeConfig, Vec2::UP));
	m_gameState->AddEntity(new SawBlade(m_gameState, EntityType::ENEMY_PROJECTILE, m_gameState->GetBoss3()->GetPosition() + Vec2::DOWN * 4.f, sawBladeConfig, Vec2::DOWN));
}

void T_B3_SawAttack::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		int currentTaskBeats = RoundDownToInt(m_gameState->GetCurrentTaskBeats());
		if (currentTaskBeats == 2)
		{
			m_gameState->GetBoss3()->Shake();
		}
	}
}


T_B3_Falldown::T_B3_Falldown(GameStateLevel* gameState, float numBeats, bool addArms, int beatToAddArms)
	: Task(gameState, numBeats)
	, m_addArms(addArms)
	, m_beatToAddArms(beatToAddArms)
{
}

void T_B3_Falldown::Begin()
{
	m_gameState->GetBoss3()->PlayAnimation("Boss3FallDown");
}

void T_B3_Falldown::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		int currentTaskBeats = RoundDownToInt(m_gameState->GetCurrentTaskBeats());
		if (currentTaskBeats == 2)
		{
			g_theGame->ShakeScreen(.2f, 2.0f);
		}
		if (currentTaskBeats == m_beatToAddArms && m_addArms)
		{
			m_gameState->AddArms(35.f);
		}
	}
}

T_B3_Arms::T_B3_Arms(GameStateLevel* gameState, float numBeats, bool startImmediately)
	: Task(gameState, numBeats)
	, m_startImmediately(startImmediately)
{
}

void T_B3_Arms::Begin()
{
	if (m_startImmediately)
	{
		m_gameState->GetBoss3()->ArmShake();
		m_gameState->GetBoss3()->PlayAnimation("Boss3Shake");
	}
	else
	{
		for (int i = 0; i < 6; i++)
		{
			m_gameState->AddArms(35.f);
		}
	}
}

void T_B3_Arms::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		int currentTaskBeats = RoundDownToInt(m_gameState->GetCurrentTaskBeats());
		if (currentTaskBeats == 1 && !m_startImmediately)
		{
			m_gameState->GetBoss3()->ArmShake();
			m_gameState->GetBoss3()->PlayAnimation("Boss3Shake");
		}
	}
}

void T_B3_Arms::End()
{
	m_gameState->RemoveAllArms();
}

T_B3_FlyIn::T_B3_FlyIn(GameStateLevel* gameState, float numBeats, bool shrinkOut)
	: Task(gameState, numBeats)
	, m_shrink(shrinkOut)
{
	m_scaleTimer = Timer(m_gameState->m_beatTime * m_numBeats, g_theApp->GetGameClock());
}

void T_B3_FlyIn::Begin()
{
	m_gameState->GetBoss3()->m_hidden = false;
	m_gameState->GetBoss3()->m_isHazard = false;
	m_gameState->GetBoss3()->PlayAnimation("Boss3FlyIn");
	m_gameState->AddArms(35.f);
	m_scaleTimer.Start();
	if (m_shrink)
	{
		m_startBossSize = m_gameState->GetBoss3()->m_spriteBounds.m_maxs;
	}
}

void T_B3_FlyIn::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		int currentTaskBeats = RoundDownToInt(m_gameState->GetCurrentTaskBeats());
		if (currentTaskBeats == 2)
		{
			m_gameState->GetBoss3()->ArmShake(true, m_scaleTimer.m_period - m_gameState->m_beatTime * 2.f);
		}
	}
	if (m_scaleTimer.GetElapsedFraction() < .4f)
	{
		m_gameState->GetBoss3()->m_scale = Vec2::Lerp(Vec2::ONE, Vec2(2.f, 2.f), GetFractionWithinRange(m_scaleTimer.GetElapsedFraction(), .0f, .4f));

	}
	else if (m_scaleTimer.GetElapsedFraction() < .8f)
	{
		m_gameState->GetBoss3()->m_scale = Vec2::Lerp(Vec2(2.f, 2.f), Vec2::ONE, GetFractionWithinRange(m_scaleTimer.GetElapsedFraction(), .4f, .8f));
	}
	else if (m_shrink && m_scaleTimer.GetElapsedFraction() > .8f)
	{
		m_gameState->GetBoss3()->m_timesToChangeFace = 0;
		m_gameState->GetBoss3()->m_isHazard = false;
		m_gameState->GetBoss3()->m_scale = Vec2::Lerp(Vec2::ONE, Vec2(.5f, .5f), GetFractionWithinRange(m_scaleTimer.GetElapsedFraction(), .8f, 1.f));
	}
}

void T_B3_FlyIn::End()
{
	m_gameState->RemoveAllArms();
}

T_B3_HeartbeatAttack::T_B3_HeartbeatAttack(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
}

void T_B3_HeartbeatAttack::Begin()
{
	m_gameState->SpawnHeartbeatIndicator();
}

void T_B3_HeartbeatAttack::Tick()
{
	int currentBeatNumber;
	if (m_gameState->IsBeatThisFrame(currentBeatNumber))
	{
		int currentTaskBeats = RoundDownToInt(m_gameState->GetCurrentTaskBeats());
		if (currentTaskBeats == 1)
		{
			EntityConfig heartConfig = EntityConfig::GetEntityConfigByName("Heart");
			m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, m_gameState->GetBoss3()->GetPosition(), heartConfig));
		}
		if (currentTaskBeats == 2)
		{
			EntityConfig circleConfig = EntityConfig::GetEntityConfigByName("CircleBullet");
			circleConfig.m_startAnimation = "";
			circleConfig.m_liveTime = .48f;
			circleConfig.m_sortOrder = 10;
			m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, m_gameState->GetBoss3()->GetPosition() + Vec2::LEFT * 138.f, circleConfig));
		}
		if (currentTaskBeats == 2)
		{
			EntityConfig circleConfig = EntityConfig::GetEntityConfigByName("CircleBullet");
			circleConfig.m_startAnimation = "";
			circleConfig.m_liveTime = .48f;
			circleConfig.m_sortOrder = 10;
			circleConfig.m_spriteBounds = AABB2( Vec2::ZERO, Vec2(16.f, 16.f) );
			m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, m_gameState->GetBoss3()->GetPosition() + Vec2::LEFT * 138.f, circleConfig));
		}
		if (currentTaskBeats == 3)
		{
			EntityConfig circleConfig = EntityConfig::GetEntityConfigByName("CircleBullet");
			circleConfig.m_startAnimation = "";
			circleConfig.m_liveTime = .96f;
			circleConfig.m_sortOrder = 10;
			circleConfig.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(24.f, 24.f));
			m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, m_gameState->GetBoss3()->GetPosition() + Vec2::LEFT * 138.f, circleConfig));
		}
		if (currentTaskBeats == 4)
		{
			EntityConfig bulletConfig = EntityConfig::GetEntityConfigByName("CircleBullet");
			m_gameState->SpawnProjectilesInSpiral(m_gameState->GetBoss3()->GetPosition() + Vec2::LEFT * 138.f, 48, .02f, bulletConfig, 32.f, 96.f, true, 3);
		}
	}
}

T_SadBoySequence::T_SadBoySequence(GameStateLevel* gameState, float numBeats)
	: Task(gameState, numBeats)
{
}

void T_SadBoySequence::Begin()
{
	Vec2 sawPos = Vec2(g_theGame->m_worldCamera.GetOrthoTopRight().x - 52.f, g_theGame->m_worldCamera.GetOrthoBottomLeft().y + 26.f);
	EntityConfig slowSawConfig = EntityConfig::GetEntityConfigByName("SlowSawBlade");
	m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, sawPos, slowSawConfig));
	sawPos.x -= 52.f;
	m_gameState->AddEntity(new Entity(m_gameState, EntityType::ENEMY_PROJECTILE, sawPos, slowSawConfig));
	m_gameState->GetBoss3()->PlayAnimation("SadBoySequence");
}

void T_SadBoySequence::Tick()
{
}
