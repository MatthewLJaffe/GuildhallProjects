#include "Game/Weapon.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

Weapon::Weapon(WeaponDefinition const* def, ActorUID owningActor)
	: m_definition(def)
	, m_owningActor(owningActor)
	, m_soundPlayTimer(Timer(m_definition->m_soundPlayCooldown, g_theApp->GetGameClock()))
{
}

bool Weapon::Fire(Vec3 const& position, Vec3 const& direction)
{
	Clock const* gameClock = g_theApp->GetGameClock();
	float elapsedTime = gameClock->GetTotalSeconds() - m_timeSinceLastFire;
	if (m_timeSinceLastFire == 0.f || elapsedTime > m_definition->m_refireTime)
	{
		if (m_soundPlayTimer.IsStopped() || m_soundPlayTimer.HasPeriodElapsed())
		{
			m_soundPlayTimer.Start();
			m_currentSound = g_theAudio->StartSoundAt(m_definition->m_fireSound, position);
		}
		m_timeSinceLastFire = g_theApp->GetGameClock()->GetTotalSeconds();
		FireRays(position, direction);
		FireProjectiles(position, direction);
		FireMelees(position, direction);
		return true;
	}
	return false;
}

Vec3 Weapon::GetRandomDirectionInCone(Vec3 forwardDir, float coneDegrees)
{
	float spreadTheta = g_randGen->RollRandomFloatInRange(0.f, coneDegrees);
	float displacmentLength = TanDegrees(spreadTheta);
	Vec2 yzDisp = g_randGen->RollRandomNormalizedVec2() * displacmentLength;
	Vec3 displacedDirLocalSpace = Vec3(1.f, yzDisp.x, yzDisp.y);
	displacedDirLocalSpace.GetNormalized();

	Vec3 iBasis = forwardDir;
	Vec3 jBasis;
	if ( DotProduct3D(iBasis, Vec3(0.f, 0.f, 1.f)) < .999f )
	{
		jBasis = CrossProduct3D(forwardDir, Vec3(0.f, 0.f, 1.f));
	}
	else
	{
		jBasis = CrossProduct3D(forwardDir, Vec3(0.f, 1.f, 0.f));
	}
	Vec3 kBasis = CrossProduct3D(iBasis, jBasis);

	Mat44 forwardDirTransform;
	forwardDirTransform.SetIJK3D(iBasis, jBasis, kBasis);
	Vec3 displacedDirWorldSpace = forwardDirTransform.TransformVectorQuantity3D(displacedDirLocalSpace);
	return displacedDirWorldSpace;
}

SpriteDefinition const& Weapon::GetCurrentAnimationFrame()
{
	Clock const* gameClock = g_theApp->GetGameClock();
	float elapsedTime = gameClock->GetTotalSeconds() - m_timeSinceLastFire;
	if (m_definition->m_attackAnimation == nullptr)
	{
		return m_definition->m_idleAnimation->GetSpriteDefAtTime(gameClock->GetTotalSeconds());
	}
	else if (m_definition->m_attackAnimation->GetAnimDurationSeconds() > elapsedTime)
	{
		return m_definition->m_attackAnimation->GetSpriteDefAtTime(elapsedTime);
	}
	else
	{
		return m_definition->m_idleAnimation->GetSpriteDefAtTime(elapsedTime);
	}
}


void Weapon::FireRays(Vec3 const& position, Vec3 const& direction)
{
	for (int i = 0; i < m_definition->m_rayCount; i++)
	{
		Vec3 shootDirection = GetRandomDirectionInCone(direction, m_definition->m_rayCone);
		RaycastResultDoomenstein result = g_theGame->m_currentMap->RaycastVsMap(position, shootDirection, m_definition->m_rayRange, m_owningActor);
		if (result.m_didImpact)
		{
			if (result.m_actorHit != ActorUID::INVALID)
			{
				Actor* hitActor = g_theGame->m_currentMap->GetActorByUID(result.m_actorHit);
				float damage = g_randGen->RollRandomFloatInRange(m_definition->m_rayDamage.m_min, m_definition->m_rayDamage.m_max);
				hitActor->TakeDamage(damage, m_owningActor);
				if (!hitActor->m_actorDefinition->m_binkeyAI)
				{
					hitActor->AddImpulse(shootDirection * m_definition->m_rayImpulse);
				}

				SpawnInfo bloodSplatterEffect;
				bloodSplatterEffect.m_position = result.m_impactPos;
				bloodSplatterEffect.m_velocity = Vec3::ZERO;
				bloodSplatterEffect.m_actorDefinition = ActorDefinition::GetByName("BloodSplatter");
				g_theGame->m_currentMap->SpawnActor(bloodSplatterEffect);
			}
			else
			{
				SpawnInfo hitWallEffect;
				hitWallEffect.m_position = result.m_impactPos;
				hitWallEffect.m_velocity = Vec3::ZERO;
				hitWallEffect.m_actorDefinition = ActorDefinition::GetByName("BulletHit");
				g_theGame->m_currentMap->SpawnActor(hitWallEffect);
			}
		}
	}
}

void Weapon::FireProjectiles(Vec3 const& position, Vec3 const& direction)
{
	Actor* owningActor = GetOwningActorPtr();
	for (int i = 0; i < m_definition->m_projectileCount; i++)
	{
		SpawnInfo projectileSpawnInfo;
		projectileSpawnInfo.m_actorDefinition = m_definition->m_projectileActor;
		projectileSpawnInfo.m_position = position;
		projectileSpawnInfo.m_position -= Vec3(0.f, 0.f, projectileSpawnInfo.m_actorDefinition->m_collisionHeight);
		projectileSpawnInfo.m_position += direction * (projectileSpawnInfo.m_actorDefinition->m_collisionRadius + owningActor->m_physicsCylinderRadius);
		Vec3 forwardDirection = GetRandomDirectionInCone(direction, m_definition->m_projectileCone);
		projectileSpawnInfo.m_velocity = forwardDirection * m_definition->m_projectileSpeed;
		Actor* projectileActor = g_theGame->m_currentMap->SpawnActor(projectileSpawnInfo);
		projectileActor->m_currentFaction = GetOwningActorPtr()->m_currentFaction;
		projectileActor->m_owningActor = m_owningActor;
	}
}

void Weapon::FireMelees(Vec3 const& position, Vec3 const& direction)
{
	Actor* owningActor = GetOwningActorPtr();
	std::vector<Actor*> actorsInArc;
	owningActor->m_map->GetActorsInMeleeSwing(actorsInArc, position, direction.GetXY(), m_definition->m_meleeArc, m_definition->m_meleeRange);
	for (size_t i = 0; i < actorsInArc.size(); i++)
	{
		if (owningActor->IsOpposingFaction(actorsInArc[i]))
		{
			actorsInArc[i]->TakeDamage(g_randGen->RollRandomFloatInRange(m_definition->m_meleeDamage), m_owningActor);
			actorsInArc[i]->AddImpulse(m_definition->m_meleeImpulse * (actorsInArc[i]->m_position - owningActor->m_position).GetNormalized());
		}
	}
}

Actor* Weapon::GetOwningActorPtr()
{
	return g_theGame->m_currentMap->GetActorByUID(m_owningActor);
}
