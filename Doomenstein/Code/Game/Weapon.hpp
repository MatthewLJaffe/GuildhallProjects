#pragma once
#include "Game/WeaponDefinition.hpp"
#include "Engine/Core/Time.hpp"

class Actor;
class SpriteDefinition;

class Weapon
{
public:
	Weapon(WeaponDefinition const* def, ActorUID owningActor);
	WeaponDefinition const* m_definition = nullptr;
	ActorUID m_owningActor;
	float m_timeSinceLastFire = 0.f;
	bool Fire(Vec3 const& position, Vec3 const& direction);
	Vec3 GetRandomDirectionInCone(Vec3 forwardDir, float coneDegrees);
	SpriteDefinition const& GetCurrentAnimationFrame();
private:
	void FireRays(Vec3 const& position, Vec3 const& direction);
	void FireProjectiles(Vec3 const& position, Vec3 const& direction);
	void FireMelees(Vec3 const& position, Vec3 const& direction);
	Actor* GetOwningActorPtr();
	SoundPlaybackID m_currentSound = MISSING_SOUND_ID;
	Timer m_soundPlayTimer;
};