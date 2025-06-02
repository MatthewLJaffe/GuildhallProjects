#include "Game/PlayerShip.hpp"
#include "Game/Model.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Game/GameScene.hpp"
#include "Game/ParticleEntity.hpp"
#include "Game/Actor.hpp"
#include "Game/EndScene.hpp"

PlayerShip::PlayerShip(SpawnInfo const& spawnInfo, ActorUID actorUID)
	: Actor(spawnInfo, actorUID)
{
	m_camera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.f, .1f, 1000.f);
	m_camera.m_mode = Camera::eMode_Perspective;
	m_camera.SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.f, 1.f, 0.f));
	m_camera.m_useWorldTransform = true;
	m_cameraTransform.AppendTranslation3D(m_defaultCameraDisplacment);
	m_cameraTransform.Append(m_defaultCameraRotation.GetAsMatrix_IFwd_JLeft_KUp());
	AddChild(new Model(Vec3(0.f, 0.f, 0.f), "Data/Models/Ships/StrikerBlueModel.xml"));

	
	ParticleEffect* thrustCenter = g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/Thrust.xml", Vec3::ZERO, EulerAngles(), true, -1.f, true);
	ParticleEffect* thrustLeft = g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/Thrust.xml", Vec3::ZERO, EulerAngles(), true, -1.f, true);
	ParticleEffect* thrustRight= g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/Thrust.xml", Vec3::ZERO, EulerAngles(), true, -1.f, true);
	Vec3 thrustCenterDisp(-1.75f, 0.f, .15f);
	Vec3 thrustLeftDisp(-1.1f, .55f, -.1f);
	Vec3 thrustRightDisp(-1.1f, -.55f, -.1f);
	m_children[0]->AddChild(new ParticleEntity(thrustCenterDisp, EulerAngles(), thrustCenter));
	m_children[0]->AddChild(new ParticleEntity(thrustLeftDisp, EulerAngles(), thrustLeft));
	m_children[0]->AddChild(new ParticleEntity(thrustRightDisp, EulerAngles(), thrustRight));
	m_thrustEffects.push_back(thrustCenter);
	m_thrustEffects.push_back(thrustLeft);
	m_thrustEffects.push_back(thrustRight);
	

	Vec3 dashRightWingTip = Vec3(-.5f, -1.05f, -.5f);
	Vec3 dashLeftWingTip = Vec3(-.5f, 1.05f, -.5f);
	ParticleEffect* dashRightWing = g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/DashWing.xml", Vec3::ZERO, EulerAngles(), false, -1.f, true);
	ParticleEffect* dashLeftWing = g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/DashWing.xml", Vec3::ZERO, EulerAngles(), false, -1.f, true);
	m_dashEffects.push_back(dashRightWing);
	m_dashEffects.push_back(dashLeftWing);
	m_children[0]->AddChild(new ParticleEntity(dashRightWingTip, EulerAngles(), dashRightWing));
	m_children[0]->AddChild(new ParticleEntity(dashLeftWingTip, EulerAngles(), dashLeftWing));

	
	ParticleEffect* thrustDashShockwave = g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/DashShockwave.xml", Vec3::ZERO, EulerAngles(), false, .5f, true);
	m_children[0]->AddChild(new ParticleEntity(thrustCenterDisp, EulerAngles(), thrustDashShockwave));
	m_dashEffects.push_back(thrustDashShockwave);
	

	Mat44 worldCameraTransform;
	worldCameraTransform.Append(GetWorldTransform());
	worldCameraTransform.Append(m_cameraTransform);
	m_camera.SetTransform(worldCameraTransform);
	m_adjustRollTimer = Timer(1.25f, g_theApp->m_clock);
	m_adjustRollTimer.Start();
	m_dashTimer = Timer(m_dashDuration, g_theApp->m_clock);
	m_dashCooldownTimer = Timer(m_dashCooldownDuration, g_theApp->m_clock);

	m_dashSpeedKeyFrames.push_back(AnimValue(0.f, 0.f, EasingFunction::SMOOTH_STEP_3));
	m_dashSpeedKeyFrames.push_back(AnimValue(.25f, m_maxSpeedAlongAxis, EasingFunction::LINEAR));
	m_dashSpeedKeyFrames.push_back(AnimValue(.7f, m_maxSpeedAlongAxis, EasingFunction::SMOOTH_STOP_2));
	m_dashSpeedKeyFrames.push_back(AnimValue(1.f, 0.f, EasingFunction::LINEAR));

	m_dashRollKeyFrames.push_back(AnimValue(0.f, 0.f, EasingFunction::SMOOTH_STEP_3));
	m_dashRollKeyFrames.push_back(AnimValue(.25f, 0.f, EasingFunction::LINEAR));
	m_dashRollKeyFrames.push_back(AnimValue(.7f, 250.f, EasingFunction::SMOOTH_STOP_2));
	m_dashRollKeyFrames.push_back(AnimValue(1.f, 360.f, EasingFunction::LINEAR));

	m_badDashSpeedKeyFrames.push_back(AnimValue(0.f, 0.f, EasingFunction::SMOOTH_STEP_3));
	m_badDashSpeedKeyFrames.push_back(AnimValue(.35f, m_maxSpeedAlongAxis * .8f, EasingFunction::LINEAR));
	m_badDashSpeedKeyFrames.push_back(AnimValue(.6f, m_maxSpeedAlongAxis * .8f, EasingFunction::SMOOTH_STOP_2));
	m_badDashSpeedKeyFrames.push_back(AnimValue(1.f, 0.f, EasingFunction::LINEAR));

	m_badDashRollKeyFrames.push_back(AnimValue(0.f, 0.f, EasingFunction::SMOOTH_STEP_3));
	m_badDashRollKeyFrames.push_back(AnimValue(.35f, 0.f, EasingFunction::LINEAR));
	m_badDashRollKeyFrames.push_back(AnimValue(.6f, 25.f, EasingFunction::SMOOTH_STOP_2));
	m_badDashRollKeyFrames.push_back(AnimValue(1.f, 35.f, EasingFunction::LINEAR));
	m_prevFrameMousePos = g_theInput->GetCursorPosition().GetVec2();

	m_megaLaserCharge = g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/LaserBeamCharge.xml", Vec3::ZERO, EulerAngles(), false, -1.f, true);
	m_megaLaserFire = g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/LaserBeamImmediate.xml", Vec3::ZERO, EulerAngles(), false, -1.f, true);
	m_laserSound = g_theAudio->CreateOrGetSound("Data/Audio/LaserShoot3.mp3");
	m_dashSound = g_theAudio->CreateOrGetSound("Data/Audio/DashV1.mp3");
	m_reloadSound = g_theAudio->CreateOrGetSound("Data/Audio/Reload.mp3");
	m_reloadFailedSound = g_theAudio->CreateOrGetSound("Data/Audio/ReloadFailed.mp3");

	m_megaLaserChargeSFX = g_theAudio->CreateOrGetSound("Data/Audio/MegaLaserCharge-2.mp3");
	m_megaLaserFire1SFX = g_theAudio->CreateOrGetSound("Data/Audio/MegaLaser-2-1.mp3");
	m_megaLaserFire2SFX = g_theAudio->CreateOrGetSound("Data/Audio/MegaLaser-2-2.mp3");

	m_missleTrackSFX = g_theAudio->CreateOrGetSound("Data/Audio/Tracks/RocketsTrack-2.mp3");

	m_primaryRechargeCooldown = Timer(BEAT_TIME, g_theApp->m_clock);
	m_primaryRechargeCooldown.Start();

	m_failedReloadTimer = Timer(BEAT_TIME * 2.f, g_theApp->m_clock);
	m_reloadTimer = Timer(BEAT_TIME, g_theApp->m_clock);

}

void PlayerShip::Update(float deltaSeconds)
{
	if (m_deathTimer.HasPeriodElapsed())
	{
		g_theGame->m_gameScene->LoseGame();
		return;
	}
	if (g_theInput->WasKeyJustPressed('K'))
	{
		Die();
	}
	
	GetTargetedActor();
	m_currentFrameTranslation = Vec3::ZERO;
	m_currentFrameRotation = EulerAngles();
	m_modelThrustTranslationX = 0.f;
	if (m_isPlayerAlive)
	{
		if (g_theInput->GetController(0).IsConnected())
		{
			HandleControlsController(deltaSeconds);
		}
		else
		{
			HandleControlsKeyboard(deltaSeconds);
		}
	}
	if (!m_reloadTimer.HasPeriodElapsed() && !m_reloadTimer.IsStopped())
	{
		float t = m_reloadTimer.GetElapsedFraction();
		t = SmoothStep3(t);
		m_primaryCurrentEnergy = Lerp(m_primaryEnergyBeforeRecharge, m_primaryMaxEnergy, t);
	}
	else if (m_primaryRechargeCooldown.HasPeriodElapsed())
	{
		m_primaryCurrentEnergy += m_primaryRechargeRate * deltaSeconds;
		m_primaryCurrentEnergy = Clamp(m_primaryCurrentEnergy, 0.f, m_primaryMaxEnergy);
	}

	UpdatePlayerDash(deltaSeconds);
	MoveShipTowardsVelocity(deltaSeconds);
	UpdateThrustDirection();
	UpdateMegaLaser(deltaSeconds);
	UpdateMissileFire(deltaSeconds);
	m_currentFrameTranslation += m_velocity * deltaSeconds;
	m_transform.AppendTranslation3D(m_currentFrameTranslation);
	m_transform.Append(m_currentFrameRotation.GetAsMatrix_IFwd_JLeft_KUp());

	if (m_adjustRollTimer.HasPeriodElapsed())
	{
		//idk if this is even necessary
		//FixShipRoll(deltaSeconds);
	}

	m_children[0]->SetLocalPosition(m_moveModelTranslation + m_yawPitchModelTranslation);
	m_children[0]->SetLocalOrientation(m_moveModelRotation + m_yawPitchModelRotation);
	for (int i = 0; i < (int)m_children.size(); i++)
	{
		m_children[i]->Update(deltaSeconds);
	}
	
	Mat44 worldCameraTransform;
	worldCameraTransform.Append(GetWorldTransform());
	worldCameraTransform.Append(m_cameraTransform);
	m_camera.SetTransform(worldCameraTransform);

	/*
	Mat44 debugBasis;
	debugBasis.AppendTranslation3D(GetWorldPosition());
	//debugBasis.AppendTranslation3D(m_cameraTransform.GetTranslation3D());
	debugBasis.AppendTranslation3D(m_cameraTransform.GetIBasis3D());
	debugBasis.SetScale(Vec3(.33f, .33f, .33f));
	DebugAddWorldBasis(debugBasis, 0.f, DebugRenderMode::ALWAYS);
	*/
}

PlayerShip::~PlayerShip()
{
	delete m_megaLaserCharge;
	delete m_megaLaserFire;
	//Actor::~Actor();
}

void PlayerShip::Render() const
{
	if (!m_isPlayerAlive)
	{
		return;
	}
	for (int i = 0; i < (int)m_children.size(); i++)
	{
		m_children[i]->Render();
	}
}


Mat44 PlayerShip::GetWorldTransform() const
{
	return m_transform;
}

void PlayerShip::GetTargetedActor()
{
	Vec3 cameraPos = m_camera.GetPosition();
	Vec3 cameraFowarard = m_camera.GetCameraForward();

	std::vector<Actor*>& allActors = g_theGame->m_gameScene->m_allActors;
	m_targetedActorUID = ActorUID::INVALID;
	float bestTheta = 99999999999.f;
	for (int i = 0; i < (int)allActors.size(); i++)
	{
		if (!IsValidActor(allActors[i]))
		{
			continue;
		}
		Vec3 dispToActor = allActors[i]->GetWorldPosition() - cameraPos;
		float theta = GetUnsignedAngleBetweenVectors3D(dispToActor, cameraFowarard);
		if (theta < allActors[i]->m_definition->m_maxTargetAngle && theta < bestTheta)
		{
			bestTheta = theta;
			m_targetedActorUID = allActors[i]->m_uid;
		}
		allActors[i]->m_isTargeted = false;
	}

	if (m_targetedActorUID != ActorUID::INVALID)
	{
		m_targetedActorUID.GetActor()->m_isTargeted = true;
	}
}


void PlayerShip::StopMegaLaser()
{
	if (!m_usingMegaLaser)
	{
		return;
	}
	m_usingMegaLaser = false;
	if (m_megaLaserCurrentlyFiring)
	{
		m_megaLaserFire->Stop();
		m_megaLaserCurrentlyFiring = false;
	}
	else
	{
		m_megaLaserCharge->Stop();
	}
	if (m_currentMegaLaserSound != MISSING_SOUND_ID)
	{
		g_theAudio->StopSound(m_currentMegaLaserSound);
		m_currentMegaLaserSound = MISSING_SOUND_ID;
	}
}

void PlayerShip::StopMissiles()
{
	if (!m_usingMissiles)
	{
		return;
	}
	m_usingMissiles = false;
	//g_theGame->m_gameScene->SetTrackLayerVolume(TrackLayerType::MISSILE, 0.f);
	StopMissileSound();
}

void PlayerShip::HandlePlayerThrust(float deltaSeconds, float thrustAxis)
{
	//thrust
	if (fabsf(thrustAxis) > .25f)
	{
		m_velocity.x += deltaSeconds * thrustAxis * m_thrustAcceration;
	}
	//slow down when not thrusting
	else
	{
		if (m_velocity.x > 0.f)
		{
			if (m_velocity.x < m_nonThrustSlowDown * deltaSeconds)
			{
				m_velocity.x = 0.f;
			}
			else
			{
				m_velocity.x -= m_nonThrustSlowDown * deltaSeconds;
			}
		}
		else if (m_velocity.x < 0.f)
		{
			if (fabsf(m_velocity.x) < m_nonThrustSlowDown * deltaSeconds)
			{
				m_velocity.x = 0.f;
			}
			else
			{
				m_velocity.x += m_nonThrustSlowDown * deltaSeconds;
			}
		}
	}
	m_velocity.x = Clamp(m_velocity.x, m_thrustSpeedRange);

	if (m_isDashing)
	{
		if (m_currentDashDirection.x > 0.f)
		{
			m_currentDashSpeedKeyFrames[m_currentDashSpeedKeyFrames.size() - 1].m_value = m_velocity.x;
		}
	}
	else
	{
		//match thrust particles to axis
		float thrustAxisPositive = thrustAxis;
		thrustAxisPositive = Clamp(thrustAxisPositive, 0.f, 1.f);
		for (int i = 0; i < (int)m_thrustEffects.size(); i++)
		{
			EmitterUpdateDefinitionGPU* updateDef = m_thrustEffects[i]->m_emitters[0]->GetEmitterUpdateDef();
			updateDef->m_emissionRate = RangeMap(thrustAxisPositive, 0.f, 1.f, m_thrustParticlesRange.m_min, m_thrustParticlesRange.m_max);
			m_thrustEffects[i]->m_emitters[0]->GetEmitterUpdateDef()->isDirty = true;
		}
	}

	
}

void PlayerShip::HandlePlayerStrafe(float deltaSeconds, Vec2 strafeAxis)
{
	Vec2 velocityJK(m_velocity.y, m_velocity.z);
	Vec2 correctedStrafeAxis = Vec2(-strafeAxis.x, strafeAxis.y);
	//strafe left / right
	if (correctedStrafeAxis.GetLength() > .01f)
	{
		m_adjustRollTimer.Start();
		velocityJK += correctedStrafeAxis * deltaSeconds * m_strafeAcceleration;
		velocityJK.ClampLength(m_maxStrafeSpeed);
	}
	//slow down strafe
	else if (velocityJK.GetLength() > .0f)
	{
		Vec2 prevStrafeVelocity = velocityJK;
		velocityJK -= velocityJK * deltaSeconds * m_nonstrafeSlowDown;
		if (DotProduct2D(prevStrafeVelocity, velocityJK) < 0.f)
		{
			velocityJK = Vec2::ZERO;
		}
	}
	m_velocity.y = velocityJK.x;
	m_velocity.z = velocityJK.y;
	//dashing horizontally
	if (m_isDashing && fabsf(m_currentDashDirection.y) > 0.f)
	{
		m_currentDashSpeedKeyFrames[m_currentDashSpeedKeyFrames.size() - 1].m_value = fabsf(m_velocity.y);
	}

	Vec2 normalizedVelcotiy = Vec2(m_velocity.y / m_maxSpeedAlongAxis, m_velocity.z / m_maxSpeedAlongAxis);
	normalizedVelcotiy.x *= -1.f;
	float normalizedModelRoll = m_currentStrafeRoll / m_maxModelRollRotationOffset;
	float modelRollDir = normalizedVelcotiy.x - normalizedModelRoll;
	normalizedModelRoll += (modelRollDir * deltaSeconds * m_modelRollRotationSpeed);

	//overshot velocity
	if ((normalizedVelcotiy.x - normalizedModelRoll) * modelRollDir < 0)
	{
		normalizedModelRoll = normalizedVelcotiy.x;
	}
	m_currentStrafeRoll = normalizedModelRoll * m_maxModelRollRotationOffset;
	if (m_isDashing)
	{
		if (m_isDashGood)
		{
			if (m_currentDashDirection.y < 0.f || m_currentDashDirection.x > 0.f)
			{
				m_currentDashRollKeyFrames[m_currentDashRollKeyFrames.size() - 1].m_value = 360.f + m_currentStrafeRoll;
			}
			else
			{
				m_currentDashRollKeyFrames[m_currentDashRollKeyFrames.size() - 1].m_value = -360.f + m_currentStrafeRoll;
			}
		}
		else
		{
			m_currentDashRollKeyFrames[m_currentDashRollKeyFrames.size() - 1].m_value = m_currentStrafeRoll;
		}
	}

	//we do not want the ship to roll according to strafe if we are dashing
	if (!m_isDashing)
	{
		m_moveModelRotation.m_roll = m_currentStrafeRoll;
	}
}

void PlayerShip::TakeDamage(float damage)
{
	if (m_isInvincible)
	{
		return;
	}
	Actor::TakeDamage(damage);
}

void PlayerShip::HandlePlayerYawPitch(float deltaSeconds, Vec2 yawPitchAxis)
{
	Vec2 angularVelocityBefore = m_currentAngularVelocity;
	if (yawPitchAxis.GetLengthSquared() > .01f)
	{
		m_currentAngularVelocity += yawPitchAxis * m_angularAcceleration * deltaSeconds;
		m_adjustRollTimer.Start();
	}
	else
	{
		if (m_currentAngularVelocity.GetLength() > 0.f)
		{
			m_currentAngularVelocity -= m_currentAngularVelocity.GetNormalized() * m_angularSlowDown * deltaSeconds;
			if (DotProduct2D(angularVelocityBefore, m_currentAngularVelocity) <= 0.f)
			{
				m_currentAngularVelocity = Vec2::ZERO;
			}
		}
	}
	m_currentAngularVelocity.ClampLength(m_maxAngularSpeed);
	m_currentFrameRotation.m_yaw -= m_currentAngularVelocity.x * deltaSeconds;
	m_currentFrameRotation.m_pitch -= m_currentAngularVelocity.y * deltaSeconds;

	Vec2 normalizedVelcotiy = Vec2(m_currentAngularVelocity.x / m_maxAngularSpeed, m_currentAngularVelocity.y / m_maxAngularSpeed);
	Vec2 normalizedModelPosition = Vec2(m_yawPitchModelTranslation.y / m_modelYawJTranslationRange.m_max, -m_yawPitchModelTranslation.z / m_modelPitchKTranslationRange.m_max);
	Vec2 modelTranslationDir = (normalizedVelcotiy - normalizedModelPosition);
	normalizedModelPosition += (modelTranslationDir * deltaSeconds * m_modelYawPitchTranslationSpeed);
	if (DotProduct2D(normalizedVelcotiy - normalizedModelPosition, modelTranslationDir) < 0.f)
	{
		normalizedModelPosition = normalizedVelcotiy;
	}
	m_yawPitchModelTranslation.y = normalizedModelPosition.x * m_modelYawJTranslationRange.m_max;
	m_yawPitchModelTranslation.z = -normalizedModelPosition.y * m_modelPitchKTranslationRange.m_max;

	Vec2 normalizedModelYawPitch = Vec2(-m_yawPitchModelRotation.m_yaw / m_maxModelYawRotationOffset, -m_yawPitchModelRotation.m_pitch / m_maxModelPitchRotationOffset);
	Vec2 modelRotationDir = normalizedVelcotiy - normalizedModelYawPitch;
	normalizedModelYawPitch += (modelRotationDir * deltaSeconds * m_modelYawPitchRotationSpeed);
	if (DotProduct2D(normalizedVelcotiy - normalizedModelYawPitch, modelRotationDir) < 0.f)
	{
		normalizedModelYawPitch = normalizedVelcotiy;
	}
	m_yawPitchModelRotation.m_yaw = -normalizedModelYawPitch.x * m_maxModelYawRotationOffset;
	m_yawPitchModelRotation.m_pitch = -normalizedModelYawPitch.y * m_maxModelPitchRotationOffset;
	m_yawPitchModelRotation.m_roll = normalizedModelYawPitch.x * m_maxModelRollRotationOffset;

}

void PlayerShip::MoveShipTowardsVelocity(float deltaSeconds)
{
	Vec3 normalizedVelcotiy = m_velocity / m_maxSpeedAlongAxis;
	Vec3 normalizedModelPosition = Vec3(
		m_moveModelTranslation.x / m_modelThrustITranslationRange.m_max,
		m_moveModelTranslation.y / m_modelStrafeJTranslationRange.m_max,
		m_moveModelTranslation.z / m_modelStrafeKTranslationRange.m_max);

	Vec3 modelTranslationDir = (normalizedVelcotiy - normalizedModelPosition);
	normalizedModelPosition += (modelTranslationDir * deltaSeconds * m_modelStrafeTranslationSpeed);
	if (DotProduct3D(normalizedVelcotiy - normalizedModelPosition, modelTranslationDir) <= 0.f)
	{
		normalizedModelPosition = normalizedVelcotiy;
	}
	m_moveModelTranslation.x = normalizedModelPosition.x * m_modelThrustITranslationRange.m_max;
	m_moveModelTranslation.y = normalizedModelPosition.y * m_modelStrafeJTranslationRange.m_max;
	m_moveModelTranslation.z = normalizedModelPosition.z * m_modelStrafeKTranslationRange.m_max;
}

void PlayerShip::Die()
{
	
	if (!m_deathTimer.IsStopped())
	{
		return;
	}
	for (int i = 0; i < (int)m_thrustEffects.size(); i++)
	{
		m_thrustEffects[i]->KillParticles();
	}
	for (int i = 0; i < (int)m_dashEffects.size(); i++)
	{
		m_dashEffects[i]->KillParticles();
	}

	StopMegaLaser();
	StopMissiles();

	if (m_definition->m_dieSound.m_soundID != MISSING_SOUND_ID)
	{
		BeatSound beatSound;
		beatSound.m_beatTimeInSeconds = BEAT_TIME;
		beatSound.m_sound = m_definition->m_dieSound.m_soundID;
		beatSound.m_volume = m_definition->m_dieSound.m_volume;
		g_theGame->m_gameScene->QueueBeatSound(beatSound);

		//g_theAudio->StartSound(m_definition->m_dieSound);
	}
	m_deathTimer.Start();
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
	m_isPlayerAlive = false;
}

void PlayerShip::HandlePlayerYawPitchAxisAngle(float deltaSeconds, Vec2 yawPitchAxis)
{
	if (yawPitchAxis.GetLengthSquared() > .01f)
	{
		m_adjustRollTimer.Start();
	}
	Vec2 normalizedRotationVelocity = m_currentAngularVelocity / m_maxAngularSpeed;
	Vec2 dispFromVelocityToStick = yawPitchAxis - normalizedRotationVelocity;
	Vec2 normalizedRotationVelocityBefore = normalizedRotationVelocity;
	float normalizedRotationSpeed = 1.f;
	normalizedRotationVelocity += dispFromVelocityToStick * normalizedRotationSpeed;
	if (DotProduct2D(dispFromVelocityToStick, yawPitchAxis - normalizedRotationVelocity) <= 0.f)
	{
		normalizedRotationVelocity = yawPitchAxis;
	}
	m_currentAngularVelocity = normalizedRotationVelocity * m_maxAngularSpeed;

	//apply yaw pitch rotation
	if (normalizedRotationVelocity.GetLengthSquared() > .00001f)
	{
		Vec3 axisOfRotationKComponent;
		Vec3 axisOfRotationJComponent;
		axisOfRotationKComponent = normalizedRotationVelocity.x * Vec3(0.f, 0.f, 1.f);
		axisOfRotationJComponent = normalizedRotationVelocity.y * Vec3(0.f, 1.f, 0.f);

		Vec3 axisOfRotation = (axisOfRotationKComponent + axisOfRotationJComponent).GetNormalized();
		float theta = m_currentAngularVelocity.GetLength() * deltaSeconds;
		Mat44 currentFrameRotationMatrix = Mat44::CreateAxisAngleRotation(axisOfRotation, theta);
		m_transform.Append(currentFrameRotationMatrix);
	}
	
	
	Vec2 normalizedModelPosition = Vec2(-m_yawPitchModelTranslation.y / m_modelYawJTranslationRange.m_max, m_yawPitchModelTranslation.z / m_modelPitchKTranslationRange.m_max);
	Vec2 modelTranslationDir = (-yawPitchAxis - normalizedModelPosition);
	normalizedModelPosition += (modelTranslationDir * deltaSeconds * m_modelYawPitchTranslationSpeed);
	if (DotProduct2D(-yawPitchAxis - normalizedModelPosition, modelTranslationDir) < 0.f)
	{
		normalizedModelPosition = yawPitchAxis;
	}
	m_yawPitchModelTranslation.y = -normalizedModelPosition.x * m_modelYawJTranslationRange.m_max;
	m_yawPitchModelTranslation.z = normalizedModelPosition.y * m_modelPitchKTranslationRange.m_max;
	

	Vec2 normalizedModelYawPitch = Vec2(-m_yawPitchModelRotation.m_yaw / m_maxModelYawRotationOffset, -m_yawPitchModelRotation.m_pitch / m_maxModelPitchRotationOffset);
	Vec2 modelRotationDir = yawPitchAxis - normalizedModelYawPitch;
	normalizedModelYawPitch += (modelRotationDir * deltaSeconds * m_modelYawPitchRotationSpeed);
	if (DotProduct2D(yawPitchAxis - normalizedModelYawPitch, modelRotationDir) < 0.f)
	{
		normalizedModelYawPitch = yawPitchAxis;
	}
	m_yawPitchModelRotation.m_yaw = -normalizedModelYawPitch.x * m_maxModelYawRotationOffset;
	m_yawPitchModelRotation.m_pitch = -normalizedModelYawPitch.y * m_maxModelPitchRotationOffset;
	m_yawPitchModelRotation.m_roll = normalizedModelYawPitch.x * m_maxModelRollRotationOffset;
}

void PlayerShip::FixShipRoll(float deltaSeconds)
{
	Mat44 rotationMatrixCopy;
	rotationMatrixCopy.SetIJK3D(m_transform.GetIBasis3D(), m_transform.GetJBasis3D(), m_transform.GetKBasis3D());
	Vec3 worldUp = Vec3(0.f, 0.f, 1.f);
	float upsideDownNess = -DotProduct3D(worldUp, rotationMatrixCopy.GetKBasis3D()) * .5f + .5f;
	upsideDownNess = SmoothStop6(upsideDownNess);
	float rollRotation = Lerp(m_autoAdjustRollRate.m_min, m_autoAdjustRollRate.m_max, upsideDownNess) * deltaSeconds;

	Mat44 positiveRoll;
	positiveRoll.AppendXRotation(rollRotation);
	Mat44 negativeRoll;
	negativeRoll.AppendXRotation(-rollRotation);


	Vec3 positiveRollUp = positiveRoll.TransformVectorQuantity3D(worldUp);
	positiveRollUp = rotationMatrixCopy.TransformVectorQuantity3D(positiveRollUp);
	Vec3 negativeRollUp = negativeRoll.TransformVectorQuantity3D(worldUp);
	negativeRollUp = rotationMatrixCopy.TransformVectorQuantity3D(negativeRollUp);
	if (DotProduct3D(positiveRollUp, worldUp) > DotProduct3D(m_transform.GetKBasis3D(), worldUp))
	{
		m_transform.AppendXRotation(rollRotation);
	}
	else if (DotProduct3D(negativeRollUp, worldUp) > DotProduct3D(m_transform.GetKBasis3D(), worldUp))
	{
		m_transform.AppendXRotation(-rollRotation);
	}
}

void PlayerShip::ResetGameState()
{
}

Entity* PlayerShip::GetShipModelEntity()
{
	return m_children[0];
}

Vec3 PlayerShip::GetWorldSpaceVelocity()
{
	Vec3 worldVelocity = m_velocity;
	worldVelocity = GetWorldOrientaiton().TransformVectorQuantity3D(worldVelocity);
	return worldVelocity;
}

void PlayerShip::StartPlayerDash(Vec2 dashDirection)
{
	if ((!m_dashCooldownTimer.IsStopped() && !m_dashCooldownTimer.HasPeriodElapsed()) || m_isDashing)
	{
		return;
	}
	static float totalBeatMS = 0.f;
	static int totalDashes = 0;
	float currentBeatTimeToPlayer = g_theGame->m_gameScene->m_currentTrackPlaybackTime - g_theGame->m_gameScene->m_offsetInMS / 1000.f;
	float currentBeatsToPlayer = currentBeatTimeToPlayer / BEAT_TIME;
	float beatFractionToPlayer = currentBeatsToPlayer - RoundDownToInt(currentBeatsToPlayer);
	if (beatFractionToPlayer > .5f)
	{
		beatFractionToPlayer = (1.f - beatFractionToPlayer) * -1.f;
	}
	totalBeatMS += beatFractionToPlayer * BEAT_TIME * 1000.f;
	totalDashes++;
	m_averageBeatDeviationMS = totalBeatMS / (float)totalDashes;
	if (!g_theGame->m_gameScene->m_isFrameOnBeatToPlayer)
	{
		//DebugAddMessage(Stringf("Current MS %.2f", beatFractionToPlayer * BEAT_TIME * 1000.f), 20.f, 10.f, Rgba8::RED, Rgba8::RED);

		m_isDashGood = false;
		m_dashTimer.m_period = m_badDashDuration;
		m_currentDashRollKeyFrames = m_badDashRollKeyFrames;
		m_currentDashSpeedKeyFrames = m_badDashSpeedKeyFrames;
	}
	else
	{
		m_isDashGood = true;
		m_dashTimer.m_period = m_dashDuration;
		m_currentDashRollKeyFrames = m_dashRollKeyFrames;
		m_currentDashSpeedKeyFrames = m_dashSpeedKeyFrames;
		//DebugAddMessage(Stringf("Current MS %.2f", beatFractionToPlayer * BEAT_TIME * 1000.f), 20.f, 10.f, Rgba8::CYAN, Rgba8::CYAN);
		g_theAudio->StartSound(m_dashSound);

	}

	m_isDashing = true;
	m_dashTimer.Start();
	m_currentDashDirection = Vec3( dashDirection.y, -dashDirection.x, 0.f);
	m_startDashVelocity = m_velocity;

	//roll clockwise
	if (m_currentDashDirection.y < 0.f || m_currentDashDirection.x > 0.f)
	{
		for (int i = 2; i < m_currentDashRollKeyFrames.size() - 1; i++)
		{
			m_currentDashRollKeyFrames[i].m_value = fabsf(m_currentDashRollKeyFrames[i].m_value);
		}
		/*
		for (int i = 1; i < m_currentDashSpeedKeyFrames.size() - 1; i++)
		{
			m_currentDashSpeedKeyFrames[i].m_value = -fabsf(m_currentDashSpeedKeyFrames[i].m_value);
		}
		*/
	}

	//roll counter clockwise
	else
	{
		for (int i = 2; i < m_currentDashRollKeyFrames.size() - 1; i++)
		{
			m_currentDashRollKeyFrames[i].m_value = -fabsf(m_currentDashRollKeyFrames[i].m_value);
		}
		/*
		for (int i = 1; i < m_currentDashSpeedKeyFrames.size() - 1; i++)
		{
			m_currentDashSpeedKeyFrames[i].m_value = fabsf(m_currentDashSpeedKeyFrames[i].m_value);
		}
		*/
	}

	//make beginning of dash match current roll
	m_currentDashSpeedKeyFrames[0].m_value = m_velocity.GetLength();
	m_currentDashRollKeyFrames[0].m_value = m_moveModelRotation.m_roll;
	m_currentDashRollKeyFrames[1].m_value = m_moveModelRotation.m_roll;


	for (int i = 0; i < (int)m_thrustEffects.size(); i++)
	{
		EmitterUpdateDefinitionGPU* updateDef = m_thrustEffects[i]->m_emitters[0]->GetEmitterUpdateDef();
		updateDef->m_emissionRate = 250.f;
		m_thrustEffects[i]->m_emitters[0]->GetEmitterUpdateDef()->isDirty = true;
		FloatGraph* maxSpeed = m_thrustEffects[i]->m_emitters[0]->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_MAX_SPEED);
		maxSpeed->isDirty = 1;
		maxSpeed->constantValue = 4.f;
	}
	if (m_isDashGood)
	{
		for (int i = 0; i < m_dashEffects.size(); i++)
		{
			m_dashEffects[i]->Play(m_dashDuration);
		}
	}
}

float PlayerShip::GetAnimValueFromTime(std::vector<AnimValue> const& anim, float time, bool override, float overrideStart, float overrideEnd)
{
	if (time < anim[0].m_time)
	{
		if (override)
		{
			return overrideStart;
		}
		return anim[0].m_value;
	}
	else if (time > anim[anim.size() -1].m_time)
	{
		if (override)
		{
			return overrideEnd;
		}
		return anim[anim.size() - 1].m_value;
	}
	for (int i = 0; i < (int)anim.size() - 1; i++)
	{
		if (time >= anim[i].m_time && time <= anim[i + 1].m_time)
		{
			float normalizedT = GetFractionWithinRange(time, anim[i].m_time, anim[i + 1].m_time);
			normalizedT = GetValueFromEasingFunction(anim[i].m_easingFunction, normalizedT);
			float startValue = anim[i].m_value;
			if (override && i == 0)
			{
				startValue = overrideStart;
			}
			float endValue = anim[i + 1].m_value;
			if (override && (i + 1) == (int)anim.size() - 1)
			{
				endValue = overrideEnd;
			}
			float value = Lerp(startValue, endValue, normalizedT);
			return value;
		}
	}
	return anim[0].m_value;
}

void PlayerShip::UpdateThrustDirection()
{
	Vec3 thurstDirection = -5.f * (.7f * m_yawPitchModelTranslation + m_moveModelTranslation);
	for (int i = 0; i < (int)m_thrustEffects.size(); i++)
	{
		EmitterUpdateDefinitionGPU* updateDef = m_thrustEffects[i]->m_emitters[0]->GetEmitterUpdateDef();
		Float3Graph* linearForce = m_thrustEffects[i]->m_emitters[0]->GetFloat3GraphByType(Float3GraphType::FLOAT3GRAPH_LINEAR_FORCE);
		linearForce->constantValue[0] = thurstDirection.x;
		linearForce->constantValue[1] = thurstDirection.y;
		linearForce->isDirty = true;
		updateDef->isDirty = true;
	}
}

void PlayerShip::FireSmallLaser()
{
	if (m_usingMegaLaser)
	{
		return;
	}
	if (m_primaryCurrentEnergy < m_energyPerBullet)
	{
		return;
	}
	if (!m_failedReloadTimer.HasPeriodElapsed() && !m_failedReloadTimer.IsStopped())
	{
		return;
	}
	if (!m_reloadTimer.HasPeriodElapsed() && !m_reloadTimer.IsStopped())
	{
		return;
	}

	m_primaryRechargeCooldown.Start();
	m_primaryCurrentEnergy -= m_energyPerBullet;
	Vec3 laserStartPos;
	if (m_fireRight)
	{
		laserStartPos = m_children[0]->GetWorldPosition() - m_children[0]->GetWorldOrientaiton().GetJBasis3D() + m_children[0]->GetWorldOrientaiton().GetIBasis3D();
	}
	else
	{
		laserStartPos = m_children[0]->GetWorldPosition() + m_children[0]->GetWorldOrientaiton().GetJBasis3D() + m_children[0]->GetWorldOrientaiton().GetIBasis3D();
	}

	SpawnInfo laserSpawnInfo(laserStartPos, m_children[0]->GetWorldOrientaiton(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName("LaserPlayer"));

	Actor* targetedActor = nullptr;
	if (m_targetedActorUID != ActorUID::INVALID)
	{
		targetedActor = m_targetedActorUID.GetActor();
	}
	if (targetedActor != nullptr)
	{
		float desiredVelocity = 50.f;
		float distanceBetweenEntitiesNow = GetDistance3D(GetWorldPosition(), targetedActor->GetWorldPosition());
		float desiredTravelTime = distanceBetweenEntitiesNow / desiredVelocity;
		int halfTravelBeatsRoundedDown = RoundDownToInt(desiredTravelTime / (BEAT_TIME * .5f));

		//in the future we can just treat this as a hitscan and spawn the particle at the enemy ship
		if (halfTravelBeatsRoundedDown % 2 == 0)
		{
			halfTravelBeatsRoundedDown++;
		}
		float travelTime = (float)halfTravelBeatsRoundedDown * (BEAT_TIME * .5f);
		Vec3 actorFuturePos = targetedActor->GetWorldPosition() + travelTime * targetedActor->m_velocity;
		Vec3 projectileVelocityDirection = actorFuturePos - laserStartPos;
		projectileVelocityDirection.SetLength(projectileVelocityDirection.GetLength() / travelTime);
		laserSpawnInfo.m_velocity = projectileVelocityDirection;
		laserSpawnInfo.m_orientation = GetRotationMatrixFromIBasisKUp(laserSpawnInfo.m_velocity.GetNormalized());
	}
	else
	{
		m_targetedActorUID = ActorUID::INVALID;
		laserSpawnInfo.m_velocity = m_children[0]->GetForwardNormal() * 50.f;
	}

	//g_theAudio->StartSound(m_laserSound, false, .125f);
	BeatSound beatSound;
	beatSound.m_beatTimeInSeconds = BEAT_TIME * .5f;
	beatSound.m_volume = .125f;
	beatSound.m_sound = m_laserSound;
	g_theGame->m_gameScene->QueueBeatSound(beatSound);
	
	g_theGame->m_gameScene->SpawnActor(laserSpawnInfo);
	//m_laserFiredOnBeat = true;
	m_fireRight = !m_fireRight;
}

void PlayerShip::UpdatePlayerDash(float deltaSeconds)
{
	//DebugAddMessage(Stringf("BEAT AVERAGE MS %.2f", m_averageBeatDeviationMS), 20.f, 0.f, Rgba8::WHITE, Rgba8::WHITE);
	if (!m_isDashing)
	{
		return;
	}
	UNUSED(deltaSeconds);
	float currentTimeFraction = m_dashTimer.GetElapsedFraction();
	if (!m_dashTimer.HasPeriodElapsed())
	{
		float dashSpeed = GetAnimValueFromTime(m_currentDashSpeedKeyFrames, currentTimeFraction);
		float roll = GetAnimValueFromTime(m_currentDashRollKeyFrames, currentTimeFraction);
		//get roll between -180 to 180 so that lerping works correctly
		if (roll > 180.f)
		{
			roll -= 360.f;
		}
		else if (roll < -180.f)
		{
			roll += 360.f;
		}	
		//lateral dash
		if (fabsf(m_currentDashDirection.y) > 0.f)
		{
			m_velocity.y = m_currentDashDirection.y * dashSpeed;
		}
		//forward dash
		else
		{
			m_velocity.x = m_currentDashDirection.x * dashSpeed;
		}
		m_moveModelRotation.m_roll = roll;
		return;
	}
	//dash just finished do cooldown
	else
	{
		m_isDashing = false;
		m_dashTimer.Stop();
		m_dashCooldownTimer.Start();

		for (int i = 0; i < (int)m_thrustEffects.size(); i++)
		{
			FloatGraph* maxSpeed = m_thrustEffects[i]->m_emitters[0]->GetFloatGraphByType(FloatGraphType::FLOATGRAPH_MAX_SPEED);
			maxSpeed->isDirty = 1;
			maxSpeed->constantValue = 1.f;
		}
	}
}

void PlayerShip::HandleControlsKeyboard(float deltaSeconds)
{

	if (g_theInput->IsKeyDown('W'))
	{
		HandlePlayerThrust(deltaSeconds, 1.f);
	}
	else if (g_theInput->IsKeyDown('S'))
	{
		HandlePlayerThrust(deltaSeconds, -1.f);
	}
	else
	{
		HandlePlayerThrust(deltaSeconds, 0.f);
	}

	Vec2 strafeAxis;
	if (g_theInput->IsKeyDown('A'))
	{
		strafeAxis.x = -1.f;
	}
	else if (g_theInput->IsKeyDown('D'))
	{
		strafeAxis.x = 1.f;
	}
	if (g_theInput->IsKeyDown(KEYCODE_L_SHIFT))
	{
		strafeAxis.y = 1.f;
	}
	//add down
	/*
	else if (g_theInput->IsKeyDown())
	{
		strafeAxis.y = -1.f;
	}
	*/
	HandlePlayerStrafe(deltaSeconds, strafeAxis);
	
	Vec2 mousePixelDisplacment = g_theInput->GetCursorClientDelta().GetVec2();
	m_mouseScreenSpaceOffset += mousePixelDisplacment;
	m_mouseScreenSpaceOffset.ClampLength(m_mouseConstraintPixelRadius);

	Vec2 yawPitchAxis = m_mouseScreenSpaceOffset / m_mouseConstraintPixelRadius;
	if (yawPitchAxis.x > 0)
	{
		yawPitchAxis.x = RangeMapClamped(yawPitchAxis.x, m_mouseAxisDeadzone, 1.f, 0.f, 1.f);
	}
	else
	{
		yawPitchAxis.x = RangeMapClamped(yawPitchAxis.x, -1.f, -m_mouseAxisDeadzone, -1.f, 0.f);
	}

	if (yawPitchAxis.y > 0)
	{
		yawPitchAxis.y = RangeMapClamped(yawPitchAxis.y, m_mouseAxisDeadzone, 1.f, 0.f, 1.f);
	}
	else
	{
		yawPitchAxis.y = RangeMapClamped(yawPitchAxis.y, -1.f, -m_mouseAxisDeadzone, -1.f, 0.f);
	}
	HandlePlayerYawPitchAxisAngle(deltaSeconds, yawPitchAxis);

	if (g_theInput->IsKeyDown('Q'))
	{
		m_adjustRollTimer.Start();
		m_currentFrameRotation.m_roll -= m_manualAdjustRollRate * deltaSeconds;
	}
	else if (g_theInput->IsKeyDown('E'))
	{
		m_adjustRollTimer.Start();
		m_currentFrameRotation.m_roll += m_manualAdjustRollRate * deltaSeconds;
	}

	if (g_theInput->WasKeyJustPressed(' '))
	{
		Vec2 dashDirection;
		if (g_theInput->IsKeyDown('A'))
		{
			dashDirection.x = -1.f;
		}
		else
		{
			dashDirection.x = 1.f;
		}
		dashDirection.y = 0.f;
		StartPlayerDash(dashDirection);
	}
}

void PlayerShip::HandleControlsController(float deltaSeconds)
{
	
	XboxController const controller = g_theInput->GetController(0);
	
	Vec2 leftjoysticPos = controller.GetLeftStick().GetPosition();
	Vec2 rightjoysticPos = controller.GetRightStick().GetPosition();
	float newDeadzone = .05f;
	if (rightjoysticPos.x > 0)
	{
		rightjoysticPos.x = RangeMapClamped(rightjoysticPos.x, newDeadzone, 1.f, 0.f, 1.f);
	}
	else
	{
		rightjoysticPos.x = RangeMapClamped(rightjoysticPos.x, -1.f, -newDeadzone, -1.f, 0.f);
	}

	if (rightjoysticPos.y > 0)
	{
		rightjoysticPos.y = RangeMapClamped(rightjoysticPos.y, newDeadzone, 1.f, 0.f, 1.f);
	}
	else
	{
		rightjoysticPos.y = RangeMapClamped(rightjoysticPos.y, -1.f, -newDeadzone, -1.f, 0.f);
	}

	HandlePlayerThrust(deltaSeconds, leftjoysticPos.y);

	Vec2 strafeAxis = Vec2(leftjoysticPos.x, 0.f);
	if (fabsf(strafeAxis.x) < .2f)
	{
		strafeAxis.x = 0.f;
	}
	if (controller.IsButtonDown(XboxController::LB_BUTTON))
	{
		strafeAxis.y = 1.f;
	}
	else if (controller.GetLeftTrigger() > .1f)
	{
		strafeAxis.y = -1.f;
	}
	HandlePlayerStrafe(deltaSeconds, strafeAxis);
	//DebugAddMessage(Stringf("Right Joystick %.2f, %.2f", rightjoysticPos.x, rightjoysticPos.y), 10.f, 0.f);
	//HandlePlayerYawPitch(deltaSeconds, rightjoysticPos);
	HandlePlayerYawPitchAxisAngle(deltaSeconds, rightjoysticPos);

	if (controller.IsButtonDown(XboxController::L3_BUTTON))
	{
		m_adjustRollTimer.Start();
		m_currentFrameRotation.m_roll -= m_manualAdjustRollRate * deltaSeconds;
	}
	else if (controller.IsButtonDown(XboxController::R3_BUTTON))
	{
		m_adjustRollTimer.Start();
		m_currentFrameRotation.m_roll += m_manualAdjustRollRate * deltaSeconds;
	}

	if (controller.WasButtonJustPressed(XboxController::A_BUTTON))
	{
		Vec2 dashDirection = controller.GetLeftStick().GetPosition();
		if (dashDirection.y > fabsf(dashDirection.x))
		{
			dashDirection.y = 1.f;
			dashDirection.x = 0.f;
		}
		else
		{
			dashDirection.y = 0.f;
			if (dashDirection.x == 0.f)
			{
				dashDirection.x = 1.f;
			}
		}
		dashDirection = dashDirection.GetNormalized();
		StartPlayerDash(dashDirection);
	}

	if (controller.WasButtonJustPressed(XboxController::Y_BUTTON) && m_megaLaserAmmo > 0.f)
	{
		m_usingMegaLaser = !m_usingMegaLaser;
		if (m_usingMegaLaser)
		{
			//g_theGame->m_gameScene->SetTrackLayerVolume(TrackLayerType::BASE, 0.f);
			//g_theGame->m_gameScene->SetTrackLayerVolume(TrackLayerType::LASER, 1.f);
			int beatsToCharge = 4 - (g_theGame->m_gameScene->m_currentBeats % 4);
			float chargeTime = ((float)beatsToCharge * BEAT_TIME) - g_theGame->m_gameScene->GetCurrentTimeFromLastBeat();
			float offsetInSound = 4.f * BEAT_TIME - chargeTime;
			m_megaLaserCharge->Play(chargeTime);
			m_currentMegaLaserSound = g_theAudio->StartSound(m_megaLaserChargeSFX, false, 1.f, .5f, 1.f, false, offsetInSound);
			m_megaLaserCurrentlyFiring = false;
		}
		else
		{
			m_megaLaserCharge->Stop();
			m_megaLaserFire->Stop();
			g_theAudio->StopSound(m_currentMegaLaserSound);
			//g_theGame->m_gameScene->SetTrackLayerVolume(TrackLayerType::BASE, 1.f);
			//g_theGame->m_gameScene->SetTrackLayerVolume(TrackLayerType::LASER, 0.f);
		}
	}

	if (controller.WasButtonJustPressed(XboxController::B_BUTTON))
	{
		if (m_missileAmmo <= 0)
		{
			return;
		}
		m_usingMissiles = !m_usingMissiles;


		if (m_usingMissiles)
		{
			//g_theGame->m_gameScene->SetTrackLayerVolume(TrackLayerType::MISSILE, 1.f);
			PlayMissileSoundAtCorrectOffset();
		}
		else
		{
			//g_theGame->m_gameScene->SetTrackLayerVolume(TrackLayerType::MISSILE, 0.f);
			StopMissileSound();
		}
	}

	if (controller.WasButtonJustPressed(XboxController::X_BUTTON))
	{
		if (!m_failedReloadTimer.IsStopped() && !m_failedReloadTimer.HasPeriodElapsed())
		{
			return;
		}
		if (g_theGame->m_gameScene->m_isFrameOnBeatToPlayer)
		{
			g_theAudio->StartSound(m_reloadSound, false, .3f);
			m_reloadTimer.Start();
			m_primaryEnergyBeforeRecharge = m_primaryCurrentEnergy;
		}
		else
		{
			g_theAudio->StartSound(m_reloadFailedSound);
			m_failedReloadTimer.Start();
		}
	}

	if (controller.GetRightTrigger() > .1f && g_theGame->m_gameScene->IsTimeAtBeatFraction(BEAT_TIME * .5f))
	{
		FireSmallLaser();
	}
}

void PlayerShip::UpdateMegaLaser(float deltaSeconds)
{
	DebugAddMessage(Stringf("Mega Laser Ammo: %.2f", m_megaLaserAmmo), 20.f, 0.f);
	Vec3 fireStartPos = m_children[0]->GetWorldPosition() + m_children[0]->GetForwardNormal() * 2.f;
	Vec3 approxFireEndPos = m_camera.GetWorldTransform().GetTranslation3D() + m_camera.GetWorldTransform().GetIBasis3D() * 500.f;
	Vec3 fireFwdNormal = (approxFireEndPos - fireStartPos).GetNormalized();

	//update position and orientation for laser effects
	m_megaLaserCharge->SetPosition(fireStartPos);

	Mat44 laserTransform;
	laserTransform.AppendTranslation3D(fireStartPos);

	Mat44 laserRotation;
	Vec3 laserIBasis = fireFwdNormal;
	Vec3 laserKBasis = m_camera.GetWorldTransform().GetKBasis3D();
	Vec3 laserJBasis = CrossProduct3D(laserKBasis, laserIBasis);
	laserRotation.SetIJK3D(laserIBasis, laserJBasis, laserKBasis);
	laserTransform.Append(laserRotation);

	m_megaLaserFire->SetWorldTransform(laserTransform);

	if (!m_usingMegaLaser)
	{
		return;
	}
	if (m_megaLaserAmmo <= 0.f)
	{
		StopMegaLaser();
		return;
	}

	if (g_theGame->m_gameScene->m_isFrameBeat && g_theGame->m_gameScene->m_currentBeats % 4 == 0)
	{
		m_megaLaserCurrentlyFiring = !m_megaLaserCurrentlyFiring;
		if (m_megaLaserCurrentlyFiring)
		{
			m_megaLaserFire->Play(4.f * BEAT_TIME);
			m_megaLaserCharge->Stop();
			if (g_theGame->m_gameScene->m_currentBeats % 8 == 0)
			{
				m_currentMegaLaserSound = g_theAudio->StartSound(m_megaLaserFire2SFX, false);
			}
			else
			{
				m_currentMegaLaserSound = g_theAudio->StartSound(m_megaLaserFire1SFX, false);
			}
		}
		else
		{
			m_megaLaserCharge->Play(4.f * BEAT_TIME);
			m_megaLaserFire->Stop();
			m_currentMegaLaserSound = g_theAudio->StartSound(m_megaLaserChargeSFX, false);
		}
	}
	Vec3 cameraPos = m_camera.GetWorldTransform().GetTranslation3D();
	Vec3 cameraFwd = m_camera.GetWorldTransform().GetIBasis3D();

	if (m_megaLaserCurrentlyFiring)
	{
		m_megaLaserAmmo -= BEAT_TIME * deltaSeconds;
		std::vector<Actor*>& damageableActors = g_theGame->m_gameScene->m_damageableActors;
		for (int i = 0; i < damageableActors.size(); i++)
		{
			if (!IsValidActor(damageableActors[i]))
			{
				continue;
			}

			Actor* actorToDamage = damageableActors[i];
			//#TODO also add cylinder Overlap detection
			Vec3 dispToActor = actorToDamage->GetWorldPosition() - cameraPos;
			float theta = GetUnsignedAngleBetweenVectors3D(dispToActor, cameraFwd);
			if (theta < m_maxMegaLaserDamageAngle)
			{
				actorToDamage->TakeDamage(m_megaLaserDPS * deltaSeconds);
			}
		}
	}
}

void PlayerShip::UpdateMissileFire(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	DebugAddMessage(Stringf("Missile Ammo:    %d", m_missileAmmo), 20.f, 0.f);

	if (!m_usingMissiles || !g_theGame->m_gameScene->IsTimeAtBeatFraction(BEAT_TIME*.5f))
	{
		return;
	}
	if (m_missileAmmo <= 0)
	{
		StopMissiles();
		return;
	}

	//loop missile sound correctly
	float currentMissileSoundPos = g_theAudio->GetSoundPlaybackPosition(m_missleTrackSFXPlayback);
	if (currentMissileSoundPos < m_prevFrameMissileTrackPos)
	{
		StopMissileSound();
		PlayMissileSoundAtCorrectOffset();
	}
	m_prevFrameMissileTrackPos = currentMissileSoundPos;

	Vec3 cameraPos = m_camera.GetWorldTransform().GetTranslation3D();
	Vec3 cameraFwd = m_camera.GetWorldTransform().GetIBasis3D();
	std::vector<Actor*> allActors = g_theGame->m_gameScene->m_allActors;
	Actor* actorToShootAt = nullptr;
	float bestTheta = 99999999999.f;
	for (int i = 0; i < (int)allActors.size(); i++)
	{
		if (!IsValidActor(allActors[i]))
		{
			continue;
		}
		Vec3 dispToActor = allActors[i]->GetWorldPosition() - cameraPos;
		float theta = GetUnsignedAngleBetweenVectors3D(dispToActor, cameraFwd);
		if (allActors[i]->m_definition->m_maxTargetAngle > 0.f && theta < 30.f && theta < bestTheta && !allActors[i]->m_missileChasing)
		{
			bestTheta = theta;
			actorToShootAt = allActors[i];
		}
	}
	if (actorToShootAt != nullptr)
	{
		m_missileAmmo--;
		actorToShootAt->m_missileChasing = true;
		Vec3 missileStartPos;
		Vec3 dispToActor = actorToShootAt->GetWorldPosition() - GetWorldPosition();
		Vec3 jBasis = GetWorldOrientaiton().GetJBasis3D();
		bool fireRight = DotProduct3D(dispToActor, jBasis) < 0.f;
		Vec3 i = m_children[0]->GetWorldOrientaiton().GetIBasis3D();
		Vec3 j = m_children[0]->GetWorldOrientaiton().GetJBasis3D();
		Vec3 k = m_children[0]->GetWorldOrientaiton().GetKBasis3D();
		if (fireRight)
		{
			missileStartPos = m_children[0]->GetWorldPosition() + 4.f*i - .75f*j - .6f*k;
		}
		else
		{
			missileStartPos = m_children[0]->GetWorldPosition() + 4.f*i + .75f*j - .6f*k;
		}
		SpawnInfo missileSpawnInfo(missileStartPos, m_children[0]->GetWorldOrientaiton(), Vec3::ZERO, ActorDefinition::GetActorDefinitionFromName("MissilePlayer"));
		Actor& firedMissile = g_theGame->m_gameScene->SpawnActor(missileSpawnInfo);
		float desiredVelocity = 40.f;
		float distanceBetweenEntitiesNow = GetDistance3D(GetWorldPosition(), actorToShootAt->GetWorldPosition());
		float desiredTravelTime = distanceBetweenEntitiesNow / desiredVelocity;
		int halfTravelBeatsRoundedDown = RoundDownToInt(desiredTravelTime / (BEAT_TIME * .5f));
		if (halfTravelBeatsRoundedDown == 0)
		{
			halfTravelBeatsRoundedDown++;
		}
		
		if ((halfTravelBeatsRoundedDown % 2 != 0 && g_theGame->m_gameScene->m_isFrameBeat) || (halfTravelBeatsRoundedDown % 2 == 0 && !g_theGame->m_gameScene->m_isFrameBeat))
		{
			halfTravelBeatsRoundedDown++;
		}
		

		float travelTime = (float)halfTravelBeatsRoundedDown * (BEAT_TIME * .5f);
		firedMissile.m_lifetimeOverride = travelTime;

		firedMissile.m_bezierCurve.m_startPos = missileStartPos;
		firedMissile.m_bezierCurve.m_guidePos1 = missileStartPos + m_children[0]->GetForwardNormal() * 2.f;

		firedMissile.m_bezierTarget = actorToShootAt->m_uid;
		Vec3 fireDir = actorToShootAt->GetWorldPosition() - missileStartPos;
		fireDir = fireDir.GetNormalized();

		Vec3 perpToPath = CrossProduct3D(fireDir, Vec3(0.f, 0.f, 1.f)).GetNormalized();

		//want to make missile always arc outwards
		if (DotProduct3D(fireDir, jBasis) * DotProduct3D(perpToPath, jBasis) < 0.f)
		{
			perpToPath *= -1.f;
		}
		firedMissile.m_missilePerpToPath = perpToPath;
	}
}

void PlayerShip::PlayMissileSoundAtCorrectOffset()
{
	SoundPlaybackID mainTrackPlayback = g_theGame->m_gameScene->GetTrackLayerPlaybackID(TrackLayerType::BASE);
	float mainTrackPlaybackPosition = g_theAudio->GetSoundPlaybackPosition(mainTrackPlayback);
	float missleSoundStartPos = mainTrackPlaybackPosition;
	float missileTrackLength = g_theAudio->GetSoundLength(m_missleTrackSFX);
	while (missleSoundStartPos > missileTrackLength)
	{
		missleSoundStartPos -= missileTrackLength;
	}
	m_missleTrackSFXPlayback = g_theAudio->StartSound(m_missleTrackSFX, true, 1.f, 0.f, 1.f, false, missleSoundStartPos);
	m_prevFrameMissileTrackPos = missleSoundStartPos;
}

void PlayerShip::StopMissileSound()
{
	g_theAudio->StopSound(m_missleTrackSFXPlayback);
	m_missleTrackSFXPlayback = MISSING_SOUND_ID;
}

AnimValue::AnimValue(float time, float value, EasingFunction easingFunction)
	: m_value(value)
	, m_easingFunction(easingFunction)
	, m_time(time)
{
}
