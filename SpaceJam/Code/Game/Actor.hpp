#pragma once
#include "Game/Entity.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/ActorUID.hpp"

class AIBehaviour;

struct SpawnInfo
{
	SpawnInfo(Vec3 const& position, Mat44 const& m_orientation, Vec3 const& velocity, ActorDefinition const* actorDef);
	Vec3 m_position;
	Vec3 m_velocity;
	Mat44 m_orientation;
	ActorDefinition const* m_definition;
};

class Actor : public Entity
{
public:
	Actor(SpawnInfo const& spawnInfo, ActorUID actorUID);
	virtual ~Actor();
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	bool ActorOverlapHandled(Actor& otherActor);
	virtual void Die();
	virtual void TakeDamage(float damage);
	void DebugDrawCollider();
	virtual void Activate();

	void SetRotationFromFwdPlayerUp(Vec3 const& fwdNormal);

	ActorUID m_uid = ActorUID::INVALID;
	ActorDefinition const* m_definition = nullptr;
	bool m_isTargeted = false;
	bool m_missileChasing = false;
	Timer m_changeDirectionTimer;
	bool steerForward = true;
	std::vector<ParticleEffect*> m_ownedParticleEffects;
	Timer m_lifetimeTimer;
	Timer m_deathTimer;
	float m_currentHealth = 0.f;
	Vec3 m_prevFramePos;
	bool m_isActive = true;
	EulerAngles m_rotationSpeed;
	float m_lifetimeOverride = -1.f;

	Vec3 m_missilePerpToPath;
	ActorUID m_bezierTarget = ActorUID::INVALID;
	CubicBezierCurve3D m_bezierCurve;

	//AI
	AIBehaviour* m_currentBehaviour = nullptr;
	std::vector<AIBehaviour*> m_aiBehaviours;

	Vec3 m_thrustVelocity;
	Vec3 m_strafeVelocity;
	bool m_overrideOrientation = false;

private:
	void UpdateMissileFirePath(Actor* targetActor);
};