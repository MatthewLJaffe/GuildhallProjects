#pragma once
#include "Game/GameCommon.hpp"
#include "SpawnInfo.hpp"

class Controller;
class AIController;
class Map;
struct RaycastResultDoomenstein;
class Weapon;
struct AnimationGroup;
class SpriteDefinition;

class Actor
{
public:
	Actor(SpawnInfo const& spawnInfo, ActorUID const& actorUID, Map* map);
	~Actor();
	ActorDefinition const* m_actorDefinition = nullptr;
	Map* m_map;
	Vec3 m_position = Vec3::ZERO;
	Vec3 m_velocity;
	Vec3 m_acceleration;
	EulerAngles m_orientation;
	Faction m_currentFaction = Faction::NEUTRAL;
	Rgba8 m_color = Rgba8::WHITE;
	int m_currentWeaponIdx = 0;
	float m_physicsCylinderHeight = 0.f;
	float m_physicsCylinderRadius = 0.f;
	bool m_isGarbage = false;
	float m_currentHealth = 0.f;
	Controller* m_controller = nullptr;
	AIController* m_previousAIController = nullptr;
	//only applies to projectiles
	ActorUID m_owningActor;
	std::vector<Weapon*> m_weapons;
	float m_currentDeadTime = 0.f;
	bool m_isDead = false;

private:
	std::vector<Vertex_PCU> m_actorVerts;
	ActorUID m_actorUID;

public:
	void Update(float deltaSeconds);
	void Render();
	void OnPossessed(Controller* controllerPossessing);
	void OnUnpossessed();
	void OnCollide(Actor* collidingActor);
	bool IsPlayer();
	Mat44 GetModelMatrix();
	Mat44 GetModelMatrixZUp();
	RaycastResultDoomenstein RaycastVsActor(Vec3 const& startPos, Vec3 const& rayFwdNormal, float rayDistance);
	FloatRange GetPhysicsCylinderZMinMax();
	Vec2 GetPhysicsCylinderCenter();
	ActorUID GetActorUID();
	void SetActorUID(ActorUID actorUID);
	void UpdatePhysics(float deltaSeconds);
	void AddForce(Vec3 force);
	void AddImpulse(Vec3 impulse);
	void MoveInDirection(Vec3 direction, float speed);
	void TakeDamage(float damage, ActorUID damageSource);
	void TurnTorwardsDirection(Vec2 const& directionXY);
	Vec3 GetForwardZUp();
	bool IsOpposingFaction(Actor* otherActor);
	void Attack();
	bool IsAlive() const;
	void KillActor();
	void DestroyActor();
	bool IsInSightline(ActorUID const& targetActor);
	void EquipPreviousWeapon();
	void EquipNextWeapon();
	Weapon* GetCurrentWeapon();
	SoundPlaybackID m_activeHurtSound;
	Clock* m_animationClock;
	std::string m_currAnimStateName = "Walk";
	AnimationGroup GetCurrentAnimationState();
	SpriteDefinition GetSpriteDefForCurrentAnimation();
	void TransitionAnimationState(std::string newAnimState);
	void UpdateAnimation(float deltaSeconds);
	void AddVertsForRoundedActorQuad(std::vector<Vertex_PCUTBN>& verts, AABB2 uvs);
	void AddVertsForActorQuad(std::vector<Vertex_PCU>& verts, AABB2 uvs);
	void GetActorSpriteBounds(Vec3& out_spriteMins, Vec3& out_spriteMaxs);
	Timer m_lifeTimer;
};