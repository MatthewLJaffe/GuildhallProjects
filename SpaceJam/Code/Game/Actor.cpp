#include "Game/Actor.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"
#include "Game/GameScene.hpp"
#include "Game/PlayerShip.hpp"

Actor::Actor(SpawnInfo const& spawnInfo, ActorUID actorUID)
	: m_definition(spawnInfo.m_definition)
	, m_uid(actorUID)
	, Entity(spawnInfo.m_position, spawnInfo.m_orientation)
	, m_currentHealth(spawnInfo.m_definition->m_health)
{
	m_deathTimer = Timer(m_definition->m_deathTime, g_theApp->m_clock);
	m_deathTimer.Stop();
	m_changeDirectionTimer = Timer((float)g_randGen->RollRandomIntInRange(4, 8) * BEAT_TIME, g_theApp->m_clock);
	m_changeDirectionTimer.Start();
	m_velocity = spawnInfo.m_velocity;
	m_rotationSpeed = m_definition->m_rotationalVelocity;

	for (int i = 0; i < (int)m_definition->m_owendParticleEffectFilePaths.size(); i++)
	{
		m_ownedParticleEffects.push_back(g_theParticleSystem->AddParticleEffectByFileName(m_definition->m_owendParticleEffectFilePaths[i], GetWorldPosition(), GetWorldOrientaiton(), true));
	}
	m_prevFramePos = GetWorldPosition();

	for (int i = 0; i < (int)m_definition->m_behaviourDefs.size(); i++)
	{
		AIBehaviour* currBehaviour = AIBehaviour::ConstructAIBehaviour(m_definition->m_behaviourDefs[i], *this);
		m_aiBehaviours.push_back(currBehaviour);
		if (currBehaviour->m_definition.m_default)
		{
			m_currentBehaviour = currBehaviour;
			m_currentBehaviour->OnBegin();
		}
	}
	if (m_currentBehaviour == nullptr && m_aiBehaviours.size() > 0)
	{
		m_currentBehaviour = m_aiBehaviours[0];
	}
}

Actor::~Actor()
{
	for (int i = 0; i < (int)m_aiBehaviours.size(); i++)
	{
		delete m_aiBehaviours[i];
		m_aiBehaviours[i] = nullptr;
	}
	//Entity::~Entity();
}

void Actor::Update(float deltaSeconds)
{
	if (m_isTargeted)
	{
		m_color = Rgba8(255, 200, 200);
	}
	else
	{
		m_color = Rgba8::WHITE;
	}
	if (m_lifetimeTimer.HasPeriodElapsed())
	{
		Die();
		return;
	}
	if (m_deathTimer.HasPeriodElapsed())
	{
		m_isAlive = false;
		return;
	}
	if (m_definition->m_debugDrawCollider)
	{
		DebugDrawCollider();
	}

	//AI
	if (m_currentBehaviour != nullptr)
	{
		AIBehaviour* prevBehaviour = m_currentBehaviour;
		m_currentBehaviour = m_currentBehaviour->Tick(deltaSeconds);
		if (prevBehaviour != m_currentBehaviour)
		{
			prevBehaviour->OnEnd();
			m_currentBehaviour->OnBegin();
		}
	}

	if (m_definition->m_name == "Bob")
	{
		if (m_changeDirectionTimer.HasPeriodElapsed())
		{
			m_changeDirectionTimer.Start();
			steerForward = !steerForward;

			if (steerForward)
			{
				m_velocity = GetWorldOrientaiton().GetIBasis3D() * 5.f;
			}
			else
			{
				m_velocity = GetWorldOrientaiton().GetIBasis3D() * -5.f;
			}
		}
	}
	m_velocity = m_velocity.Clamp(m_definition->m_maxSpeed);

	Translate(m_velocity * deltaSeconds);
	if (m_definition->m_faceVelocity && !m_overrideOrientation)
	{
		//DebugAddWorldArrow(GetWorldPosition() + Vec3(0.f, 0.f, 2.f), GetWorldPosition() + Vec3(0.f, 0.f, 2.f) + GetWorldOrientaiton().GetIBasis3D() * 5.f, .05f, 0.f, Rgba8::RED, Rgba8::RED);

		if (m_thrustVelocity.GetLengthSquared() > 0.f)
		{
			//DebugAddWorldArrow(GetWorldPosition() + Vec3(0.f, 0.f, 1.f), GetWorldPosition() + Vec3(0.f, 0.f, 1.f) + m_thrustVelocity.GetNormalized() * 5.f, .05f, 0.f, Rgba8::YELLOW, Rgba8::YELLOW);
			
			//Simple snap towards velocity
			/*
			Mat44 rotation;
			Vec3 iBasis = m_thrustVelocity.GetNormalized();
			Vec3 kBasis = g_theGame->m_playerShip->GetWorldOrientaiton().GetKBasis3D().GetNormalized();
			Vec3 jBasis = CrossProduct3D(kBasis, iBasis).GetNormalized();
			rotation.SetIJK3D(iBasis, jBasis, kBasis);
			SetLocalOrientation(rotation);
			*/

			// attempt 3 lerp between current and target
			
			Vec3 targetIBasis = m_thrustVelocity.GetNormalized();
			Mat44 currentRotation = GetWorldOrientaiton();
			Vec3 currentIBasis = currentRotation.GetIBasis3D();
			Vec3 dispToTarget = targetIBasis - currentRotation.GetIBasis3D();
			float normalizedDisplacment = RangeMap(DotProduct3D(currentIBasis, targetIBasis), -1.f, 1.f, 0.f, 1.f);
			Vec3 newIBasis = currentIBasis;
			if (fabsf(normalizedDisplacment) < .001f)
			{
				newIBasis = (currentIBasis + .01f * currentRotation.GetJBasis3D()).GetNormalized();
			}
			else
			{
				float turnRate = 4.f;
				float minNormalizedDisplacment = .1f;
				if (normalizedDisplacment < minNormalizedDisplacment)
				{
					normalizedDisplacment = minNormalizedDisplacment;
				}
				float turnRateThisFrame = normalizedDisplacment * deltaSeconds * turnRate;
				newIBasis += turnRateThisFrame * dispToTarget;
				newIBasis = newIBasis.GetNormalized();
			}

			//recompute j based on player up
			Vec3 newJBasis = CrossProduct3D(g_theGame->m_playerShip->GetWorldOrientaiton().GetKBasis3D(), currentIBasis).GetNormalized();
			Vec3 newKBasis = CrossProduct3D(newIBasis, newJBasis);
			Mat44 newOrientation;
			newOrientation.SetIJK3D(newIBasis, newJBasis, newKBasis);
			SetLocalOrientation(newOrientation);
		

			//TODO Turn towards thrust velocity instead of snapping to it currently not working
			/*
			float turnRate = 360.f;
			float angleBetweenIandVel = GetAngleDegreesBetweenVectors3D(m_thrustVelocity.GetNormalized(), GetWorldOrientaiton().GetIBasis3D());
			float turnThisFrame = deltaSeconds * turnRate;
			//clamp to turn rate
			if (turnThisFrame > angleBetweenIandVel)
			{
				turnThisFrame = angleBetweenIandVel;
			}
			if (turnThisFrame > .001f)
			{
				Vec3 axisDir = CrossProduct3D(GetWorldOrientaiton().GetIBasis3D(), m_thrustVelocity).GetNormalized();
				Mat44 rotationToFaceThrust = Mat44::CreateAxisAngleRotation(axisDir, turnThisFrame);
				Mat44 currentOrientation = GetLocalOrientation();
				currentOrientation.Append(rotationToFaceThrust);
				SetLocalOrientation(currentOrientation);
			}
			*/
			
		}
	}
	if (m_definition->m_facePlayer && !m_overrideOrientation)
	{
		Vec3 dispToPlayer = g_theGame->m_playerShip->GetWorldPosition() - GetWorldPosition();
		Mat44 rotation;
		Vec3 iBasis = dispToPlayer.GetNormalized();
		Vec3 kBasis = g_theGame->m_playerShip->GetWorldOrientaiton().GetKBasis3D().GetNormalized();
		Vec3 jBasis = CrossProduct3D(kBasis, iBasis).GetNormalized();
		rotation.SetIJK3D(iBasis, jBasis, kBasis);
		SetLocalOrientation(rotation);
	}


	if (fabsf(m_rotationSpeed.m_yaw) > 0.f || fabsf(m_rotationSpeed.m_pitch) > 0.f || fabsf(m_rotationSpeed.m_roll) > 0.f)
	{
		EulerAngles currentFrameRotation = EulerAngles(m_rotationSpeed.m_yaw * deltaSeconds, m_rotationSpeed.m_pitch * deltaSeconds, m_rotationSpeed.m_roll * deltaSeconds);
		Mat44 newRotation = GetWorldOrientaiton();
		newRotation.Append(currentFrameRotation.GetAsMatrix_IFwd_JLeft_KUp());
		SetLocalOrientation(newRotation);
	}
	

	if (m_definition->m_bezierPath)
	{
		Actor* targetActor = m_bezierTarget.GetActor();
		if (targetActor != nullptr)
		{
			UpdateMissileFirePath(targetActor);
		}
		else
		{
			Die();
		}
	}
	Mat44 transform = GetWorldTransform();
	for (int i = 0; i < (int)m_ownedParticleEffects.size(); i++)
	{
		m_ownedParticleEffects[i]->SetWorldTransform(transform);
	}
	m_prevFramePos = GetWorldPosition();
}

void Actor::Render() const
{

	if (m_definition->m_meshType == "model")
	{
		ModelDefinition const* modelDef = m_definition->GetModelDef();
		if (modelDef == nullptr)
		{
			return;
		}

		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetModelConstants(GetWorldTransform(), m_color);
		g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		if (modelDef->m_material != nullptr)
		{
			g_theRenderer->BindMaterial(modelDef->m_material);
		}
		else
		{
			g_theRenderer->BindTexture(nullptr);
			g_theRenderer->BindShader(g_theRenderer->CreateOrGetShaderFromFile("Data/Shaders/Diffuse", VertexType::VERTEX_TYPE_PCUTBN));
		}
		modelDef->m_gpuMesh->Render();
	}

	else if (m_definition->m_meshType == "ArtilleryWarnZone")
	{
		std::vector<Vertex_PCU> warnZoneVerts;
		AABB3 warnZoneBox(Vec3(-4.f, -4., -4.f), Vec3(4.f, 4.f, 4.f));

		Rgba8 warnColor = LerpColor(Rgba8::WHITE, Rgba8::RED, m_lifetimeTimer.GetElapsedFraction());
		AddVertsForBoxEdges(warnZoneVerts, warnZoneBox, .075f, warnColor);
		AddVertsForAABB3D(warnZoneVerts, warnZoneBox, Rgba8(255, 0, 0, 25));

		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED_NO_WRITE);
		g_theRenderer->SetModelConstants(GetWorldTransform(), m_color);
		g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);

		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BindShader(nullptr);

		g_theRenderer->DrawVertexArray(warnZoneVerts.size(), warnZoneVerts.data());
	}
}

bool Actor::ActorOverlapHandled(Actor& otherActor)
{
	if (m_definition->m_collisionType == CollisionType::NONE || otherActor.m_definition->m_collisionType == CollisionType::NONE)
	{
		return false;
	}
	if (m_definition->m_collisionType == CollisionType::SPHERE)
	{
		if (otherActor.m_definition->m_collisionType == CollisionType::SPHERE)
		{
			bool overlap = DoSpheresOverlap3D(GetWorldPosition(), m_definition->m_colliderSphereRadius, otherActor.GetWorldPosition(), otherActor.m_definition->m_colliderSphereRadius);
			if (overlap && !m_definition->m_isTrigger && !otherActor.m_definition->m_isTrigger)
			{
				if (m_definition->m_static == otherActor.m_definition->m_static)
				{
					Vec3 pushedPos = GetWorldPosition();
					Vec3 otherPushedPos = otherActor.GetWorldPosition();
					PushSpheresOutOfEachOther3D(pushedPos, m_definition->m_colliderSphereRadius, otherPushedPos, otherActor.m_definition->m_colliderSphereRadius);
					SetLocalPosition(pushedPos);
					otherActor.SetLocalPosition(otherPushedPos);
				}
				else if (m_definition->m_static)
				{
					Vec3 otherPushedPos = otherActor.GetWorldPosition();
					PushSphereOutOfFixedSphere3D(GetWorldPosition(), m_definition->m_colliderSphereRadius, otherPushedPos, otherActor.m_definition->m_colliderSphereRadius);
					otherActor.SetLocalPosition(otherPushedPos);
				}
				else if (otherActor.m_definition->m_static)
				{
					Vec3 pushedPos = GetWorldPosition();
					PushSphereOutOfFixedSphere3D(otherActor.GetWorldPosition(), otherActor.m_definition->m_colliderSphereRadius, pushedPos, m_definition->m_colliderSphereRadius);
					SetLocalPosition(pushedPos);
				}
			}
			return overlap;
		}
		if (otherActor.m_definition->m_collisionType == CollisionType::AABB3)
		{
			//#ToDo push AABB and Sphere out of eachother 3D and static
			AABB3 aabbWorldSpace = otherActor.m_definition->m_colliderAABB3.GetTranslated(otherActor.GetWorldPosition());
			return DoSphereAndAABBOverlap3D(GetWorldPosition(), m_definition->m_colliderSphereRadius, aabbWorldSpace);
		}
	}
	else if (m_definition->m_collisionType == CollisionType::AABB3)
	{
		if (otherActor.m_definition->m_collisionType == CollisionType::SPHERE)
		{
			//#ToDo push AABB and Sphere out of eachother 3D and static
			AABB3 aabbWorldSpace = m_definition->m_colliderAABB3.GetTranslated(GetWorldPosition());
			return DoSphereAndAABBOverlap3D(otherActor.GetWorldPosition(), otherActor.m_definition->m_colliderSphereRadius, aabbWorldSpace);
		}
		if (otherActor.m_definition->m_collisionType == CollisionType::AABB3)
		{
			//#ToDo push AABBs out of eachother 3D and static
			AABB3 aabbWorldSpace = m_definition->m_colliderAABB3.GetTranslated(GetWorldPosition());
			AABB3 otherAabbWorldSpace = otherActor.m_definition->m_colliderAABB3.GetTranslated(otherActor.GetWorldPosition());
			return DoAABBsOverlap3D(aabbWorldSpace, otherAabbWorldSpace);
		}
	}
	return false;
}

void Actor::Die()
{
	if (!m_deathTimer.IsStopped() || !m_isAlive)
	{
		return;
	}
	Actor* missileTargetActor = m_bezierTarget.GetActor();
	if (missileTargetActor != nullptr)
	{
		missileTargetActor->m_missileChasing = false;
	}

	if (m_definition->m_name == "Bob")
	{
		g_theGame->m_gameScene->SpawnBob();
	}

	//should die immediately with no period, not wait until the next frame
	if (m_deathTimer.m_period == 0.f)
	{
		m_isAlive = false;
	}
	else
	{
		m_deathTimer.Start();
	}
	if (m_definition->m_dieSound.m_soundID != MISSING_SOUND_ID)
	{
		BeatSound beatSound;
		beatSound.m_beatTimeInSeconds = BEAT_TIME;
		beatSound.m_sound = m_definition->m_dieSound.m_soundID;
		beatSound.m_volume = m_definition->m_dieSound.m_volume;
		g_theGame->m_gameScene->QueueBeatSound(beatSound);
		
		//g_theAudio->StartSound(m_definition->m_dieSound);
	}
	if (m_definition->m_dieParticleEffect != "")
	{
		if (m_definition->m_orientDieParticleEffect)
		{
			g_theParticleSystem->PlayParticleEffectByFileName(m_definition->m_dieParticleEffect, GetWorldPosition(), GetWorldOrientaiton(), m_definition->m_dieParticleEffectDuration);
		}
		else
		{
			g_theParticleSystem->PlayParticleEffectByFileName(m_definition->m_dieParticleEffect, GetWorldPosition(), Mat44(), m_definition->m_dieParticleEffectDuration);
		}
	}

	if (m_definition->m_dieActorSpawn != "" && g_randGen->RollRandomFloatZeroToOne() <= m_definition->m_dieActorSpawnChance)
	{
		SpawnInfo dieActorSpawnInfo(GetWorldPosition(), GetWorldOrientaiton(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName(m_definition->m_dieActorSpawn));
		g_theGame->m_gameScene->SpawnActor(dieActorSpawnInfo);
	}

	for (int i = 0; i < (int)m_ownedParticleEffects.size(); i++)
	{
		delete m_ownedParticleEffects[i];
	}
}

void Actor::TakeDamage(float damage)
{
	m_currentHealth -= damage;
	if (m_currentHealth <= 0.f)
	{
		Die();
	}
	else
	{
		if (m_definition->m_hurtSound.m_soundID != MISSING_SOUND_ID)
		{
			BeatSound beatSound;
			beatSound.m_beatTimeInSeconds = BEAT_TIME * .25f;
			beatSound.m_sound = m_definition->m_hurtSound.m_soundID;
			beatSound.m_volume = m_definition->m_hurtSound.m_volume;
			g_theGame->m_gameScene->QueueBeatSound(beatSound);
		}
	}
}

void Actor::DebugDrawCollider()
{
	if (m_definition->m_collisionType == CollisionType::SPHERE)
	{
		DebugAddWorldWireSphere(GetWorldPosition(), m_definition->m_colliderSphereRadius, 0.f, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USE_DEPTH);
	}
	else if (m_definition->m_collisionType == CollisionType::AABB3)
	{
		AABB3 collider= AABB3(m_definition->m_colliderAABB3.m_mins, m_definition->m_colliderAABB3.m_maxs).GetTranslated(GetWorldPosition());
		DebugAddWorldAABB3(collider.m_mins, collider.m_maxs, 0.f, Rgba8(0,0,255,100));
	}
}

void Actor::Activate()
{
	m_isActive = true;
	float lifetime = m_definition->m_lifetime;
	if (m_lifetimeOverride > 0.f)
	{
		lifetime = m_lifetimeOverride;
	}
	if (lifetime >= 0.f)
	{
		m_lifetimeTimer = Timer(lifetime, g_theApp->m_clock);
		m_lifetimeTimer.Start();
	}
}

void Actor::SetRotationFromFwdPlayerUp(Vec3 const& fwdNormal)
{
	Vec3 iBasis = -fwdNormal;
	Vec3 kBasis = g_theGame->m_playerShip->GetWorldOrientaiton().GetKBasis3D();
	Vec3 jBasis = CrossProduct3D(kBasis, iBasis).GetNormalized();

	Mat44 newRot;
	newRot.SetIJK3D(iBasis, jBasis, kBasis);

	SetLocalOrientation(newRot);
}

void Actor::UpdateMissileFirePath(Actor* targetActor)
{
	//end pos
	m_bezierCurve.m_endPos = targetActor->GetWorldPosition();

	//guide2
	Vec3 fireDir = m_bezierCurve.m_endPos - m_bezierCurve.m_startPos;
	fireDir = fireDir.GetNormalized();
	Vec3 midpointAlongpath = (m_bezierCurve.m_endPos + m_bezierCurve.m_startPos) * .5f;
	float midpointDirectionLength = (m_bezierCurve.m_endPos - m_bezierCurve.m_startPos).GetLength() * .15f;
	m_bezierCurve.m_guidePos2 = midpointAlongpath + m_missilePerpToPath * midpointDirectionLength;


	float t = m_lifetimeTimer.GetElapsedFraction();
	t = .25f* t + .75f * SmoothStart2(t);
	Vec3 currentPos = m_bezierCurve.EvaluateAtParametric(t);
	SetLocalPosition(currentPos);
	SetLocalOrientation(GetRotationMatrixFromIBasisKUp((currentPos - m_prevFramePos).GetNormalized()));
}

SpawnInfo::SpawnInfo(Vec3 const& position, Mat44 const& orientation, Vec3 const& velocity, ActorDefinition const* actorDef)
	: m_position(position)
	, m_orientation(orientation)
	, m_velocity(velocity)
	, m_definition(actorDef)
{
}
