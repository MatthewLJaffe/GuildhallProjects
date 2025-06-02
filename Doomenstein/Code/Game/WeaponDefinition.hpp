#pragma once
#include "Game/Definition.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Game/GameCommon.hpp"

class SpriteAnimDefinition;
class ActorDefinition;

class WeaponDefinition : public Definition
{
public:
	//Methods all definitions should have
	~WeaponDefinition();
	static void InitializeDefinitions(const char* path);
	static void ClearDefinitions();
	static WeaponDefinition const* GetByName(const std::string& name);
	static std::vector<WeaponDefinition const*> s_weaponDefinitions;
	bool LoadFromXmlElement(XmlElement const& element) override;
	void ParseSounds(XmlElement const& soundsElement);
	void ParseHud(XmlElement const& soundsElement);

public:
	float m_refireTime = 0.f; 
	int m_rayCount = 0;
	float m_rayCone = 0.f;
	float m_rayRange = 0.f;
	FloatRange m_rayDamage;
	float m_rayImpulse = 0.f;
	int m_projectileCount = 0;
	float m_projectileCone = 0.f;
	float m_projectileSpeed = 0.f;
	ActorDefinition const* m_projectileActor = nullptr;
	int m_meleeCount = 0;
	float m_meleeRange = 0.f;
	float m_meleeArc = 0.f;
	FloatRange m_meleeDamage;
	float m_meleeImpulse = 0.f;
	SoundID m_fireSound;
	float m_soundPlayCooldown = 0.f;

	SpriteAnimDefinition* m_attackAnimation = nullptr;
	SpriteAnimDefinition* m_idleAnimation = nullptr;
	Shader* m_hudShader = nullptr;
	Texture* m_reticleTexture = nullptr;
	Texture* m_baseTexture = nullptr;
	Vec2 m_reticleSize;
	Vec2 m_spriteSize;
	Vec2 m_spritePivot;
};