#pragma once
#include "Game/Actor.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"
#include "Game/ActorUID.hpp"


enum class ThrustEasingState
{
	MAX_SPEED,
	SLOWING_DOWN
};

struct AnimValue
{
	AnimValue(float time, float value, EasingFunction easingFunction);
	float m_value = 0.f;
	EasingFunction m_easingFunction;
	float m_time = 0.f;
};

class PlayerShip : public Actor
{
public:
	PlayerShip(SpawnInfo const& spawnInfo, ActorUID actorUID);
	~PlayerShip();
	void HandleControlsKeyboard(float deltaSeconds);
	void HandleControlsController(float deltaSeconds);
	void Update(float deltaSeconds) override;
	void Render() const override;
	void GetTargetedActor();
	Mat44 GetWorldTransform() const override;
	Mat44 GetCameraWorldTransform() const;
	virtual void TakeDamage(float damage) override;
	virtual void Die() override;
	void ResetGameState();
	Entity* GetShipModelEntity();
	Vec3 GetWorldSpaceVelocity();
	Camera m_camera;
	Timer m_dashCooldownTimer;
	Vec2 m_mouseScreenSpaceOffset;
	bool m_isPlayerAlive = true;

	//primary blaster
	float m_primaryMaxEnergy = 10.f;
	float m_primaryCurrentEnergy = 12.f;
	float m_energyPerBullet = 1.f;
	float m_primaryRechargeRate = 3.f;
	Timer m_primaryRechargeCooldown;
	bool m_primaryEnergyRecharging = false;
	bool m_primaryEnergyBeforeRecharge = 0.f;
	Timer m_failedReloadTimer;
	Timer m_reloadTimer;
	SoundID m_reloadSound = MISSING_SOUND_ID;
	SoundID m_reloadFailedSound = MISSING_SOUND_ID;

	bool m_isDashing = false;
	bool m_isDashGood = false;
	bool m_isInvincible = false;

	int m_missileAmmo = 0;
	float m_megaLaserAmmo = 0.f;

private:
	void StopMegaLaser();
	void StopMissiles();
	void HandlePlayerThrust(float deltaSeconds, float thrustAxis);                                                          
	void HandlePlayerStrafe(float deltaSeconds, Vec2 strafeAxis);
	void HandlePlayerYawPitch(float deltaSeconds, Vec2 yawPitchAxis);
	void MoveShipTowardsVelocity(float deltaSeconds);
	void HandlePlayerYawPitchAxisAngle(float deltaSeconds, Vec2 yawPitchAxis);

	void FixShipRoll(float deltaSeconds);
	void UpdatePlayerDash(float deltaSeconds);
	void StartPlayerDash(Vec2 dashDirection);
	float GetAnimValueFromTime(std::vector<AnimValue> const& anim, float time, bool override = false, float overrideStart = 0.f, float overrideEnd = 0.f);
	void UpdateThrustDirection();
	void FireSmallLaser();
	void UpdateMegaLaser(float deltaSeconds);
	void UpdateMissileFire(float deltaSeconds);


	Vec2 m_mouseLookSensitivity = Vec2(20.f, 20.f);
	Vec2 m_prevFrameMousePos;
	EulerAngles m_currentFrameRotation;
	Vec3 m_currentFrameTranslation;
	float m_currentRoll = 0.f;
	Timer m_adjustRollTimer;
	FloatRange m_autoAdjustRollRate = FloatRange(3.f, 30.f);
	float m_manualAdjustRollRate = 30.f;

	Mat44 m_cameraTransform;
	Vec3 m_defaultCameraDisplacment = Vec3(-6.5f, 0.f, 2.4f);
	EulerAngles m_defaultCameraRotation = EulerAngles(0.f, 1.5f, 0.f);

	//thrust
	float m_thrustAcceration = 10.f;
	FloatRange m_thrustSpeedRange = FloatRange(-5.f, 10.f);
	float m_nonThrustSlowDown = 5.f;

	float m_startModelThrustDisplacment = 0.f;
	float m_endModelThrustDisplacment = 0.f;
	bool m_thrustReturningIdlePrevFrame = true;
	FloatRange m_modelThrustDisplacmentRange = FloatRange(-.25f, .6f);
	float m_modelThrustTranslationX = 0.f;

	float m_prevDeltaThrust = 0.f;
	float m_thrustCurveStartSpeed = 0.f;
	float m_thrustCurveEndSpeed = 0.f;
	std::vector<ParticleEffect*> m_thrustEffects;
	std::vector<ParticleEffect*> m_dashEffects;
	FloatRange m_thrustParticlesRange = FloatRange(0.f, 75.f);
	FloatRange m_thrustParticlesMaxSpeed = FloatRange(0.f, 1.f);


	//strafe
	float m_strafeAcceleration = 10.f;
	float m_maxStrafeSpeed = 8.f;
	
	float m_nonstrafeSlowDown = 5.f;
	float m_maxSpeedAlongAxis = 35.f;
	FloatRange m_modelStrafeJTranslationRange = FloatRange(-2.5f, 2.5f);
	FloatRange m_modelStrafeKTranslationRange = FloatRange(-1.f, 1.f);
	FloatRange m_modelThrustITranslationRange = FloatRange(-3.f, 3.f);

	float m_modelStrafeTranslationSpeed = 6.f;

	//yaw/pitch
	float m_angularAcceleration = 360.f;
	float m_angularSlowDown = 270.f;
	float m_maxAngularSpeed = 120.f;
	Vec2 m_currentAngularVelocity;
	float m_currentRotationSpeed = 0.f;
	float m_currentAngularSpeed = 0.f;
	FloatRange m_modelYawJTranslationRange = FloatRange(-1.75f, 1.75);
	FloatRange m_modelPitchKTranslationRange = FloatRange(-1.75, 1.75);
	float m_modelYawPitchTranslationSpeed = 5.f;

	float m_maxModelPitchRotationOffset = 20.f;
	float m_maxModelRollRotationOffset = 20.f;
	float m_maxModelYawRotationOffset = 15.f;
	float m_modelYawPitchRotationSpeed = 6.f;
	float m_modelRollRotationSpeed = 6.f;

	float m_mouseAxisSensitivity = 1.f;
	float m_mouseConstraintPixelRadius = 150.f;
	float m_mouseAxisDeadzone = .15f;


	//dash
	SoundID m_dashSound = MISSING_SOUND_ID;
	std::vector<AnimValue> m_currentDashSpeedKeyFrames;
	std::vector<AnimValue> m_currentDashRollKeyFrames;

	std::vector<AnimValue> m_dashSpeedKeyFrames;
	std::vector<AnimValue> m_dashRollKeyFrames;
	std::vector<AnimValue> m_badDashSpeedKeyFrames;
	std::vector<AnimValue> m_badDashRollKeyFrames;

	Vec3 m_currentDashDirection;
	Vec3 m_startDashVelocity;
	float m_currentStrafeVelocityJ;
	float m_startDashRoll = 0.f;
	float m_currentStrafeRoll = 0.f;
	float m_dashBlendStartVelocityTime = .2f;
	float m_dashBlendEndVelocityTime = .6f;
	float m_dashBlendStartRollTime = .1f;
	float m_dashBlendEndRollTime = .8f;
	Timer m_dashTimer;
	float m_dashDuration = BEAT_TIME * 2;
	float m_badDashDuration = BEAT_TIME;
	float m_dashCooldownDuration = BEAT_TIME;
	Vec3 m_moveModelTranslation = Vec3::ZERO;
	Vec3 m_yawPitchModelTranslation = Vec3::ZERO;
	EulerAngles m_moveModelRotation;
	EulerAngles m_yawPitchModelRotation;

	ActorUID m_targetedActorUID = ActorUID::INVALID;
	bool m_fireRight = true;

	SoundID m_laserSound = MISSING_SOUND_ID;

	//mega laser
	bool m_usingMegaLaser = false;
	bool m_megaLaserCurrentlyFiring = false;
	ParticleEffect* m_megaLaserCharge = nullptr;
	ParticleEffect* m_megaLaserFire = nullptr;
	float m_megaLaserDPS = 25.f;
	float m_maxMegaLaserDamageAngle = 5.f;
	SoundID m_megaLaserChargeSFX = MISSING_SOUND_ID;
	SoundID m_megaLaserFire1SFX = MISSING_SOUND_ID;
	SoundID m_megaLaserFire2SFX = MISSING_SOUND_ID;
	SoundPlaybackID m_currentMegaLaserSound = MISSING_SOUND_ID;


	SoundID m_missleTrackSFX = MISSING_SOUND_ID;
	SoundPlaybackID m_missleTrackSFXPlayback = MISSING_SOUND_ID;
	float m_prevFrameMissileTrackPos = 0.f;

	//missiles
	bool m_usingMissiles = false;

	float m_averageBeatDeviationMS = 0.f;

	void PlayMissileSoundAtCorrectOffset();
	void StopMissileSound();

};