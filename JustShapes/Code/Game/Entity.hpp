#pragma once
#include "Game/GameCommon.hpp"
#include "Game/AnimationDefinition.hpp"
#include "Engine/Core/Timer.hpp"
#include "Game/EntityConfig.hpp"

class GameState;
class App;
class Renderer;
class Player;

extern App* g_theApp;
extern Renderer* g_theRenderer;
class Texture;

enum class EntityType
{
	UNASSIGNED,
	BUTTON,
	PLAYER_VFX,
	PLAYER,
	ENEMY_PROJECTILE,
	BOSS_1,
	BOSS_2,
	BOSS_3,
	SCYTHE,
	WISP,
	BOBBING_PROJECTILE,
	ARMS,
	COUNT
};

class Entity
{
public:
	Entity(GameState* gameState, EntityType entityType, Vec2 const& startPos, EntityConfig const& config = EntityConfig());
	virtual ~Entity();
	virtual void Update(float deltaSeconds);
	virtual void Render();
	Vec2 GetForwardNormal() const;
	bool IsScreenSpace();
	virtual void UpdatePhysics(float deltaSeconds);
	void SetTimeOffsetInAnimation(float timeOffset);
	void AddForce(Vec2 const& force);
	void AddImpulse(Vec2 const& impulse);
	void PlayAnimation(std::string animationName);
	virtual void DestroyEntity();
	Vec2 GetPosition();
	virtual bool OverlapsPlayer(Player* player);
	void SetPosition(Vec2 const& pos);
	void AddChildEntity(Entity* childToAdd);
	Mat44 GetModelMatrix();
public:
	Entity* m_parent = nullptr;
	std::vector<Entity*> m_children;
	EntityConfig m_config;
	Texture* m_texture = nullptr;
	AABB2 m_uvs = AABB2::ZERO_TO_ONE;
	AABB2 m_spriteBounds = AABB2::ZERO_TO_ONE;
	Vec2 m_velocity = Vec2::ZERO;
	Vec2 m_acceleration = Vec2::ZERO;
	Rgba8 m_color = Rgba8::WHITE;
	float m_rotationSpeed = 0.f;
	float m_drag = 0.f;
	float m_radius = 0.f;
	float m_orientationDegrees = 0.f;
	Vec2 m_scale = Vec2::ONE;
	bool m_isGarbage = false;
	GameState* m_gameState = nullptr;
	EntityType m_entityType = EntityType::UNASSIGNED;
	bool m_simulatePhysics = false;
	bool m_isHazard = false;
	int m_sortOrder = 0;
protected:
	void UpdateChildren(float deltaSeconds);
	void RenderChildren();
	void UpdateAnimation();
	void SetValuesToEndOfAnimation();
	Vec2 m_localPosition = Vec2::ZERO;
	Vec2 m_position = Vec2::ZERO;
	std::vector<Vertex_PCU> m_verts;
	AnimationDefinition const* m_currentAnimation = nullptr;
	Timer m_animationTimer;
	Timer m_becomeHazardTimer;
	Timer m_hideTimer;
	Timer m_liveTimer;
	Mat44 m_transform;
	//only to be used in animations to change where the entity is relative to m_position
	Vec2 m_normalizedPivot = Vec2(.5f, .5f);
};
