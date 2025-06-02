#include "Actor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/AIController.hpp"
#include "Game/Map.hpp"
#include "Game/Weapon.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Game/BinkeyController.hpp"

Actor::Actor(SpawnInfo const& spawnInfo, ActorUID const& actorUID, Map* map)
	: m_actorUID(actorUID)
	, m_actorDefinition(spawnInfo.m_actorDefinition)
	, m_map(map)
	, m_lifeTimer( Timer(m_actorDefinition->m_lifetime, g_theApp->GetGameClock()) )
{
	m_position = spawnInfo.m_position;
	m_orientation = spawnInfo.m_orientation;
	m_velocity = spawnInfo.m_velocity;
	m_currentHealth = m_actorDefinition->m_health;
	m_physicsCylinderHeight = m_actorDefinition->m_collisionHeight;
	m_physicsCylinderRadius = m_actorDefinition->m_collisionRadius;
	m_currentFaction = m_actorDefinition->m_faction;
	for (size_t i = 0; i < m_actorDefinition->m_inventory.size(); i++)
	{
		m_weapons.push_back(new Weapon(m_actorDefinition->m_inventory[i], m_actorUID));
	}

	if (m_actorDefinition->m_aiEnabled)
	{
		AIController* aiController = new AIController(m_map);
		aiController->Possess(m_actorUID);
		OnPossessed(aiController);
	}
	else if (m_actorDefinition->m_binkeyAI)
	{
		BinkeyController* binkeyController = new BinkeyController(m_map);
		binkeyController->Possess(m_actorUID);
		OnPossessed(binkeyController);
	}
	m_animationClock = new Clock(*g_theApp->GetGameClock());
	if (m_actorDefinition->m_visible)
	{
		m_currAnimStateName = m_actorDefinition->m_animationGroups[0].m_name;
	}
	if (m_lifeTimer.m_period >= 0.f)
	{
		m_lifeTimer.Start();
	}
	if (m_actorDefinition->m_dieOnSpawn)
	{
		KillActor();
	}
	if (m_actorDefinition->m_spawnPointLight)
	{
		PointLight pointLight;
		pointLight.intensity = 40.f;
		pointLight.linearAttenuation = 200.f;
		pointLight.exponentialAttenuation = 200.f;
		pointLight.position = m_position + Vec3(0.f, 0.f, .2f);
		pointLight.pointLightColor.x = 245.f / 255.f;
		pointLight.pointLightColor.y = 116.f / 255.f;
		pointLight.pointLightColor.z = 69.f / 255.f;
		pointLight.pointLightColor.w = 1.f;
		g_theGame->m_currentMap->AddPointLight(pointLight);
	}
}

Actor::~Actor()
{
	for (size_t i = 0; i < m_weapons.size(); i++)
	{
		delete m_weapons[i];
	}
	delete m_animationClock;
}

void Actor::Update(float deltaSeconds)
{
	if (m_isDead)
	{
		m_currentDeadTime +=  deltaSeconds;
		if (m_currentDeadTime > m_actorDefinition->m_corpseLifetime)
		{
			if (m_actorDefinition->m_binkeyAI)
			{
				Task9_BinkeyRap* rapTask = dynamic_cast<Task9_BinkeyRap*>( g_theGame->GetTaskByName("Task9_BinkeyRap") );
				if (rapTask != nullptr && rapTask->m_rapStarted)
				{
					rapTask->m_rapTimer.Stop();
				}
				m_map->RespawnBinkey(this);
			}
			else
			{
				m_isGarbage = true;
			}
		}
		return;
	}
	else
	{
		if (m_controller != nullptr && dynamic_cast<Player*>(m_controller) == nullptr)
		{
			m_controller->Update(deltaSeconds);
		}
		UpdatePhysics(deltaSeconds);
		UpdateAnimation(deltaSeconds);
		if (m_lifeTimer.HasPeriodElapsed())
		{
			KillActor();
		}
	}
}

void Actor::Render()
{
	if (!m_actorDefinition->m_visible)
	{
		return;
	}

	if (g_theGame->GetCurrentlyRenderingPlayer()->GetActor() == this && !g_theGame->GetCurrentlyRenderingPlayer()->m_freeFlyCameraMode)
	{
		return;
	}
	Mat44 targetMatrix = g_theGame->GetCurrentlyRenderingPlayer()->m_worldCamera.GetWorldTransform();
	Mat44 billboardMatrix = GetBillboardMatrix(m_actorDefinition->m_billboardType, targetMatrix, m_position);
	g_theRenderer->SetModelConstants(billboardMatrix, m_actorDefinition->m_tint);
	if (m_actorDefinition->m_depthDisabled)
	{
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	}
	else
	{
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	}
	g_theRenderer->SetBlendMode(m_actorDefinition->m_blendMode);
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindShader(m_actorDefinition->m_shader);

	SpriteDefinition currAnimFrame = GetSpriteDefForCurrentAnimation();
	g_theRenderer->BindTexture(currAnimFrame.GetTexture());
	if (m_actorDefinition->m_renderRounded)
	{
		std::vector<Vertex_PCUTBN> actorVerts;
		AddVertsForRoundedActorQuad(actorVerts, currAnimFrame.GetUVs());
		g_theRenderer->DrawVertexArray(actorVerts.size(), actorVerts.data());
	}
	else
	{
		std::vector<Vertex_PCU> actorVerts;
		AddVertsForActorQuad(actorVerts, currAnimFrame.GetUVs());
		g_theRenderer->DrawVertexArray(actorVerts.size(), actorVerts.data());
	}
}

void Actor::OnPossessed(Controller* controllerPossessing)
{
	if (m_controller != nullptr)
	{
		AIController* previousAIController = dynamic_cast<AIController*>(m_controller);
		if (previousAIController != nullptr)
		{
			m_previousAIController = previousAIController;
		}
	}
	m_controller = controllerPossessing;
}

void Actor::OnUnpossessed()
{
	if (m_previousAIController != nullptr)
	{
		m_controller = m_previousAIController;
	}
	else
	{
		m_controller = nullptr;
	}
}

void Actor::OnCollide(Actor* collidingActor)
{
	if (collidingActor->IsPlayer() && m_actorDefinition->m_pickupWeapon != "")
	{
		if (m_actorDefinition->m_pickupWeapon == "Flamethrower")
		{
			g_dialogSystem->StartNewDialogue("flamethrower");
		}
		else if (m_actorDefinition->m_pickupWeapon == "Shrinkray")
		{
			Task11_ShrinkRayEnd* shrinkRayTask = dynamic_cast<Task11_ShrinkRayEnd*>(g_theGame->GetTaskByName("Task11_ShrinkRayEnd"));
			if (shrinkRayTask != nullptr)
			{
				shrinkRayTask->m_shrinkRayEquipped = true;
			}
		}
		collidingActor->m_weapons.push_back( new Weapon(WeaponDefinition::GetByName(m_actorDefinition->m_pickupWeapon), collidingActor->GetActorUID()) );
		g_theAudio->StartSound(SOUND_ID_WEAPON_PICKUP);
		DestroyActor();
		return;
	}
	if (m_actorDefinition->m_dieOnCollide)
	{
		KillActor();
		return;
	}

	float damage = g_randGen->RollRandomFloatInRange(collidingActor->m_actorDefinition->m_damageOnCollide);
	if (damage > 0.f)
	{
		TakeDamage(damage, collidingActor->m_owningActor);
	}
	if (collidingActor->m_velocity.GetLength() > 0.001f)
	{
		AddImpulse(collidingActor->m_actorDefinition->m_impulseOnCollide * collidingActor->m_velocity.GetNormalized());
	}
}

bool Actor::IsPlayer()
{
	return m_controller != nullptr && m_controller->IsPlayer();
}


Mat44 Actor::GetModelMatrix()
{
	Mat44 modelMatrix;
	modelMatrix.AppendTranslation3D(m_position);
	modelMatrix.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
	return modelMatrix;
}

Mat44 Actor::GetModelMatrixZUp()
{
	Mat44 modelMatrix;
	modelMatrix.AppendTranslation3D(m_position);
	modelMatrix.Append(EulerAngles(m_orientation.m_yaw, 0.f, 0.f).GetAsMatrix_IFwd_JLeft_KUp());
	return modelMatrix;
}

RaycastResultDoomenstein Actor::RaycastVsActor(Vec3 const& startPos, Vec3 const& rayFwdNormal, float rayDistance)
{
	RaycastResult3D result = RaycastVsCylinderZ3D(startPos, rayFwdNormal, rayDistance, GetPhysicsCylinderCenter(), m_physicsCylinderRadius, GetPhysicsCylinderZMinMax());
	RaycastResultDoomenstein resultWithActor;
	if (result.m_didImpact)
	{
		resultWithActor.m_actorHit = GetActorUID();
	}
	resultWithActor.m_didImpact = result.m_didImpact;
	resultWithActor.m_impactDist = result.m_impactDist;
	resultWithActor.m_impactNormal = result.m_impactNormal;
	resultWithActor.m_impactPos = result.m_impactPos;
	resultWithActor.m_rayFwdNormal = result.m_rayFwdNormal;
	resultWithActor.m_rayMaxLength = result.m_rayMaxLength;
	resultWithActor.m_rayStartPos = result.m_rayStartPos;
	return resultWithActor;
}

FloatRange Actor::GetPhysicsCylinderZMinMax()
{
	return FloatRange(m_position.z, m_position.z + m_physicsCylinderHeight);
}

Vec2 Actor::GetPhysicsCylinderCenter()
{
	return Vec2(m_position.x, m_position.y);
}

ActorUID Actor::GetActorUID()
{
	return m_actorUID;
}

void Actor::SetActorUID(ActorUID actorUID)
{
	m_actorUID = actorUID;
}

void Actor::UpdatePhysics(float deltaSeconds)
{
	if (!m_actorDefinition->m_physicsSimulated)
	{
		return;
	}
	AddForce(-m_velocity * m_actorDefinition->m_drag);
	m_velocity += m_acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
	if (!m_actorDefinition->m_isFlying)
	{
		m_position.z = 0.f;
	}
	m_acceleration = Vec3::ZERO;
}

void Actor::AddForce(Vec3 force)
{
	m_acceleration += force;
}

void Actor::AddImpulse(Vec3 impulse)
{
	m_velocity += impulse;
}

void Actor::MoveInDirection(Vec3 direction, float speed)
{
	AddForce(direction.GetNormalized() * speed * m_actorDefinition->m_drag);
}

void Actor::TakeDamage(float damage, ActorUID damageSource)
{
	if (!m_actorDefinition->m_damageable)
	{
		return;
	}

	TransitionAnimationState("Hurt");
	m_currentHealth -= damage;
	if (damageSource != ActorUID::INVALID && m_controller != nullptr)
	{
		m_controller->OnDamagedBy(damageSource);
	}
	if (m_currentHealth <= 0)
	{
		KillActor();
		if (m_controller != nullptr)
		{
			m_controller->OnKilledBy(damageSource);
		}
	}
	else if (m_actorDefinition->m_hurtSound != MISSING_SOUND_ID)
	{
		 g_theAudio->StartSoundAt(m_actorDefinition->m_hurtSound, m_position);
	}
}

void Actor::TurnTorwardsDirection(Vec2 const& directionXY)
{
	float targetTheta = directionXY.GetOrientationDegrees();
	m_orientation.m_yaw = GetTurnedTowardDegrees(m_orientation.m_yaw, targetTheta, m_actorDefinition->m_turnSpeed * g_theApp->GetGameClock()->GetDeltaSeconds());

}

Vec3 Actor::GetForwardZUp()
{
	Vec2 iFwdXY = m_orientation.GetIFwd().GetXY();
	return iFwdXY.GetXYZ().GetNormalized();
}

bool Actor::IsOpposingFaction(Actor* otherActor)
{
	return m_currentFaction != otherActor->m_currentFaction && m_currentFaction != Faction::NEUTRAL && otherActor->m_currentFaction != Faction::NEUTRAL;
}

void Actor::Attack()
{
	if (GetCurrentWeapon() == nullptr)
	{
		return;
	}
	Vec3 firePos = m_position + Vec3(0.f, 0.f, m_actorDefinition->m_eyeHeight);
	firePos += m_orientation.GetIFwd() * m_physicsCylinderRadius;
	if (m_weapons[m_currentWeaponIdx]->Fire(firePos, m_orientation.GetIFwd()))
	{
		TransitionAnimationState("Attack");
	}
}

bool Actor::IsAlive() const
{
	return !m_isDead;
}

void Actor::KillActor()
{
	if (m_isDead)
	{
		return;
	}
	if (m_actorDefinition->m_spawnActorOnFloor != "" && (m_position.z < .1f || g_randGen->RollRandomFloatZeroToOne() > .5f) )
	{
		ActorDefinition const* spawnActorDef = ActorDefinition::GetByName(m_actorDefinition->m_spawnActorOnFloor);
		SpawnInfo spawnActorInfo;
		spawnActorInfo.m_position = m_position;
		spawnActorInfo.m_position.z = 0.f;
		spawnActorInfo.m_actorDefinition = spawnActorDef;
		m_map->SpawnActor(spawnActorInfo);
	}


	if (m_actorDefinition->m_deathSound != MISSING_SOUND_ID)
	{
		g_theAudio->StartSoundAt(m_actorDefinition->m_deathSound, m_position);
	}

	m_isDead = true;
	TransitionAnimationState("Death");
}

void Actor::DestroyActor()
{
	m_isDead = true;
	m_isGarbage = true;
}

bool Actor::IsInSightline(ActorUID const& targetActor)
{	
	Vec3 targetPos = m_map->GetActorByUID(targetActor)->m_position;
	Vec3 dispToTargetPos = targetPos - m_position;
	if (dispToTargetPos.GetLength() > m_actorDefinition->m_sightRadius)
	{
		return false;
	}
	Vec2 dispToTargetXY = dispToTargetPos.GetXY().GetNormalized();
	Vec2 fwdXY = GetForwardZUp().GetXY();
	float angleBetween = fabsf( GetShortestAngularDispDegrees(fwdXY.GetOrientationDegrees(), dispToTargetXY.GetOrientationDegrees()) );
	if (angleBetween > m_actorDefinition->m_sightAngle)
	{
		return false;
	}
	Vec3 raystartPos = m_position + Vec3(0.f, 0.f, m_map->GetActorByUID(targetActor)->m_actorDefinition->m_eyeHeight);
	RaycastResultDoomenstein result = m_map->RaycastVsMap(raystartPos, dispToTargetPos.GetNormalized(), m_actorDefinition->m_sightRadius, m_actorUID);
	if (result.m_actorHit != targetActor) 
	{
		return false;
	}
	return true;
}

void Actor::EquipPreviousWeapon()
{
	m_currentWeaponIdx--;
	if (m_currentWeaponIdx < 0)
	{
		m_currentWeaponIdx = (int)m_weapons.size() - 1;
	}	
}

void Actor::EquipNextWeapon()
{
	m_currentWeaponIdx++;
	if (m_currentWeaponIdx > (int)m_weapons.size() - 1)
	{
		m_currentWeaponIdx = 0;
	}
}

Weapon* Actor::GetCurrentWeapon()
{
	if (m_weapons.size() == 0)
	{
		return nullptr;
	}
	return m_weapons[m_currentWeaponIdx];
}

void Actor::TransitionAnimationState(std::string newAnimState)
{
	m_currAnimStateName = newAnimState;
	m_animationClock->Reset();

	if (newAnimState == "Death")
	{
		m_animationClock->SetTimeScale(1.f);
	}
}

AnimationGroup Actor::GetCurrentAnimationState()
{
	for (size_t i = 0; i < m_actorDefinition->m_animationGroups.size(); i++)
	{
		if (m_currAnimStateName == m_actorDefinition->m_animationGroups[i].m_name)
		{
			return m_actorDefinition->m_animationGroups[i];
		}
	}
	ERROR_AND_DIE(Stringf("Could not find current animation state"));
}

void Actor::UpdateAnimation(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (!m_actorDefinition->m_visible)
	{
		return;
	}

	AnimationGroup currentAnimState = GetCurrentAnimationState();
	float animationDuration = (float)currentAnimState.m_numFrames * currentAnimState.m_secondsPerFrame;

	//go back to default walk state if single play animation finished
	if (m_animationClock->GetTotalSeconds() > animationDuration && currentAnimState.m_playbackMode == SpriteAnimPlaybackType::ONCE)
	{
		TransitionAnimationState("Walk");
	}

	if (GetCurrentAnimationState().m_scaleBySpeed)
	{
		m_animationClock->SetTimeScale(m_velocity.GetLength() / m_actorDefinition->m_walkSpeed);
	}
	else
	{
		m_animationClock->SetTimeScale(1.f);
	}
}

SpriteDefinition Actor::GetSpriteDefForCurrentAnimation()
{
	AnimationGroup currAnimGroup = GetCurrentAnimationState();
	Camera const& playerCamera = g_theGame->GetCurrentlyRenderingPlayer()->m_worldCamera;
	Vec3 dirFromCamera = m_position - playerCamera.m_position;
	dirFromCamera = Vec3(dirFromCamera.x, dirFromCamera.y, 0.f).GetNormalized();
	Mat44 worldToLocalMatrix = GetModelMatrix().GetOrthonormalInverse();
	Vec3 targetAnimationDirection = worldToLocalMatrix.TransformVectorQuantity3D(dirFromCamera);
	targetAnimationDirection = targetAnimationDirection.GetNormalized();

	float bestAnimDot = -1.f;
	int bestAnimIdx = 0;

	for (size_t i = 0; i < currAnimGroup.m_animationDirections.size(); i++)
	{
		float currAnimDirectionDot = DotProduct3D(currAnimGroup.m_animationDirections[i].m_directionVector, targetAnimationDirection);
		if (currAnimDirectionDot > bestAnimDot)
		{
			bestAnimDot = currAnimDirectionDot;
			bestAnimIdx = (int)i;
		}
	}

	return currAnimGroup.m_animationDirections[bestAnimIdx].m_animationDef.GetSpriteDefAtTime(m_animationClock->GetTotalSeconds());
}

void Actor::GetActorSpriteBounds(Vec3& out_billboardMins, Vec3& out_billboardMaxs)
{
	Vec2 actorSize = m_actorDefinition->m_billboardSize;
	Vec2 actorPivot = m_actorDefinition->m_billboardPivot;
	out_billboardMins = Vec3(0.f, Lerp(actorSize.x, 0.f, actorPivot.x), Lerp(-actorSize.y, 0.f, 1.f - actorPivot.y));
	out_billboardMaxs = Vec3(0.f, Lerp(0.f, -actorSize.x, actorPivot.x), Lerp(0.f, actorSize.y, 1.f - actorPivot.y));
	float minsY = out_billboardMaxs.y;
	out_billboardMaxs.y = out_billboardMins.y;
	out_billboardMins.y = minsY;
}

void Actor::AddVertsForRoundedActorQuad(std::vector<Vertex_PCUTBN>& verts, AABB2 uvs)
{
	//Vec2 actorSize = m_actorDefinition->m_billboardSize;
	//Vec2 actorPivot = m_actorDefinition->m_billboardPivot;
	//Vec3 pivotedMins(0.f, Lerp(actorSize.x, 0.f, actorPivot.x), Lerp(-actorSize.y, 0.f, 1.f - actorPivot.y));
	//Vec3 pivotedMaxs(0.f, Lerp(0.f, -actorSize.x, actorPivot.x), Lerp(0.f, actorSize.y, 1.f - actorPivot.y));
	//float minsY = pivotedMaxs.y;
	//pivotedMaxs.y = pivotedMins.y;
	//pivotedMins.y = minsY;
	Vec3 pivotedMins;
	Vec3 pivotedMaxs;
	GetActorSpriteBounds(pivotedMins, pivotedMaxs);
	AddVertsForRoundedQuad3D(verts, pivotedMins, Vec3(0.f, pivotedMaxs.y, pivotedMins.z), pivotedMaxs, Vec3(0.f, pivotedMins.y, pivotedMaxs.z), Rgba8::WHITE, uvs);
}

void Actor::AddVertsForActorQuad(std::vector<Vertex_PCU>& verts, AABB2 uvs)
{
	Vec3 pivotedMins;
	Vec3 pivotedMaxs;
	GetActorSpriteBounds(pivotedMins, pivotedMaxs);
	AddVertsForQuad3D(verts, pivotedMins, Vec3(0.f, pivotedMaxs.y, pivotedMins.z), pivotedMaxs, Vec3(0.f, pivotedMins.y, pivotedMaxs.z), Rgba8::WHITE, uvs);
}

