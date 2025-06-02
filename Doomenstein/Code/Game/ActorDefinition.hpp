#pragma once
#include "Game/Definition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

class WeaponDefinition;

struct AnimationDirection
{
	AnimationDirection(Vec3 const& direction, SpriteAnimDefinition const& animationDef);
	Vec3 m_directionVector;
	SpriteAnimDefinition m_animationDef;
};

struct AnimationGroup
{
	std::string m_name;
	bool m_scaleBySpeed  = true;
	float m_secondsPerFrame = .25f;
	int m_numFrames = 0;
	SpriteAnimPlaybackType m_playbackMode = SpriteAnimPlaybackType::LOOP;
	std::vector<AnimationDirection> m_animationDirections;
};

class ActorDefinition : public Definition
{
public:
	~ActorDefinition();
	//Methods all definitions should have
	static void InitializeDefinitions(const char* path);
	static void ClearDefinitions();
	static ActorDefinition const* GetByName(const std::string& name);
	static ActorDefinition const* GetByColor(Rgba8 const& color);
	static std::vector<ActorDefinition const*> s_actorDefinitions;
	bool LoadFromXmlElement(XmlElement const& element) override;

	//general
	Faction m_faction;
	float m_health = 1.f;
	bool m_canBePossesed = false;
	float m_corpseLifetime = 0.f;
	bool m_visible = false;
	bool m_dieOnSpawn = false;
	float m_lifetime = -1.f;
	Rgba8 m_mapColor;

	//collision
	float m_collisionRadius = 0.f;
	float m_collisionHeight = 0.f;
	bool m_collidesWithWorld = false;
	bool m_collidesWithActors = false;
	FloatRange m_damageOnCollide;
	float m_impulseOnCollide = 0.f;
	bool m_dieOnCollide = false;
	bool m_collidesWithSelf = true;
	std::string m_spawnActorOnFloor = "";
	bool m_damageable = false;
	std::string m_pickupWeapon = "";

	//physics
	bool m_physicsSimulated = false;
	float m_walkSpeed = 0.f;
	float m_runSpeed = 0.f;
	float m_turnSpeed = 180.f;
	float m_drag = 0.f;
	bool m_isFlying = false;
	bool m_isStatic = false;

	//camera
	float m_eyeHeight = .5f;
	float m_cameraFOV = 60.f;

	//AI
	bool m_aiEnabled = false;
	bool m_binkeyAI = false;
	float m_sightRadius = 0.f;
	float m_sightAngle = 0.f;

	//visuals
	Vec2 m_billboardSize = Vec2::ONE;
	Vec2 m_billboardPivot = Vec2(.5f, 0.f);
	BillboardType m_billboardType = BillboardType::WORLD_UP_FACING;
	bool m_renderLit = true;
	bool m_renderRounded = true;
	Shader* m_shader = nullptr;
	SpriteSheet const* m_spriteSheet = nullptr;
	std::vector<AnimationGroup> m_animationGroups;
	bool m_depthDisabled = false;
	BlendMode m_blendMode = BlendMode::ALPHA;
	Rgba8 m_tint = Rgba8::WHITE;
	bool m_spawnPointLight = false;	

	//sounds
	SoundID m_hurtSound = MISSING_SOUND_ID;
	SoundID m_deathSound = MISSING_SOUND_ID;

	//Inventory
	std::vector<WeaponDefinition const*> m_inventory;

private:
	void LoadAnimationGroup(XmlElement const& element);
	void ParseSounds(XmlElement const& soundsElement);
	void ParseAnimationGroups(XmlElement const& visualsElement);
	Faction GetFactionFromAttributeName(std::string factionName);
	BlendMode GetBlendModeFromAttributeName(std::string blendModeName);
	BillboardType GetBillboardTypeFromAttributeName(std::string billboardName);
	SpriteAnimPlaybackType GetPlaybackTypeFromAttributeName(std::string attributeName);
};